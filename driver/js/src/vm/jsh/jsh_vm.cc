/*
 *
 * Tencent is pleased to support the open source community by making
 * Hippy available.
 *
 * Copyright (C) 2022 THL A29 Limited, a Tencent company.
 * All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include "driver/vm/jsh/jsh_vm.h"

#include "footstone/check.h"
#include "footstone/string_view.h"
#include "footstone/string_view_utils.h"
#include "dom/dom_event.h"
#include "driver/napi/jsh/jsh_ctx.h"
#include "driver/napi/jsh/jsh_ctx_value.h"
#include "driver/napi/jsh/jsh_try_catch.h"

using string_view = footstone::string_view;
using Ctx = hippy::napi::Ctx;
using JSHCtx = hippy::napi::JSHCtx;
using CtxValue = hippy::napi::CtxValue;
using string_view = footstone::string_view;
using StringViewUtils = footstone::StringViewUtils;


namespace hippy {
inline namespace driver {
inline namespace vm {

static bool platform_initted = false;
static std::mutex mutex;

JSHVM::JSHVM() : VM() {
  JSVM_VMInfo vmInfo;
  OH_JSVM_GetVMInfo(&vmInfo);
  FOOTSTONE_DLOG(INFO) << "JSHVM begin, apiVersion: " << vmInfo.apiVersion
    << ", engine: " << vmInfo.engine 
    << ", version: " << vmInfo.version;
  {
    std::lock_guard<std::mutex> lock(mutex);
    if (!platform_initted) {
      JSVM_InitOptions init_options;
      memset(&init_options, 0, sizeof(init_options));
      auto status = OH_JSVM_Init(&init_options);
      FOOTSTONE_CHECK(status == JSVM_OK);
      platform_initted = true;
#ifdef ENABLE_INSPECTOR
      auto trace = reinterpret_cast<v8::platform::tracing::TracingController*>(platform->GetTracingController());
      devtools::DevtoolsDataSource::OnGlobalTracingControlGenerate(trace);
#endif
    }
  }
  
  // TODO(hot): vm params
  
  JSVM_CreateVMOptions options;
  memset(&options, 0, sizeof(options));
  auto status = OH_JSVM_CreateVM(&options, &vm_);
  FOOTSTONE_CHECK(status == JSVM_OK);
  status = OH_JSVM_OpenVMScope(vm_, &vm_scope_);
  FOOTSTONE_CHECK(status == JSVM_OK);

  FOOTSTONE_DLOG(INFO) << "V8VM end";
}

// constexpr static int kScopeWrapperIndex = 5;

void JSHVM::AddUncaughtExceptionMessageListener(const std::unique_ptr<FunctionWrapper>& wrapper) const {
  FOOTSTONE_CHECK(!wrapper->data) << "Snapshot requires the parameter data to be nullptr or the address can be determined during compilation";

  // TODO(hot-js):
}

JSHVM::~JSHVM() {
  FOOTSTONE_LOG(INFO) << "~JSHVM";

#if defined(ENABLE_INSPECTOR) && !defined(JSH_WITHOUT_INSPECTOR)
  inspector_client_ = nullptr;
#endif
  
  OH_JSVM_CloseVMScope(vm_, vm_scope_);
  vm_scope_ = nullptr;
  OH_JSVM_DestroyVM(vm_);
  vm_ = nullptr;
}

void JSHVM::PlatformDestroy() {
  platform_initted = false;
}

std::shared_ptr<Ctx> JSHVM::CreateContext() {
  FOOTSTONE_DLOG(INFO) << "CreateContext";
  return std::make_shared<JSHCtx>(vm_);
}

string_view JSHVM::ToStringView(JSVM_Env env, JSVM_Value string_value) {
  FOOTSTONE_DCHECK(string_value);
  
  JSHHandleScope handleScope(env);
  
  // TODO(hot-js): need handle one byte string?
  
  size_t result = 0;
  auto status = OH_JSVM_GetValueStringUtf16(env, string_value, NULL, 0, &result);
  if (status != JSVM_OK || result == 0) {
    return "";
  }
  std::u16string two_byte_string;
  two_byte_string.resize(result + 1);
  status = OH_JSVM_GetValueStringUtf16(env, string_value, reinterpret_cast<char16_t*>(&two_byte_string[0]), result + 1, &result);
  FOOTSTONE_DCHECK(status == JSVM_OK);
  two_byte_string.resize(result);
  return string_view(two_byte_string);
}

std::shared_ptr<CtxValue> JSHVM::CreateJSHString(JSVM_Env env, const string_view& str_view) {
  JSHHandleScope handleScope(env);

  JSVM_Value result = 0;
  string_view::Encoding encoding = str_view.encoding();
  switch (encoding) {
    case string_view::Encoding::Latin1: {
      const std::string& one_byte_str = str_view.latin1_value();
      auto status = OH_JSVM_CreateStringLatin1(env, one_byte_str.c_str(), one_byte_str.size(), &result);
      FOOTSTONE_DCHECK(status == JSVM_OK);
      return std::make_shared<JSHCtxValue>(env, result);
    }
    case string_view::Encoding::Utf8: {
      const string_view::u8string& utf8_str = str_view.utf8_value();
      auto status = OH_JSVM_CreateStringUtf8(env, (const char *)utf8_str.c_str(), utf8_str.size(), &result);
      FOOTSTONE_DCHECK(status == JSVM_OK);
      return std::make_shared<JSHCtxValue>(env, result);
    }
    case string_view::Encoding::Utf16: {
      const std::u16string& two_byte_str = str_view.utf16_value();
      auto status = OH_JSVM_CreateStringUtf16(env, two_byte_str.c_str(), two_byte_str.length(), &result);
      FOOTSTONE_DCHECK(status == JSVM_OK);
      return std::make_shared<JSHCtxValue>(env, result);
    }
    default:break;
  }

  FOOTSTONE_UNREACHABLE();
}

std::shared_ptr<VM> CreateVM(const std::shared_ptr<VM::VMInitParam>& param) {
  return std::make_shared<JSHVM>();
}

std::shared_ptr<CtxValue> JSHVM::ParseJson(const std::shared_ptr<Ctx>& ctx, const string_view& json) {
  if (StringViewUtils::IsEmpty(json)) {
    return nullptr;
  }

  auto jsh_ctx = std::static_pointer_cast<JSHCtx>(ctx);
  JSHHandleScope handleScope(jsh_ctx->env_);
//   auto isolate = v8_ctx->isolate_;
//   v8::HandleScope handle_scope(isolate);
//   auto context = v8_ctx->context_persistent_.Get(isolate);
//   v8::Context::Scope context_scope(context);

//   auto jj = StringViewUtils::ConvertEncoding(json, footstone::string_view::Encoding::Utf8);

  auto string_value = JSHVM::CreateJSHString(jsh_ctx->env_, json);
  if (!string_value) {
    return nullptr;
  }
  auto jsh_string_value = std::static_pointer_cast<JSHCtxValue>(string_value);
  JSVM_Value result = nullptr;
  auto status = OH_JSVM_JsonParse(jsh_ctx->env_, jsh_string_value->GetValue(), &result);
  if (status == JSVM_GENERIC_FAILURE) {
    FOOTSTONE_LOG(ERROR) << "JSHVM::ParseJson error, json: " << json;
    return nullptr;
  }
  FOOTSTONE_DCHECK(status == JSVM_OK);
  return std::make_shared<JSHCtxValue>(jsh_ctx->env_, result);
}

JSHVM::DeserializerResult JSHVM::Deserializer(const std::shared_ptr<Ctx>& ctx, const std::string& buffer) {
//   v8::HandleScope handle_scope(isolate_);
//   auto v8_ctx = std::static_pointer_cast<V8Ctx>(ctx);
//   auto context = v8_ctx->context_persistent_.Get(isolate_);
//
//   V8TryCatch try_catch(true, v8_ctx);
//   v8::ValueDeserializer deserializer(
//       isolate_,
//       reinterpret_cast<const uint8_t*>(buffer.c_str()),
//       buffer.length());
//   FOOTSTONE_CHECK(deserializer.ReadHeader(context).FromMaybe(false));
//   auto ret = deserializer.ReadValue(context);
//   if (!ret.IsEmpty()) {
//     return {
//       true,
//       std::make_shared<V8CtxValue>(isolate_, ret.ToLocalChecked()),
//       ""
//     };
//   } else if (try_catch.HasCaught()) {
//     return {
//       false,
//       nullptr,
//       try_catch.GetExceptionMessage()
//     };
//   }
  return {false, nullptr, ""};
}

}
}
}
