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

#include "driver/napi/jsh/jsh_ctx.h"
#include <hilog/log.h>
#include <sys/stat.h>

#include "driver/base/js_value_wrapper.h"
#include "driver/napi/jsh/jsh_ctx_value.h"
#include "driver/napi/jsh/jsh_class_definition.h"
#include "driver/napi/jsh/jsh_try_catch.h"
#include "driver/napi/callback_info.h"
#include "driver/vm/jsh/jsh_vm.h"
#include "driver/vm/native_source_code.h"
#include "footstone/check.h"
#include "footstone/string_view.h"
#include "footstone/string_view_utils.h"

namespace hippy {
inline namespace driver {
inline namespace napi {

using string_view = footstone::string_view;
using StringViewUtils = footstone::StringViewUtils;
using JSHVM = hippy::vm::JSHVM;
using CallbackInfo = hippy::CallbackInfo;

// constexpr static int kExternalIndex = 0;
// constexpr static int kNewInstanceExternalIndex = 1;
constexpr static int kScopeWrapperIndex = 5;
// constexpr static int kExternalDataMapIndex = 6;
//constexpr char kProtoKey[] = "__proto__";

// TODO(hot-js):
std::map<int32_t, void*> sEmbedderDataMap;
static void* sGetAlignedPointerFromEmbedderData(int index) {
  return sEmbedderDataMap[index];
}

std::map<JSVM_Value, void*> sEmbedderExternalMap;

void CheckPendingExeception(JSVM_Env env_, JSVM_Status status) {
  if (status == JSVM_PENDING_EXCEPTION) {
    JSVM_Value error;
    if (JSVM_OK == OH_JSVM_GetAndClearLastException((env_), &error))
    {
        // 获取异常堆栈
        JSVM_Value stack;
        OH_JSVM_GetNamedProperty((env_), error, "stack", &stack);

        JSVM_Value message;
        OH_JSVM_GetNamedProperty((env_), error, "message", &message);

        char stackstr[256];
        OH_JSVM_GetValueStringUtf8(env_, stack, stackstr, 256, nullptr);
        OH_LOG_INFO(LOG_APP, "xxx hippy JSVM error stack: %{public}s", stackstr);

        char messagestr[256];
        OH_JSVM_GetValueStringUtf8(env_, message, messagestr, 256, nullptr);
        OH_LOG_INFO(LOG_APP, "xxx hippy JSVM error message: %{public}s", messagestr);
    }
  }
}

/*
void InvokePropertyCallback(v8::Local<v8::Name> property,
                            const v8::PropertyCallbackInfo<v8::Value>& info) {
  auto isolate = info.GetIsolate();
  v8::HandleScope handle_scope(isolate);
  auto context = isolate->GetCurrentContext();
  v8::Context::Scope context_scope(context);

  CallbackInfo cb_info;
  cb_info.SetSlot(context->GetAlignedPointerFromEmbedderData(kScopeWrapperIndex));
  cb_info.SetReceiver(std::make_shared<V8CtxValue>(isolate, info.This()));
  auto name = std::make_shared<V8CtxValue>(isolate, property);
  cb_info.AddValue(name);
  auto data = info.Data().As<v8::External>();
  FOOTSTONE_CHECK(!data.IsEmpty());
  auto* func_wrapper = reinterpret_cast<FunctionWrapper*>(data->Value());
  FOOTSTONE_CHECK(func_wrapper && func_wrapper->callback);
  (func_wrapper->callback)(cb_info, func_wrapper->data);
  auto exception = std::static_pointer_cast<V8CtxValue>(cb_info.GetExceptionValue()->Get());
  if (exception) {
    const auto& global_value = exception->global_value_;
    auto handle_value = v8::Local<v8::Value>::New(isolate, global_value);
    isolate->ThrowException(handle_value);
    info.GetReturnValue().SetUndefined();
    return;
  }

  auto ret_value = std::static_pointer_cast<V8CtxValue>(cb_info.GetReturnValue()->Get());
  if (!ret_value) {
    info.GetReturnValue().SetUndefined();
    return;
  }

  info.GetReturnValue().Set(ret_value->global_value_);
}
//*/

/*
static void InvokeJsCallback_v8_bak(const v8::FunctionCallbackInfo<v8::Value>& info) {
  auto isolate = info.GetIsolate();
  v8::HandleScope handle_scope(isolate);
  auto context = isolate->GetCurrentContext();
  v8::Context::Scope context_scope(context);

  CallbackInfo cb_info;
  cb_info.SetSlot(context->GetAlignedPointerFromEmbedderData(kScopeWrapperIndex));
  auto thiz = info.This();
  auto holder = info.Holder();
  auto count = holder->InternalFieldCount();
  if (count > 0) {
    auto internal_data = holder->GetInternalField(kExternalIndex).As<v8::External>();
    cb_info.SetData(internal_data->Value());
  }
  if (info.IsConstructCall()) {
    auto new_instance_external = context->GetAlignedPointerFromEmbedderData(kNewInstanceExternalIndex);
    if (new_instance_external) {
      cb_info.SetData(new_instance_external);
    }
  }
  cb_info.SetReceiver(std::make_shared<V8CtxValue>(isolate, thiz));
  for (int i = 0; i < info.Length(); i++) {
    cb_info.AddValue(std::make_shared<V8CtxValue>(isolate, info[i]));
  }
  auto data = info.Data().As<v8::External>();
  FOOTSTONE_CHECK(!data.IsEmpty());
  auto function_wrapper = reinterpret_cast<FunctionWrapper*>(data->Value());
  auto js_cb = function_wrapper->callback;
  auto external_data = function_wrapper->data;
//  auto external_data_map = reinterpret_cast<std::unordered_map<void*, void*>*>(context->GetAlignedPointerFromEmbedderData(kExternalDataMapIndex));
//  void* external_data = nullptr;
//  auto it = external_data_map->find(js_cb_address);
//  if ( it != external_data_map->end()) {
//    external_data = it->second;
//  }
  js_cb(cb_info, external_data);
  auto exception = std::static_pointer_cast<V8CtxValue>(cb_info.GetExceptionValue()->Get());
  if (exception) {
    const auto& global_value = exception->global_value_;
    auto handle_value = v8::Local<v8::Value>::New(isolate, global_value);
    isolate->ThrowException(handle_value);
    info.GetReturnValue().SetUndefined();
    return;
  }

  auto ret_value = std::static_pointer_cast<V8CtxValue>(cb_info.GetReturnValue()->Get());
  if (!ret_value) {
    info.GetReturnValue().SetUndefined();
    return;
  }
  if (info.IsConstructCall()) {
    auto internal_data = cb_info.GetData();
    if (internal_data) {
      holder->SetInternalField(kExternalIndex, v8::External::New(isolate, internal_data));
      auto prototype = thiz->GetPrototype();
      if (!prototype.IsEmpty()) {
        auto prototype_object = v8::Local<v8::Object>::Cast(prototype);
        if (prototype_object->InternalFieldCount() > 0) {
          prototype_object->SetInternalField(kExternalIndex, v8::External::New(isolate, internal_data));
        }
      }
    }
  }
  info.GetReturnValue().Set(ret_value->global_value_);
}
//*/

JSVM_Value InvokeJsCallback(JSVM_Env env, JSVM_CallbackInfo info) {
  size_t argc = 10; // TODO(hot-js):
  JSVM_Value args[10];
  JSVM_Value thisArg;
  void *data = nullptr;
  auto s = OH_JSVM_GetCbInfo(env, info, &argc, args, &thisArg, &data);
  FOOTSTONE_DCHECK(s == JSVM_OK);
  
  FOOTSTONE_DLOG(INFO) << "xxx hippy, InvokeJsCallback, thisArg: " << thisArg << ", argc: " << argc << ", data: " << data;
  
    JSVM_ValueType valuetype = (JSVM_ValueType)100;
    JSVM_Status status = OH_JSVM_Typeof(env, thisArg, &valuetype);
    FOOTSTONE_DCHECK(status == JSVM_OK);
    FOOTSTONE_DLOG(INFO) << "xxx hippy, InvokeJsCallback, thisArg type, " << valuetype;
    if (valuetype == JSVM_EXTERNAL) {
      FOOTSTONE_DCHECK(0);
    }
  
  CallbackInfo cb_info;
  cb_info.SetSlot(sGetAlignedPointerFromEmbedderData(kScopeWrapperIndex));
//   auto thiz = info.This();
//   auto holder = info.Holder();
//   auto count = holder->InternalFieldCount();
//   if (count > 0) {
//     auto internal_data = holder->GetInternalField(kExternalIndex).As<v8::External>();
//     cb_info.SetData(internal_data->Value());
//   }
//   if (info.IsConstructCall()) {
//     auto new_instance_external = context->GetAlignedPointerFromEmbedderData(kNewInstanceExternalIndex);
//     if (new_instance_external) {
//       cb_info.SetData(new_instance_external);
//     }
//   }
  
  void *external = sEmbedderExternalMap[thisArg];
  
    // TODO(hot-js):
    auto new_instance_external = (void*)external; //context->GetAlignedPointerFromEmbedderData(kNewInstanceExternalIndex);
    if (new_instance_external) {
      cb_info.SetData(new_instance_external);
    } else {
      cb_info.SetData((void*)1);
    }
  
  cb_info.SetReceiver(std::make_shared<JSHCtxValue>(env, thisArg));
  for (size_t i = 0; i < argc; i++) {
    cb_info.AddValue(std::make_shared<JSHCtxValue>(env, args[i]));
  }
//   auto data = info.Data().As<v8::External>();
//   FOOTSTONE_CHECK(!data.IsEmpty());
  auto function_wrapper = reinterpret_cast<FunctionWrapper*>(data);
  auto js_cb = function_wrapper->callback;
  auto external_data = function_wrapper->data;
  
  js_cb(cb_info, external_data);
//   auto exception = std::static_pointer_cast<V8CtxValue>(cb_info.GetExceptionValue()->Get());
//   if (exception) {
//     const auto& global_value = exception->global_value_;
//     auto handle_value = v8::Local<v8::Value>::New(isolate, global_value);
//     isolate->ThrowException(handle_value);
//     info.GetReturnValue().SetUndefined();
//     return;
//   }

  auto ret_value = std::static_pointer_cast<JSHCtxValue>(cb_info.GetReturnValue()->Get());
  if (!ret_value) {
    
    auto error_ctx_value = std::static_pointer_cast<JSHCtxValue>(cb_info.GetExceptionValue()->Get());
    if (error_ctx_value) {
          auto error = error_ctx_value->GetValue();
    
        // 获取异常堆栈
        JSVM_Value stack;
        OH_JSVM_GetNamedProperty(env, error, "stack", &stack);

        JSVM_Value message;
        OH_JSVM_GetNamedProperty(env, error, "message", &message);

        char stackstr[256];
        OH_JSVM_GetValueStringUtf8(env, stack, stackstr, 256, nullptr);
        OH_LOG_INFO(LOG_APP, "xxx hippy JSVM error stack: %{public}s", stackstr);

        char messagestr[256];
        OH_JSVM_GetValueStringUtf8(env, message, messagestr, 256, nullptr);
        OH_LOG_INFO(LOG_APP, "xxx hippy JSVM error message: %{public}s", messagestr);
    }
    
    JSVM_Value result = nullptr;
    OH_JSVM_GetUndefined(env, &result);
    return result;
      
      
      
//     info.GetReturnValue().SetUndefined();
//     return;
  }
//   if (info.IsConstructCall()) {
//     auto internal_data = cb_info.GetData();
//     if (internal_data) {
//       holder->SetInternalField(kExternalIndex, v8::External::New(isolate, internal_data));
//       auto prototype = thiz->GetPrototype();
//       if (!prototype.IsEmpty()) {
//         auto prototype_object = v8::Local<v8::Object>::Cast(prototype);
//         if (prototype_object->InternalFieldCount() > 0) {
//           prototype_object->SetInternalField(kExternalIndex, v8::External::New(isolate, internal_data));
//         }
//       }
//     }
//   }
  
  FOOTSTONE_DLOG(INFO) << "xxx hippy, InvokeJsCallback, ret_value: " << ret_value->GetValue();
  
  return ret_value->GetValue();
}

void xx_Finalize(JSVM_Env env,
                                        void* finalizeData,
                                        void* finalizeHint) {
}

JSVM_Value InvokeJsCallback_Construct(JSVM_Env env, JSVM_CallbackInfo info) {
  size_t argc = 10; // TODO(hot-js):
  JSVM_Value args[10];
  JSVM_Value thisArg;
  void *data = nullptr;
  auto s = OH_JSVM_GetCbInfo(env, info, &argc, args, &thisArg, &data);
  FOOTSTONE_DCHECK(s == JSVM_OK);

  FOOTSTONE_DLOG(INFO) << "xxx hippy, InvokeJsCallback_Construct, thisArg: " << thisArg << ", argc: " << argc << ", data: " << data;
  
  CallbackInfo cb_info;
  cb_info.SetSlot(sGetAlignedPointerFromEmbedderData(kScopeWrapperIndex));

  
  void *external = sEmbedderExternalMap[thisArg];
  
    // TODO(hot-js):
    auto new_instance_external = (void*)external; //context->GetAlignedPointerFromEmbedderData(kNewInstanceExternalIndex);
    if (new_instance_external) {
      cb_info.SetData(new_instance_external);
    } else {
      cb_info.SetData((void*)1);
    }
  
  cb_info.SetReceiver(std::make_shared<JSHCtxValue>(env, thisArg));
  for (size_t i = 0; i < argc; i++) {
    cb_info.AddValue(std::make_shared<JSHCtxValue>(env, args[i]));
  }

  auto function_wrapper = reinterpret_cast<FunctionWrapper*>(data);
  auto js_cb = function_wrapper->callback;
  auto external_data = function_wrapper->data;
  
  js_cb(cb_info, external_data);

  FOOTSTONE_DLOG(INFO) << "xxx hippy, InvokeJsCallback_Construct, cb_info data: " << cb_info.GetData();

  auto ret_value = std::static_pointer_cast<JSHCtxValue>(cb_info.GetReturnValue()->Get());
  if (!ret_value) {
    
    auto error_ctx_value = std::static_pointer_cast<JSHCtxValue>(cb_info.GetExceptionValue()->Get());
    if (error_ctx_value) {
          auto error = error_ctx_value->GetValue();
    
        // 获取异常堆栈
        JSVM_Value stack;
        OH_JSVM_GetNamedProperty(env, error, "stack", &stack);

        JSVM_Value message;
        OH_JSVM_GetNamedProperty(env, error, "message", &message);

        char stackstr[256];
        OH_JSVM_GetValueStringUtf8(env, stack, stackstr, 256, nullptr);
        OH_LOG_INFO(LOG_APP, "xxx hippy JSVM error stack: %{public}s", stackstr);

        char messagestr[256];
        OH_JSVM_GetValueStringUtf8(env, message, messagestr, 256, nullptr);
        OH_LOG_INFO(LOG_APP, "xxx hippy JSVM error message: %{public}s", messagestr);
    }
    
    JSVM_Value result = nullptr;
    OH_JSVM_GetUndefined(env, &result);
    
    
    return result;
    
  }
  
  FOOTSTONE_DLOG(INFO) << "xxx hippy, InvokeJsCallback_Construct, ret_value: " << ret_value->GetValue();
  
//   JSVM_Value rett = nullptr;
//   auto st = OH_JSVM_CreateExternal(env, cb_info.GetData(), xx_Finalize, nullptr, &rett);
//   FOOTSTONE_DCHECK(st == JSVM_OK);
//   return rett;
  
  return ret_value->GetValue();
}

JSHCtx::JSHCtx(JSVM_VM vm) : vm_(vm) {
//     v8::HandleScope handle_scope(isolate);
//     v8::Local<v8::ObjectTemplate> global = v8::ObjectTemplate::New(isolate);
//     v8::Local<v8::Context> context = v8::Context::New(isolate, nullptr, global);
//     v8::Context::Scope contextScope(context);

//     context->SetAlignedPointerInEmbedderData(kExternalDataMapIndex, reinterpret_cast<void*>(&func_external_data_map_));

//     global_persistent_.Reset(isolate, global);
//     context_persistent_.Reset(isolate, context);
  
  // 新环境
  callBackStructs_ = new JSVM_CallbackStruct[5];
  
  // 注册用户提供的本地函数的回调函数指针和数据，通过JSVM-API暴露给js
  for (int i = 0; i < 5; i++) {
      callBackStructs_[i].data = nullptr;
  }
//   callBackStructs_[0].callback = Consoleinfo;
//   callBackStructs_[1].callback = Add;
//   callBackStructs_[2].callback = AssertEqual;
//   callBackStructs_[3].callback = OnJSResultCallback;
//   callBackStructs_[4].callback = CreatePromise;
  JSVM_PropertyDescriptor descriptors[] = {
//       {"consoleinfo", NULL, &callBackStructs_[0], NULL, NULL, NULL, JSVM_DEFAULT},
//       {"add", NULL, &callBackStructs_[1], NULL, NULL, NULL, JSVM_DEFAULT},
//       {"assertEqual", NULL, &callBackStructs_[2], NULL, NULL, NULL, JSVM_DEFAULT},
//       {"onJSResultCallback", NULL, &callBackStructs_[3], NULL, NULL, NULL, JSVM_DEFAULT},
//       {"createPromise", NULL, &callBackStructs_[4], NULL, NULL, NULL, JSVM_DEFAULT},
  };
  auto s = OH_JSVM_CreateEnv(vm_, sizeof(descriptors) / sizeof(descriptors[0]), descriptors, &env_);
  FOOTSTONE_DCHECK(s == JSVM_OK);
  s = OH_JSVM_OpenEnvScope(env_, &envScope_);
  FOOTSTONE_DCHECK(s == JSVM_OK);
}

std::shared_ptr<CtxValue> JSHCtx::CreateTemplate(const std::unique_ptr<FunctionWrapper>& wrapper) {
  // func_external_data_map_[reinterpret_cast<void*>(wrapper->cb)] = wrapper->data;
  // wrapper->cb is a function pointer which can be obtained at compile time
//   return v8::FunctionTemplate::New(isolate_, InvokeJsCallback,
//                                    v8::External::New(isolate_,
//                                                      reinterpret_cast<void*>(wrapper.get())));
  
  JSVM_CallbackStruct *param = new JSVM_CallbackStruct();
  param->data = wrapper.get();
  param->callback = InvokeJsCallback;

  JSVM_Value funcValue = nullptr;
  JSVM_Status status = OH_JSVM_CreateFunction(env_, nullptr, JSVM_AUTO_LENGTH, param, &funcValue);
  FOOTSTONE_DCHECK(status == JSVM_OK);
  return std::make_shared<JSHCtxValue>(env_, funcValue);
}

std::shared_ptr<CtxValue> JSHCtx::CreateFunction(const std::unique_ptr<FunctionWrapper>& wrapper) {
  
  JSHHandleScope handleScope(env_);
  
  return CreateTemplate(wrapper);
  
  
//   v8::HandleScope handle_scope(isolate_);
//   auto context = context_persistent_.Get(isolate_);
//   v8::Context::Scope context_scope(context);

//   auto function_template = CreateTemplate(wrapper);
//   return std::make_shared<V8CtxValue>(isolate_,
//                                       function_template->GetFunction(context).ToLocalChecked());
}

/*
class ExternalOneByteStringResourceImpl
    : public v8::String::ExternalOneByteStringResource {
 public:
  ExternalOneByteStringResourceImpl(const uint8_t* data, size_t length)
      : data_(data), length_(length) {}

  explicit ExternalOneByteStringResourceImpl(const std::string&& data)
      : data_(nullptr), str_data_(data) {
    length_ = str_data_.length();
  }

  ~ExternalOneByteStringResourceImpl() override = default;
  ExternalOneByteStringResourceImpl(const ExternalOneByteStringResourceImpl&) = delete;
  const ExternalOneByteStringResourceImpl& operator=(const ExternalOneByteStringResourceImpl&) = delete;

  const char* data() const override {
    if (data_) {
      return reinterpret_cast<const char*>(data_);
    } else {
      return str_data_.c_str();
    }
  }
  size_t length() const override { return length_; }

 private:
  const uint8_t* data_;
  std::string str_data_;
  size_t length_;
};
*/
/*
class ExternalStringResourceImpl : public v8::String::ExternalStringResource {
 public:
  ExternalStringResourceImpl(const uint16_t* data, size_t length)
      : data_(data), length_(length) {}

  explicit ExternalStringResourceImpl(const std::string&& data)
      : data_(nullptr), str_data_(data) {
    length_ = str_data_.length();
  }

  ~ExternalStringResourceImpl() override = default;
  ExternalStringResourceImpl(const ExternalStringResourceImpl&) = delete;
  const ExternalStringResourceImpl& operator=(const ExternalStringResourceImpl&) = delete;

  const uint16_t* data() const override {
    if (data_) {
      return data_;
    } else {
      return reinterpret_cast<const uint16_t*>(str_data_.c_str());
    }
  }

  size_t length() const override { return length_ / 2; }

 private:
  const uint16_t* data_;
  const std::string str_data_;
  size_t length_;
};
*/

void JSHCtx::SetExternalData(void* address) {
  SetAlignedPointerInEmbedderData(kScopeWrapperIndex, reinterpret_cast<intptr_t>(address));
}

std::shared_ptr<ClassDefinition> JSHCtx::GetClassDefinition(const string_view& name) {
  FOOTSTONE_DCHECK(template_map_.find(name) != template_map_.end());
  return template_map_[name];
}

void JSHCtx::SetAlignedPointerInEmbedderData(int index, intptr_t address) {
  JSHHandleScope handleScope(env_);
  
  sEmbedderDataMap[index] = reinterpret_cast<void*>(address);
  
//   v8::HandleScope handle_scope(isolate_);
//   auto context = context_persistent_.Get(isolate_);
//   context->SetAlignedPointerInEmbedderData(index,
//                                            reinterpret_cast<void*>(address));
}

// std::string JSHCtx::GetSerializationBuffer(const std::shared_ptr<CtxValue>& value,
//                                           std::string& reused_buffer) {
//   v8::HandleScope handle_scope(isolate_);
//   v8::Local<v8::Context> context = context_persistent_.Get(isolate_);
//   v8::Context::Scope context_scope(context);
//   std::shared_ptr<V8CtxValue> ctx_value = std::static_pointer_cast<V8CtxValue>(value);
//   v8::Local<v8::Value> handle_value = v8::Local<v8::Value>::New(isolate_, ctx_value->global_value_);
//
//   Serializer serializer(isolate_, context, reused_buffer);
//   serializer.WriteHeader();
//   serializer.WriteValue(handle_value);
//   std::pair<uint8_t*, size_t> pair = serializer.Release();
//   return {reinterpret_cast<const char*>(pair.first), pair.second};
// }

std::shared_ptr<CtxValue> JSHCtx::GetGlobalObject() {
//   v8::HandleScope handle_scope(isolate_);
//   v8::Local<v8::Context> context = context_persistent_.Get(isolate_);
//   v8::Context::Scope context_scope(context);
  
  JSHHandleScope handleScope(env_);
  
  JSVM_Value global = 0;
  JSVM_Status status = OH_JSVM_GetGlobal(env_, &global);
  FOOTSTONE_DCHECK(status == JSVM_OK);

  return std::make_shared<JSHCtxValue>(env_, global);
}

std::shared_ptr<CtxValue> JSHCtx::GetProperty(
    const std::shared_ptr<CtxValue>& object,
    const string_view& name) {
  FOOTSTONE_CHECK(object);
//   v8::HandleScope handle_scope(isolate_);
//   v8::Local<v8::Context> context = context_persistent_.Get(isolate_);
//   v8::Context::Scope context_scope(context);
  
  JSHHandleScope handleScope(env_);

  auto key = JSHVM::CreateV8String(env_, name);
  return GetProperty(object, key);
}

std::shared_ptr<CtxValue> JSHCtx::GetProperty(
    const std::shared_ptr<CtxValue>& object,
    std::shared_ptr<CtxValue> key) {
  FOOTSTONE_CHECK(object && key);
//   v8::HandleScope handle_scope(isolate_);
//   v8::Local<v8::Context> context = context_persistent_.Get(isolate_);
//   v8::Context::Scope context_scope(context);
  
  JSHHandleScope handleScope(env_);

  auto jsh_object = std::static_pointer_cast<JSHCtxValue>(object);
//   auto v8_object_handle = v8::Local<v8::Value>::New(isolate_, v8_object->global_value_);

  auto jsh_key = std::static_pointer_cast<JSHCtxValue>(key);
//   auto v8_key_handle = v8::Local<v8::Value>::New(isolate_, v8_key->global_value_);
  
  JSVM_Value result = 0;
  auto s = OH_JSVM_GetProperty(env_, jsh_object->GetValue(), jsh_key->GetValue(), &result);
  FOOTSTONE_DCHECK(s == JSVM_OK);

  return std::make_shared<JSHCtxValue>(env_, result);
}

std::shared_ptr<CtxValue> JSHCtx::RunScript(const string_view& str_view,
                                           const string_view& file_name) {
  return RunScript(str_view, file_name, false, nullptr, true);
}

std::shared_ptr<CtxValue> JSHCtx::RunScript(const string_view& str_view,
                                           const string_view& file_name,
                                           bool is_use_code_cache,
                                           string_view* cache,
                                           bool is_copy) {
  FOOTSTONE_LOG(INFO) << "JSHCtx::RunScript file_name = " << file_name
                      << ", is_use_code_cache = " << is_use_code_cache
                      << ", cache = " << cache << ", is_copy = " << is_copy;
//   v8::HandleScope handle_scope(isolate_);
//   v8::Local<v8::Context> context = context_persistent_.Get(isolate_);
//   v8::Context::Scope context_scope(context);
//   v8::MaybeLocal<v8::String> source;
  
  JSHHandleScope handleScope(env_);
  
  auto source_value = JSHVM::CreateV8String(env_, str_view);
  
  /*
  string_view::Encoding encoding = str_view.encoding();
  switch (encoding) {
    case string_view::Encoding::Latin1: {
      const std::string& str = str_view.latin1_value();
      if (is_copy) {
        source = v8::String::NewFromOneByte(
            isolate_,
            reinterpret_cast<const uint8_t*>(str.c_str()),
            v8::NewStringType::kInternalized,
            footstone::checked_numeric_cast<size_t, int>(str.length()));
      } else {
        auto* one_byte =
            new ExternalOneByteStringResourceImpl(
                reinterpret_cast<const uint8_t*>(str.c_str()), str.length());
        source = v8::String::NewExternalOneByte(isolate_, one_byte);
      }
      break;
    }
    case string_view::Encoding::Utf16: {
      const std::u16string& str = str_view.utf16_value();
      if (is_copy) {
        source = v8::String::NewFromTwoByte(
            isolate_,
            reinterpret_cast<const uint16_t*>(str.c_str()),
            v8::NewStringType::kNormal,
            footstone::checked_numeric_cast<size_t, int>(str.length()));
      } else {
        auto* two_byte = new ExternalStringResourceImpl(
            reinterpret_cast<const uint16_t*>(str.c_str()), str.length());
        source = v8::String::NewExternalTwoByte(isolate_, two_byte);
      }
      break;
    }
    case string_view::Encoding::Utf32: {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
      const std::u32string& str = str_view.utf32_value();
      std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> convert;
      std::string bytes = convert.to_bytes(str);
      std::u16string two_byte(reinterpret_cast<const char16_t*>(bytes.c_str()),
                              bytes.length() / sizeof(char16_t));
      source = v8::String::NewFromTwoByte(
          isolate_, reinterpret_cast<const uint16_t*>(str.c_str()),
          v8::NewStringType::kNormal, footstone::checked_numeric_cast<size_t, int>(str.length()));
#pragma clang diagnostic pop
      break;
    }
    case string_view::Encoding::Utf8: {
      const string_view::u8string& str = str_view.utf8_value();
      source = v8::String::NewFromUtf8(
          isolate_, reinterpret_cast<const char*>(str.c_str()),
          v8::NewStringType::kNormal);
      break;
    }
    default: {
      FOOTSTONE_UNREACHABLE();
    }
  }
  */

  if (!source_value) {
    FOOTSTONE_DLOG(WARNING) << "jsh_source empty, file_name = " << file_name;
    return nullptr;
  }

  return InternalRunScript(source_value, file_name, is_use_code_cache, cache);
}

// void JSHCtx::SetDefaultContext(const std::shared_ptr<v8::SnapshotCreator>& creator) {
//   FOOTSTONE_CHECK(creator);
//   v8::HandleScope handle_scope(isolate_);
//   auto context = context_persistent_.Get(isolate_);
//   creator->SetDefaultContext(context);
// }

std::shared_ptr<CtxValue> JSHCtx::InternalRunScript(
    std::shared_ptr<CtxValue> &source_value,
    const string_view& file_name,
    bool is_use_code_cache,
    string_view* cache) {
  /*
  v8::Local<v8::String> v8_file_name = V8VM::CreateV8String(isolate_, context, file_name);
#if (V8_MAJOR_VERSION == 8 && V8_MINOR_VERSION == 9 && \
     V8_BUILD_NUMBER >= 45) || \
    (V8_MAJOR_VERSION == 8 && V8_MINOR_VERSION > 9) || (V8_MAJOR_VERSION > 8)
  v8::ScriptOrigin origin(isolate_, v8_file_name);
#else
  v8::ScriptOrigin origin(v8_file_name);
#endif
  v8::MaybeLocal<v8::Script> script;
  if (is_use_code_cache && cache && !StringViewUtils::IsEmpty(*cache)) {
    string_view::Encoding encoding = cache->encoding();
    if (encoding == string_view::Encoding::Utf8) {
      const string_view::u8string& str = cache->utf8_value();
      auto* cached_data =
          new v8::ScriptCompiler::CachedData(
              str.c_str(), footstone::checked_numeric_cast<size_t, int>(str.length()),
              v8::ScriptCompiler::CachedData::BufferNotOwned);
      v8::ScriptCompiler::Source script_source(source, origin, cached_data);
      script = v8::ScriptCompiler::Compile(
          context, &script_source, v8::ScriptCompiler::kConsumeCodeCache);
    } else {
      FOOTSTONE_UNREACHABLE();
    }
  } else {
    if (is_use_code_cache && cache) {
      v8::ScriptCompiler::Source script_source(source, origin);
      script = v8::ScriptCompiler::Compile(context, &script_source);
      if (script.IsEmpty()) {
        return nullptr;
      }
      const v8::ScriptCompiler::CachedData* cached_data =
          v8::ScriptCompiler::CreateCodeCache(
              script.ToLocalChecked()->GetUnboundScript());
      *cache = string_view(cached_data->data,
                                   footstone::checked_numeric_cast<int,
                                                                     size_t>(cached_data->length));
    } else {
      script = v8::Script::Compile(context, source, &origin);
    }
  }
  //*/
  
  JSHHandleScope handleScope(env_);
  
  auto jsh_source_value = std::static_pointer_cast<JSHCtxValue>(source_value);
  
  JSVM_Script script;
  JSVM_Status status = OH_JSVM_CompileScript(env_, jsh_source_value->GetValue(), nullptr, 0, true, nullptr, &script);
  if (status == JSVM_PENDING_EXCEPTION) {
    JSVM_Value error;
    if (JSVM_OK == OH_JSVM_GetAndClearLastException((env_), &error))
    {
        // 获取异常堆栈
        JSVM_Value stack;
        OH_JSVM_GetNamedProperty((env_), error, "stack", &stack);

        JSVM_Value message;
        OH_JSVM_GetNamedProperty((env_), error, "message", &message);

        char stackstr[256];
        OH_JSVM_GetValueStringUtf8(env_, stack, stackstr, 256, nullptr);
        OH_LOG_INFO(LOG_APP, "xxx hippy JSVM error stack: %{public}s", stackstr);

        char messagestr[256];
        OH_JSVM_GetValueStringUtf8(env_, message, messagestr, 256, nullptr);
        OH_LOG_INFO(LOG_APP, "xxx hippy JSVM error message: %{public}s", messagestr);
    }
      
    FOOTSTONE_DCHECK(status == JSVM_OK);
      
      
    JSVM_Value errorResult = nullptr;
    auto s = OH_JSVM_GetAndClearLastException(env_, &errorResult);
    FOOTSTONE_DCHECK(s == JSVM_OK);
    
    JSVM_ValueType valuetype = (JSVM_ValueType)100;
    status = OH_JSVM_Typeof(env_, errorResult, &valuetype);
    FOOTSTONE_DCHECK(status == JSVM_OK);
    FOOTSTONE_DLOG(INFO) << "xxx hippy error, result type, " << valuetype;
    if (valuetype == 6) {
      auto obj = errorResult;
      JSVM_Value propNames = nullptr;
      OH_JSVM_GetAllPropertyNames(env_, obj, JSVM_KEY_OWN_ONLY,
                                static_cast<JSVM_KeyFilter>(JSVM_KEY_ENUMERABLE | JSVM_KEY_SKIP_SYMBOLS),
                                JSVM_KEY_NUMBERS_TO_STRINGS, &propNames);
    
      bool isArray = false;
      OH_JSVM_IsArray(env_, propNames, &isArray);
      if (!isArray) {
        return nullptr;
      }
    
      uint32_t arrayLength = 0;
      OH_JSVM_GetArrayLength(env_, propNames, &arrayLength);
      if (!arrayLength) {
        return nullptr;
      }
    
      for (uint32_t i = 0; i < arrayLength; i++)
      {
        bool hasElement = false;
        OH_JSVM_HasElement(env_, propNames, i, &hasElement);
        if (!hasElement) {
          continue;
        }
    
        JSVM_Value propName = nullptr;
        OH_JSVM_GetElement(env_, propNames, i, &propName);
    
        bool hasProp = false;
        OH_JSVM_HasProperty(env_, obj, propName, &hasProp);
        if (!hasProp) {
          continue;
        }
    
        JSVM_Value propValue = nullptr;
        OH_JSVM_GetProperty(env_, obj, propName, &propValue);
        
        char buf[200] = {0};
        size_t resultLen = 0;
        OH_JSVM_GetValueStringUtf8(env_, propName, buf, 200, &resultLen);
        
        char buf2[200] = {0};
        size_t resultLen2 = 0;
        OH_JSVM_GetValueStringUtf8(env_, propValue, buf2, 200, &resultLen2);
        
        FOOTSTONE_DLOG(INFO) << "xxx hippy error, result pair, " << buf << " : " << buf2;
        
      }
    }
  }
  FOOTSTONE_DCHECK(status == JSVM_OK);
  
  JSVM_Value result;
  status = OH_JSVM_RunScript(env_, script, &result);
  FOOTSTONE_DCHECK(status == JSVM_OK);
  
  JSVM_ValueType valuetype = (JSVM_ValueType)100;
  status = OH_JSVM_Typeof(env_, result, &valuetype);
  FOOTSTONE_DCHECK(status == JSVM_OK);
  FOOTSTONE_DLOG(INFO) << "xxx hippy run script, result type, " << valuetype;
  
  if (valuetype == 3) {
    double tt = 0;
    OH_JSVM_GetValueDouble(env_, result, &tt);
    FOOTSTONE_DLOG(INFO) << "xxx hippy run script, result value, " << tt;
  }

//   if (script.IsEmpty()) {
//     return nullptr;
//   }
//
//   v8::MaybeLocal<v8::Value> v8_maybe_value = script.ToLocalChecked()->Run(context);
//   if (v8_maybe_value.IsEmpty()) {
//     return nullptr;
//   }
//   v8::Local<v8::Value> v8_value = v8_maybe_value.ToLocalChecked();
  
  return std::make_shared<JSHCtxValue>(env_, result);
}

void JSHCtx::ThrowException(const std::shared_ptr<CtxValue>& exception) {
  FOOTSTONE_DCHECK(false);
  JSHHandleScope handleScope(env_);
  
//   v8::HandleScope handle_scope(isolate_);
//   auto context = context_persistent_.Get(isolate_);
//   v8::Context::Scope context_scope(context);
//
//   auto ctx_value = std::static_pointer_cast<V8CtxValue>(exception);
//   auto handle_value = v8::Local<v8::Value>::New(isolate_, ctx_value->global_value_);
//   isolate_->ThrowException(handle_value);
}

void JSHCtx::ThrowException(const string_view& exception) {
  ThrowException(CreateException(exception));
}

std::shared_ptr<CtxValue> JSHCtx::CallFunction(
    const std::shared_ptr<CtxValue>& function,
    const std::shared_ptr<CtxValue>& receiver,
    size_t argument_count,
    const std::shared_ptr<CtxValue> arguments[]) {
  if (!function) {
    FOOTSTONE_LOG(ERROR) << "function is nullptr";
    return nullptr;
  }
  
  JSHHandleScope handleScope(env_);

//   v8::HandleScope handle_scope(isolate_);
//   auto context = context_persistent_.Get(isolate_);
//   v8::Context::Scope contextScope(context);
//   if (context.IsEmpty() || context->Global().IsEmpty()) {
//     FOOTSTONE_LOG(ERROR) << "CallFunction context error";
//     return nullptr;
//   }

  auto ctx_value = std::static_pointer_cast<JSHCtxValue>(function);
//   auto handle_value = v8::Local<v8::Value>::New(isolate_, ctx_value->global_value_);
//   if (!handle_value->IsFunction()) {
//     FOOTSTONE_LOG(WARNING) << "CallFunction handle_value is not a function";
//     return nullptr;
//   }

//   auto v8_fn = v8::Function::Cast(*handle_value);
//   v8::Local<v8::Value> args[argument_count];
  JSVM_Value args[argument_count];
  for (size_t i = 0; i < argument_count; i++) {
    auto argument = std::static_pointer_cast<JSHCtxValue>(arguments[i]);
    if (argument) {
//       args[i] = v8::Local<v8::Value>::New(isolate_, argument->global_value_);
      args[i] = argument->GetValue();
    } else {
      FOOTSTONE_LOG(WARNING) << "CallFunction argument error, i = " << i;
      return nullptr;
    }
  }

  auto receiver_object = std::static_pointer_cast<JSHCtxValue>(receiver);
//   auto handle_object = v8::Local<v8::Value>::New(isolate_, receiver_object->global_value_);
//   v8::MaybeLocal<v8::Value> maybe_result = v8_fn->Call(
//       context, handle_object, static_cast<int>(argument_count), args);
  
  FOOTSTONE_DLOG(INFO) << "xxx hippy, CallFunction, receiver_object: " << receiver_object->GetValue() << ", argc: " << argument_count;
  
  JSVM_Value result = 0;
  auto s = OH_JSVM_CallFunction(env_, receiver_object->GetValue(), ctx_value->GetValue(), argument_count, args, &result);
  CheckPendingExeception(env_, s);
  FOOTSTONE_DCHECK(s == JSVM_OK);
  
  FOOTSTONE_DLOG(INFO) << "xxx hippy, CallFunction, after, result: " << result;

  if (!result) {
    FOOTSTONE_DLOG(INFO) << "maybe_result is empty";
    return nullptr;
  }
  return std::make_shared<JSHCtxValue>(env_, result);
}

std::shared_ptr<CtxValue> JSHCtx::CreateNumber(double number) {
//   v8::HandleScope isolate_scope(isolate_);
//
//   auto v8_number = v8::Number::New(isolate_, number);
//   if (v8_number.IsEmpty()) {
//     return nullptr;
//   }
  JSHHandleScope handleScope(env_);
  
  JSVM_Value value = 0;
  auto s = OH_JSVM_CreateDouble(env_, number, &value);
  FOOTSTONE_DCHECK(s == JSVM_OK);
  return std::make_shared<JSHCtxValue>(env_, value);
}

std::shared_ptr<CtxValue> JSHCtx::CreateBoolean(bool b) {
//   v8::HandleScope isolate_scope(isolate_);
//
//   auto v8_boolean = v8::Boolean::New(isolate_, b);
//   if (v8_boolean.IsEmpty()) {
//     return nullptr;
//   }
  
  JSHHandleScope handleScope(env_);
  
  JSVM_Value value = 0;
  auto s = OH_JSVM_CreateInt32(env_, b ? 1 : 0, &value); // TODO(hot-js):
  FOOTSTONE_DCHECK(s == JSVM_OK);
  return std::make_shared<JSHCtxValue>(env_, value);
}

std::shared_ptr<CtxValue> JSHCtx::CreateString(const string_view& str_view) {
  if (str_view.encoding() == string_view::Encoding::Unknown) {
    return nullptr;
  }
  
  JSHHandleScope handleScope(env_);
  
//   v8::HandleScope isolate_scope(isolate_);
//   auto context = context_persistent_.Get(isolate_);
//   v8::Context::Scope context_scope(context);
//
//   auto v8_string = V8VM::CreateV8String(isolate_, context, str_view);
  
  auto value = JSHVM::CreateV8String(env_, str_view);
  return value;
}

std::shared_ptr<CtxValue> JSHCtx::CreateUndefined() {
  JSHHandleScope handleScope(env_);
  
//   v8::HandleScope isolate_scope(isolate_);
//
//   auto undefined = v8::Undefined(isolate_);
//   if (undefined.IsEmpty()) {
//     return nullptr;
//   }
  JSVM_Value value = 0;
  auto s = OH_JSVM_GetUndefined(env_, &value);
  FOOTSTONE_DCHECK(s == JSVM_OK);
  return std::make_shared<JSHCtxValue>(env_, value);
}

std::shared_ptr<CtxValue> JSHCtx::CreateNull() {
  JSHHandleScope handleScope(env_);
  
//   v8::HandleScope isolate_scope(isolate_);
//
//   auto v8_null = v8::Null(isolate_);
//   if (v8_null.IsEmpty()) {
//     return nullptr;
//   }
  JSVM_Value value = 0;
  auto s = OH_JSVM_GetNull(env_, &value);
  FOOTSTONE_DCHECK(s == JSVM_OK);
  return std::make_shared<JSHCtxValue>(env_, value);
}

std::shared_ptr<CtxValue> JSHCtx::CreateObject(const std::unordered_map<
    string_view,
    std::shared_ptr<CtxValue>>& object) {
  std::unordered_map<std::shared_ptr<CtxValue>, std::shared_ptr<CtxValue>> obj;
  for (const auto& it: object) {
    auto key = CreateString(it.first);
    obj[key] = it.second;
  }
  return CreateObject(obj);
}

std::shared_ptr<CtxValue> JSHCtx::CreateObject(const std::unordered_map<
    std::shared_ptr<CtxValue>,
    std::shared_ptr<CtxValue>>& object) {
  
//   v8::HandleScope handle_scope(isolate_);
//   auto context = context_persistent_.Get(isolate_);
//   v8::Context::Scope context_scope(context);

//   v8::Local<v8::Object> obj = v8::Object::New(isolate_);
//   for (const auto& it: object) {
//     auto key_ctx_value = std::static_pointer_cast<V8CtxValue>(it.first);
//     auto key_handle_value = v8::Local<v8::Value>::New(isolate_, key_ctx_value->global_value_);
//     auto value_ctx_value = std::static_pointer_cast<V8CtxValue>(it.second);
//     auto value_handle_value = v8::Local<v8::Value>::New(isolate_, value_ctx_value->global_value_);
//     obj->Set(context, key_handle_value, value_handle_value).ToChecked();
//   }
  
  JSHHandleScope handleScope(env_);
  
  JSVM_Value obj = nullptr;
  auto s = OH_JSVM_CreateObject(env_, &obj);
  FOOTSTONE_DCHECK(s == JSVM_OK);
  
  for (const auto& it: object) {
    auto key_ctx_value = std::static_pointer_cast<JSHCtxValue>(it.first);
    
    auto value_ctx_value = std::static_pointer_cast<JSHCtxValue>(it.second);
    
    s = OH_JSVM_SetProperty(env_, obj, key_ctx_value->GetValue(), value_ctx_value->GetValue());
    FOOTSTONE_DCHECK(s == JSVM_OK);
  }
  
  return std::make_shared<JSHCtxValue>(env_, obj);
}

std::shared_ptr<CtxValue> JSHCtx::CreateArray(
    size_t count,
    std::shared_ptr<CtxValue> value[]) {
  int array_size;
  if (!footstone::numeric_cast<size_t, int>(count, array_size)) {
    FOOTSTONE_LOG(ERROR) << "array length out of boundary";
    return nullptr;
  }
//   v8::HandleScope handle_scope(isolate_);
//   v8::Local<v8::Context> context = context_persistent_.Get(isolate_);
//   v8::Context::Scope context_scope(context);

//   v8::Local<v8::Array> array = v8::Array::New(isolate_, array_size);
//   for (auto i = 0; i < array_size; i++) {
//     v8::Local<v8::Value> handle_value;
//     std::shared_ptr<V8CtxValue> ctx_value = std::static_pointer_cast<V8CtxValue>(value[i]);
//     if (ctx_value) {
//       handle_value = v8::Local<v8::Value>::New(isolate_, ctx_value->global_value_);
//     } else {
//       FOOTSTONE_LOG(ERROR) << "array item error";
//       return nullptr;
//     }
//     if (!array->Set(context, static_cast<uint32_t >(i), handle_value).FromMaybe(false)) {
//       FOOTSTONE_LOG(ERROR) << "set array item failed";
//       return nullptr;
//     }
//   }
  
  JSHHandleScope handleScope(env_);
  
  JSVM_Value array = 0;
  auto s = OH_JSVM_CreateArrayWithLength(env_, (size_t)array_size, &array);
  FOOTSTONE_DCHECK(s == JSVM_OK);
  for (auto i = 0; i < array_size; i++) {
    std::shared_ptr<JSHCtxValue> ctx_value = std::static_pointer_cast<JSHCtxValue>(value[i]);
    s = OH_JSVM_SetElement(env_, array, (uint32_t)i, ctx_value->GetValue());
    FOOTSTONE_DCHECK(s == JSVM_OK);
  }
  
  return std::make_shared<JSHCtxValue>(env_, array);
}

std::shared_ptr<CtxValue> JSHCtx::CreateMap(const std::map<
    std::shared_ptr<CtxValue>,
    std::shared_ptr<CtxValue>>& map) {

//   v8::HandleScope handle_scope(isolate_);
//   v8::Local<v8::Context> context = context_persistent_.Get(isolate_);
//   v8::Context::Scope context_scope(context);

//   v8::Local<v8::Map> js_map = v8::Map::New(isolate_);
//   for (auto& it: map) {
//     auto key_ctx_value = std::static_pointer_cast<V8CtxValue>(it.first);
//     auto key_handle_value = v8::Local<v8::Value>::New(isolate_, key_ctx_value->global_value_);
//     auto value_ctx_value = std::static_pointer_cast<V8CtxValue>(it.second);
//     auto value_handle_value = v8::Local<v8::Value>::New(isolate_, value_ctx_value->global_value_);
//     js_map->Set(context, key_handle_value, value_handle_value).ToLocalChecked();
//   }
//   return std::make_shared<JSHCtxValue>(env_, js_map);
  
  JSHHandleScope handleScope(env_);
  
  JSVM_Value obj = nullptr;
  auto s = OH_JSVM_CreateObject(env_, &obj); // TODO(hot-js):
  FOOTSTONE_DCHECK(s == JSVM_OK);
  
  for (const auto& it: map) {
    auto key_ctx_value = std::static_pointer_cast<JSHCtxValue>(it.first);
    
    auto value_ctx_value = std::static_pointer_cast<JSHCtxValue>(it.second);
    
    s = OH_JSVM_SetProperty(env_, obj, key_ctx_value->GetValue(), value_ctx_value->GetValue());
    FOOTSTONE_DCHECK(s == JSVM_OK);
  }
  return std::make_shared<JSHCtxValue>(env_, obj);
}

std::shared_ptr<CtxValue> JSHCtx::CreateException(const string_view& msg) {
  FOOTSTONE_DLOG(INFO) << "JSHCtx::CreateException msg = " << msg;
//   v8::HandleScope handle_scope(isolate_);
//   auto context = context_persistent_.Get(isolate_);
//   v8::Context::Scope context_scope(context);

//   auto error = v8::Exception::Error(V8VM::CreateV8String(isolate_, context, msg));
//   if (error.IsEmpty()) {
//     FOOTSTONE_LOG(INFO) << "error is empty";
//     return nullptr;
//   }
  
  JSHHandleScope handleScope(env_);
  
  JSVM_Value code = nullptr;
  auto msg_value = JSHVM::CreateV8String(env_, msg);
  auto jsh_msg_value = std::static_pointer_cast<JSHCtxValue>(msg_value);
  JSVM_Value error = nullptr;
  auto s = OH_JSVM_CreateError(env_, code, jsh_msg_value->GetValue(), &error);
  FOOTSTONE_DCHECK(s == JSVM_OK);
  
  return std::make_shared<JSHCtxValue>(env_, error);
}

// #if V8_MAJOR_VERSION >= 9
// static void ArrayBufferDataDeleter(void* data, size_t length, void* deleter_data) {
//   free(data);
// }
// #endif //V8_MAJOR_VERSION >= 9

std::shared_ptr<CtxValue> JSHCtx::CreateByteBuffer(void* buffer, size_t length) {
  if (!buffer) {
    return nullptr;
  }
//   v8::HandleScope handle_scope(isolate_);
//   v8::Local<v8::Context> context = context_persistent_.Get(isolate_);
//   v8::Context::Scope context_scope(context);
// #if V8_MAJOR_VERSION < 9
//   v8::Local<v8::ArrayBuffer> array_buffer = v8::ArrayBuffer::New(isolate_, buffer, length, v8::ArrayBufferCreationMode::kInternalized);
// #else
//   auto backingStore = v8::ArrayBuffer::NewBackingStore(buffer, length, ArrayBufferDataDeleter,
//                                                        nullptr);
//   v8::Local<v8::ArrayBuffer> array_buffer = v8::ArrayBuffer::New(isolate_, std::move(backingStore));
// #endif //V8_MAJOR_VERSION >= 9
//
//   if (array_buffer.IsEmpty()) {
//     FOOTSTONE_LOG(ERROR) << "array_buffer is empty";
//     return nullptr;
//   }
  
  JSHHandleScope handleScope(env_);
  
  JSVM_Value arrayBuffer = nullptr;
  auto s = OH_JSVM_CreateArraybuffer(env_, length, &buffer, &arrayBuffer);
  FOOTSTONE_DCHECK(s == JSVM_OK);

  return std::make_shared<JSHCtxValue>(env_, arrayBuffer);
}

bool JSHCtx::GetValueNumber(const std::shared_ptr<CtxValue>& value, double* result) {
  if (!value || !result) {
    return false;
  }
//   v8::HandleScope handle_scope(isolate_);
//   auto context = context_persistent_.Get(isolate_);
//   v8::Context::Scope context_scope(context);
  JSHHandleScope handleScope(env_);
  auto ctx_value = std::static_pointer_cast<JSHCtxValue>(value);
//   auto handle_value = v8::Local<v8::Value>::New(isolate_, ctx_value->global_value_);

//   if (handle_value.IsEmpty() || !handle_value->IsNumber()) {
//     return false;
//   }

//   auto number = handle_value->ToNumber(context).ToLocalChecked();
//   if (number.IsEmpty()) {
//     return false;
//   }

//   *result = number->Value();
  
  auto s = OH_JSVM_GetValueDouble(env_, ctx_value->GetValue(), result);
  FOOTSTONE_DCHECK(s == JSVM_OK);
  
  return true;
}

bool JSHCtx::GetValueNumber(const std::shared_ptr<CtxValue>& value, int32_t* result) {
  if (!value || !result) {
    return false;
  }
//   v8::HandleScope handle_scope(isolate_);
//   v8::Local<v8::Context> context = context_persistent_.Get(isolate_);
//   v8::Context::Scope context_scope(context);
  JSHHandleScope handleScope(env_);
  std::shared_ptr<JSHCtxValue> ctx_value = std::static_pointer_cast<JSHCtxValue>(value);
//   const v8::Global<v8::Value>& global_value = ctx_value->global_value_;
//   v8::Local<v8::Value> handle_value =
//       v8::Local<v8::Value>::New(isolate_, global_value);
//
//   if (handle_value.IsEmpty() || !handle_value->IsInt32()) {
//     return false;
//   }
//
//   v8::Local<v8::Int32> number = handle_value->ToInt32(context).ToLocalChecked();
//   *result = number->Value();
  if (!IsNumber(ctx_value)) {
    return false;
  }
  
  auto s = OH_JSVM_GetValueInt32(env_, ctx_value->GetValue(), result);
  FOOTSTONE_DCHECK(s == JSVM_OK);
  return true;
}

bool JSHCtx::GetValueBoolean(const std::shared_ptr<CtxValue>& value, bool* result) {
  if (!value || !result) {
    return false;
  }
//   v8::HandleScope handle_scope(isolate_);
//   v8::Local<v8::Context> context = context_persistent_.Get(isolate_);
//   v8::Context::Scope context_scope(context);
  JSHHandleScope handleScope(env_);
  std::shared_ptr<JSHCtxValue> ctx_value = std::static_pointer_cast<JSHCtxValue>(value);
//   const v8::Global<v8::Value>& global_value = ctx_value->global_value_;
//   v8::Local<v8::Value> handle_value =
//       v8::Local<v8::Value>::New(isolate_, global_value);
//
//   if (handle_value.IsEmpty() ||
//       (!handle_value->IsBoolean() && !handle_value->IsBooleanObject())) {
//     return false;
//   }
//
//   v8::Local<v8::Boolean> boolean = handle_value->ToBoolean(isolate_);
//   *result = boolean->Value();
  auto s = OH_JSVM_GetValueBool(env_, ctx_value->GetValue(), result);
  FOOTSTONE_DCHECK(s == JSVM_OK);
  return true;
}

bool JSHCtx::GetValueString(const std::shared_ptr<CtxValue>& value,
                           string_view* result) {
  if (!value || !result) {
    return false;
  }
  std::shared_ptr<JSHCtxValue> ctx_value = std::static_pointer_cast<JSHCtxValue>(value);
//   v8::HandleScope handle_scope(isolate_);
//   v8::Local<v8::Context> context = context_persistent_.Get(isolate_);
//   v8::Context::Scope context_scope(context);
//   const v8::Global<v8::Value>& global_value = ctx_value->global_value_;
//   auto handle_value = v8::Local<v8::Value>::New(isolate_, global_value);
//   if (handle_value.IsEmpty()) {
//     return false;
//   }

//   if (handle_value->IsString() || handle_value->IsStringObject()) {
//     *result = V8VM::ToStringView(isolate_, context, handle_value->ToString(context).ToLocalChecked());
//     return true;
//   }
  JSHHandleScope handleScope(env_);
  *result = JSHVM::ToStringView(env_, ctx_value->GetValue());
  return true;
}

bool JSHCtx::GetValueJson(const std::shared_ptr<CtxValue>& value,
                         string_view* result) {
  if (!value || !result) {
    return false;
  }
//   v8::HandleScope handle_scope(isolate_);
//   v8::Local<v8::Context> context = context_persistent_.Get(isolate_);
//   v8::Context::Scope context_scope(context);
  JSHHandleScope handleScope(env_);
  std::shared_ptr<JSHCtxValue> ctx_value = std::static_pointer_cast<JSHCtxValue>(value);
//   const v8::Global<v8::Value>& global_value = ctx_value->global_value_;
//   v8::Local<v8::Value> handle_value =
//       v8::Local<v8::Value>::New(isolate_, global_value);
//   if (handle_value.IsEmpty() || !handle_value->IsObject()) {
//     return false;
//   }
  
  JSVM_Value jsonResult;
  auto status = OH_JSVM_JsonStringify(env_, ctx_value->GetValue(), &jsonResult);
  FOOTSTONE_DCHECK(status == JSVM_OK);
  
//   v8::MaybeLocal<v8::String> v8_maybe_string =
//       v8::JSON::Stringify(context, handle_value);
//   if (v8_maybe_string.IsEmpty()) {
//     return false;
//   }

//   auto v8_string = v8_maybe_string.ToLocalChecked();
  *result = JSHVM::ToStringView(env_, jsonResult);
  return true;
}

bool JSHCtx::GetEntriesFromObject(const std::shared_ptr<CtxValue>& value,
                                 std::unordered_map<std::shared_ptr<CtxValue>, std::shared_ptr<CtxValue>>& map) {
//   v8::HandleScope handle_scope(isolate_);
//   auto context = context_persistent_.Get(isolate_);

  JSHHandleScope handleScope(env_);
  auto ctx_value = std::static_pointer_cast<JSHCtxValue>(value);
//   auto handle_value = v8::Local<v8::Value>::New(isolate_, ctx_value->global_value_);
//   auto handle_object = v8::Local<v8::Object>::Cast(handle_value);

  /*
  auto property_names = handle_object->GetPropertyNames(context);
  if (property_names.IsEmpty()) {
    return false;
  }
  auto names = property_names.ToLocalChecked();
  for (uint32_t i = 0; i < names->Length(); ++i) {
    auto maybe_key = names->Get(context, i);
    FOOTSTONE_DCHECK(!maybe_key.IsEmpty());
    if (maybe_key.IsEmpty()) {
      continue;
    }
    auto key = maybe_key.ToLocalChecked();
    auto maybe_value= handle_object->Get(context, key);
    FOOTSTONE_DCHECK(!maybe_value.IsEmpty());
    if (maybe_value.IsEmpty()) {
      continue;
    }
    map[std::make_shared<V8CtxValue>(isolate_, maybe_key.ToLocalChecked())] = std::make_shared<V8CtxValue>(
        isolate_, maybe_value.ToLocalChecked());
  }
  //*/
  
  auto obj = ctx_value->GetValue();
  
  JSVM_Value propNames = nullptr;
  auto s = OH_JSVM_GetPropertyNames(env_, obj, &propNames);
  FOOTSTONE_DCHECK(s == JSVM_OK);

  bool isArray = false;
  s = OH_JSVM_IsArray(env_, propNames, &isArray);
  FOOTSTONE_DCHECK(s == JSVM_OK);
  if (!isArray) {
    return false;
  }

  uint32_t arrayLength = 0;
  s = OH_JSVM_GetArrayLength(env_, propNames, &arrayLength);
  FOOTSTONE_DCHECK(s == JSVM_OK);
  if (!arrayLength) {
    return true; // TODO(hot-js):
  }

  for (uint32_t i = 0; i < arrayLength; i++)
  {
    bool hasElement = false;
    s = OH_JSVM_HasElement(env_, propNames, i, &hasElement);
    FOOTSTONE_DCHECK(s == JSVM_OK);
    if (!hasElement) {
      continue;
    }

    JSVM_Value propName = nullptr;
    s = OH_JSVM_GetElement(env_, propNames, i, &propName);
    FOOTSTONE_DCHECK(s == JSVM_OK);

    bool hasProp = false;
    s = OH_JSVM_HasProperty(env_, obj, propName, &hasProp);
    FOOTSTONE_DCHECK(s == JSVM_OK);
    if (!hasProp) {
      continue;
    }

    JSVM_Value propValue = nullptr;
    s = OH_JSVM_GetProperty(env_, obj, propName, &propValue);
    FOOTSTONE_DCHECK(s == JSVM_OK);
    
    map[std::make_shared<JSHCtxValue>(env_, propName)] = std::make_shared<JSHCtxValue>(env_, propValue);
  }
  
  return true;
}

bool JSHCtx::GetEntriesFromMap(const std::shared_ptr<CtxValue>& value,
                              std::unordered_map<std::shared_ptr<CtxValue>, std::shared_ptr<CtxValue>>& map) {
//   v8::HandleScope handle_scope(isolate_);
//   auto context = context_persistent_.Get(isolate_);

  JSHHandleScope handleScope(env_);
  auto ctx_value = std::static_pointer_cast<JSHCtxValue>(value);
  auto js_value = ctx_value->GetValue();
//   auto handle_value = v8::Local<v8::Value>::New(isolate_, ctx_value->global_value_);
//   auto handle_object = v8::Local<v8::Map>::Cast(handle_value);

  std::shared_ptr<CtxValue> map_key;
  std::shared_ptr<CtxValue> map_value;
  
  // TODO(hot-js):
  bool isJsArray = false;
  auto s = OH_JSVM_IsArray(env_, js_value, &isJsArray);
  FOOTSTONE_DCHECK(s == JSVM_OK);
  if (isJsArray) {
    uint32_t arrayLength = 0;
    s = OH_JSVM_GetArrayLength(env_, js_value, &arrayLength);
    FOOTSTONE_DCHECK(s == JSVM_OK);

    for (uint32_t i = 0; i < arrayLength; i++) {
      bool hasElement = false;
      s = OH_JSVM_HasElement(env_, js_value, i, &hasElement);
      FOOTSTONE_DCHECK(s == JSVM_OK);

      JSVM_Value element = nullptr;
      s = OH_JSVM_GetElement(env_, js_value, i, &element);
      FOOTSTONE_DCHECK(s == JSVM_OK);
      
      if (i % 2 == 0) {
        map_key = std::make_shared<JSHCtxValue>(env_, element);
      } else {
        map[map_key] = std::make_shared<JSHCtxValue>(env_, element);
      }
    }
  }
  
  bool isJsObj = false;
  s = OH_JSVM_IsObject(env_, ctx_value->GetValue(), &isJsObj);
  FOOTSTONE_DCHECK(s == JSVM_OK);
  if (isJsObj) {
    JSVM_Value propNames = nullptr;
    s = OH_JSVM_GetPropertyNames(env_, js_value, &propNames);
    FOOTSTONE_DCHECK(s == JSVM_OK);
  
    bool isArray = false;
    s = OH_JSVM_IsArray(env_, propNames, &isArray);
    FOOTSTONE_DCHECK(s == JSVM_OK);
    if (!isArray) {
      return false;
    }
  
    uint32_t arrayLength = 0;
    s = OH_JSVM_GetArrayLength(env_, propNames, &arrayLength);
    FOOTSTONE_DCHECK(s == JSVM_OK);
    if (!arrayLength) {
      return false;
    }
  
    for (uint32_t i = 0; i < arrayLength; i++)
    {
      bool hasElement = false;
      s = OH_JSVM_HasElement(env_, propNames, i, &hasElement);
      FOOTSTONE_DCHECK(s == JSVM_OK);
      if (!hasElement) {
        continue;
      }
  
      JSVM_Value propName = nullptr;
      s = OH_JSVM_GetElement(env_, propNames, i, &propName);
      FOOTSTONE_DCHECK(s == JSVM_OK);
  
      bool hasProp = false;
      s = OH_JSVM_HasProperty(env_, js_value, propName, &hasProp);
      FOOTSTONE_DCHECK(s == JSVM_OK);
      if (!hasProp) {
        continue;
      }
  
      JSVM_Value propValue = nullptr;
      s = OH_JSVM_GetProperty(env_, js_value, propName, &propValue);
      FOOTSTONE_DCHECK(s == JSVM_OK);
      
      map[std::make_shared<JSHCtxValue>(env_, propName)] = std::make_shared<JSHCtxValue>(env_, propValue);
    }
  }
  
  
//   auto handle_array = handle_object->AsArray();
//   std::shared_ptr<CtxValue> map_key;
//   std::shared_ptr<CtxValue> map_value;
//   for (uint32_t i = 0; i < handle_array->Length(); ++i) {
//     if (i % 2 == 0) {
//       map_key = std::make_shared<V8CtxValue>(isolate_, handle_array->Get(context, i).ToLocalChecked());
//     } else {
//       map[map_key] = std::make_shared<V8CtxValue>(isolate_, handle_array->Get(context, i).ToLocalChecked());
//     }
//   }
  return true;
}

bool JSHCtx::IsMap(const std::shared_ptr<CtxValue>& value) {
  if (!value) {
    return false;
  }

//   v8::HandleScope handle_scope(isolate_);
//   auto context = context_persistent_.Get(isolate_);
//   v8::Context::Scope context_scope(context);
  JSHHandleScope handleScope(env_);
  auto ctx_value = std::static_pointer_cast<JSHCtxValue>(value);
//   const v8::Global<v8::Value>& persistent_value = ctx_value->global_value_;
//   auto handle_value = v8::Local<v8::Value>::New(isolate_, persistent_value);

//   if (handle_value.IsEmpty()) {
//     return false;
//   }
//   return handle_value->IsMap();
  
  // TODO(hot-js):
  bool result = false;
  auto s = OH_JSVM_IsObject(env_, ctx_value->GetValue(), &result);
  FOOTSTONE_DCHECK(s == JSVM_OK);
  return result;
}

bool JSHCtx::IsNull(const std::shared_ptr<CtxValue>& value) {
  if (!value) {
    return false;
  }

//   v8::HandleScope handle_scope(isolate_);
//   auto context = context_persistent_.Get(isolate_);
//   v8::Context::Scope context_scope(context);
  JSHHandleScope handleScope(env_);
  auto ctx_value = std::static_pointer_cast<JSHCtxValue>(value);
//   const v8::Global<v8::Value>& persistent_value = ctx_value->global_value_;
//   auto handle_value = v8::Local<v8::Value>::New(isolate_, persistent_value);
//   if (handle_value.IsEmpty()) {
//     return false;
//   }
//   return handle_value->IsNull();
  
  bool result = false;
  auto s = OH_JSVM_IsNull(env_, ctx_value->GetValue(), &result);
  FOOTSTONE_DCHECK(s == JSVM_OK);
  return result;
}

bool JSHCtx::IsUndefined(const std::shared_ptr<CtxValue>& value) {
  if (!value) {
    return false;
  }

//   v8::HandleScope handle_scope(isolate_);
//   auto context = context_persistent_.Get(isolate_);
//   v8::Context::Scope context_scope(context);
  JSHHandleScope handleScope(env_);
  auto ctx_value = std::static_pointer_cast<JSHCtxValue>(value);
//   const v8::Global<v8::Value>& persistent_value = ctx_value->global_value_;
//   auto handle_value = v8::Local<v8::Value>::New(isolate_, persistent_value);
//   if (handle_value.IsEmpty()) {
//     return false;
//   }
//   return handle_value->IsUndefined();
  
  bool result = false;
  auto s = OH_JSVM_IsUndefined(env_, ctx_value->GetValue(), &result);
  FOOTSTONE_DCHECK(s == JSVM_OK);
  return result;
}

bool JSHCtx::IsNullOrUndefined(const std::shared_ptr<CtxValue>& value) {
  if (!value) {
    return true;
  }

//   v8::HandleScope handle_scope(isolate_);
//   auto context = context_persistent_.Get(isolate_);
//   v8::Context::Scope context_scope(context);
  JSHHandleScope handleScope(env_);
  auto ctx_value = std::static_pointer_cast<JSHCtxValue>(value);
//   const v8::Global<v8::Value>& persistent_value = ctx_value->global_value_;
//   auto handle_value = v8::Local<v8::Value>::New(isolate_, persistent_value);
//   if (handle_value.IsEmpty()) {
//     return false;
//   }
//   return handle_value->IsNullOrUndefined();
  bool result = false;
  auto s = OH_JSVM_IsNullOrUndefined(env_, ctx_value->GetValue(), &result);
  FOOTSTONE_DCHECK(s == JSVM_OK);
  return result;
}

// Array Helpers

bool JSHCtx::IsArray(const std::shared_ptr<CtxValue>& value) {
  if (!value) {
    return false;
  }
//   v8::HandleScope handle_scope(isolate_);
//   v8::Local<v8::Context> context = context_persistent_.Get(isolate_);
//   v8::Context::Scope context_scope(context);
  JSHHandleScope handleScope(env_);
  auto ctx_value = std::static_pointer_cast<JSHCtxValue>(value);
//   const v8::Global<v8::Value>& global_value = ctx_value->global_value_;
//   v8::Local<v8::Value> handle_value =
//       v8::Local<v8::Value>::New(isolate_, global_value);
//
//   if (handle_value.IsEmpty()) {
//     return false;
//   }
//   return handle_value->IsArray();
  bool result = false;
  auto s = OH_JSVM_IsArray(env_, ctx_value->GetValue(), &result);
  FOOTSTONE_DCHECK(s == JSVM_OK);
  return result;
}

uint32_t JSHCtx::GetArrayLength(const std::shared_ptr<CtxValue>& value) {
  if (value == nullptr) {
    return 0;
  }
//   v8::HandleScope handle_scope(isolate_);
//   v8::Local<v8::Context> context = context_persistent_.Get(isolate_);
//   v8::Context::Scope context_scope(context);
  JSHHandleScope handleScope(env_);
  auto ctx_value = std::static_pointer_cast<JSHCtxValue>(value);
//   const v8::Global<v8::Value>& global_value = ctx_value->global_value_;
//   v8::Local<v8::Value> handle_value =
//       v8::Local<v8::Value>::New(isolate_, global_value);
//
//   if (handle_value.IsEmpty()) {
//     return 0;
//   }
//
//   if (handle_value->IsArray()) {
//     v8::Array* array = v8::Array::Cast(*handle_value);
//     return array->Length();
//   }
  
  bool isArray = false;
  auto s = OH_JSVM_IsArray(env_, ctx_value->GetValue(), &isArray);
  FOOTSTONE_DCHECK(s == JSVM_OK);
  if (isArray) {
    uint32_t arrayLength = 0;
    s = OH_JSVM_GetArrayLength(env_, ctx_value->GetValue(), &arrayLength);
    FOOTSTONE_DCHECK(s == JSVM_OK);
    return arrayLength;
  }

  return 0;
}

std::shared_ptr<CtxValue> JSHCtx::CopyArrayElement(
    const std::shared_ptr<CtxValue>& value,
    uint32_t index) {
  if (!value) {
    return nullptr;
  }
//   v8::HandleScope handle_scope(isolate_);
//   v8::Local<v8::Context> context = context_persistent_.Get(isolate_);
//   v8::Context::Scope context_scope(context);
  JSHHandleScope handleScope(env_);
  auto ctx_value = std::static_pointer_cast<JSHCtxValue>(value);
//   const v8::Global<v8::Value>& global_value = ctx_value->global_value_;
//   v8::Local<v8::Value> handle_value = v8::Local<v8::Value>::New(isolate_, global_value);

//   if (handle_value.IsEmpty()) {
//     return nullptr;
//   }

//   if (handle_value->IsArray()) {
//     v8::Array* array = v8::Array::Cast(*handle_value);
//     v8::Local<v8::Value> ret = array->Get(context, index).ToLocalChecked();
//     if (ret.IsEmpty()) {
//       return nullptr;
//     }
//
//     return std::make_shared<V8CtxValue>(isolate_, ret);
//   }
  
  bool isArray = false;
  auto s = OH_JSVM_IsArray(env_, ctx_value->GetValue(), &isArray);
  FOOTSTONE_DCHECK(s == JSVM_OK);
  if (isArray) {
    uint32_t arrayLength = 0;
    s = OH_JSVM_GetArrayLength(env_, ctx_value->GetValue(), &arrayLength);
    FOOTSTONE_DCHECK(s == JSVM_OK);
    if (index >= arrayLength) {
      return nullptr;
    }
    JSVM_Value result = nullptr;
    s = OH_JSVM_GetElement(env_, ctx_value->GetValue(), index, &result);
    FOOTSTONE_DCHECK(s == JSVM_OK);
    if (!result) {
      return nullptr;
    }
    return std::make_shared<JSHCtxValue>(env_, result);
  }
  
  return nullptr;
}

// Map Helpers
size_t JSHCtx::GetMapLength(std::shared_ptr<CtxValue>& value) {
  if (value == nullptr) {
    return 0;
  }
//   v8::HandleScope handle_scope(isolate_);
//   v8::Local<v8::Context> context = context_persistent_.Get(isolate_);
//   v8::Context::Scope context_scope(context);
  JSHHandleScope handleScope(env_);
  auto ctx_value = std::static_pointer_cast<JSHCtxValue>(value);
//   const v8::Global<v8::Value>& persistent_value = ctx_value->global_value_;
//   v8::Local<v8::Value> handle_value =
//       v8::Local<v8::Value>::New(isolate_, persistent_value);
//
//   if (handle_value.IsEmpty()) {
//     return 0;
//   }
  
//   if (handle_value->IsMap()) {
//     v8::Map* map = v8::Map::Cast(*handle_value);
//     return map->Size();
//   }
  
  auto js_value = ctx_value->GetValue();
  
  // TODO(hot-js):
  bool isJsObj = false;
  auto s = OH_JSVM_IsObject(env_, ctx_value->GetValue(), &isJsObj);
  FOOTSTONE_DCHECK(s == JSVM_OK);
  if (isJsObj) {
    JSVM_Value propNames = nullptr;
    s = OH_JSVM_GetPropertyNames(env_, js_value, &propNames);
    FOOTSTONE_DCHECK(s == JSVM_OK);
  
    bool isArray = false;
    s = OH_JSVM_IsArray(env_, propNames, &isArray);
    FOOTSTONE_DCHECK(s == JSVM_OK);
    if (!isArray) {
      return 0;
    }
  
    uint32_t arrayLength = 0;
    s = OH_JSVM_GetArrayLength(env_, propNames, &arrayLength);
    FOOTSTONE_DCHECK(s == JSVM_OK);
    return arrayLength;
  }

  return 0;
}

std::shared_ptr<CtxValue> JSHCtx::ConvertMapToArray(
    const std::shared_ptr<CtxValue>& value) {
  if (value == nullptr) {
    return nullptr;
  }
//   v8::HandleScope handle_scope(isolate_);
//   v8::Local<v8::Context> context = context_persistent_.Get(isolate_);
//   v8::Context::Scope context_scope(context);
//   std::shared_ptr<V8CtxValue> ctx_value =
//       std::static_pointer_cast<V8CtxValue>(value);
//   const v8::Global<v8::Value>& persistent_value = ctx_value->global_value_;
//   v8::Local<v8::Value> handle_value =
//       v8::Local<v8::Value>::New(isolate_, persistent_value);
//
//   if (handle_value.IsEmpty()) {
//     return nullptr;
//   }
//
//   if (handle_value->IsMap()) {
//     v8::Map* map = v8::Map::Cast(*handle_value);
//     v8::Local<v8::Array> array = map->AsArray();
//     return std::make_shared<V8CtxValue>(isolate_, array);
//   }

  JSHHandleScope handleScope(env_);
  return nullptr;
}

// Object Helpers

bool JSHCtx::HasNamedProperty(const std::shared_ptr<CtxValue>& value,
                             const string_view& name) {
  if (!value || StringViewUtils::IsEmpty(name)) {
    return false;
  }
//   v8::HandleScope handle_scope(isolate_);
//   auto context = context_persistent_.Get(isolate_);
//   v8::Context::Scope context_scope(context);
  JSHHandleScope handleScope(env_);
  auto ctx_value = std::static_pointer_cast<JSHCtxValue>(value);
//   const v8::Global<v8::Value>& global_value = ctx_value->global_value_;
//   auto handle_value = v8::Local<v8::Value>::New(isolate_, global_value);
//
//   if (handle_value.IsEmpty()) {
//     return false;
//   }

//   if (handle_value->IsMap()) {
//     v8::Map* map = v8::Map::Cast(*handle_value);
//     auto key = V8VM::CreateV8String(isolate_, context, name);
//     if (key.IsEmpty()) {
//       return false;
//     }
//
//     v8::Maybe<bool> ret = map->Has(context, key);
//     return ret.ToChecked();
//   }
  
  // TODO(hot-js):
  bool isJsObj = false;
  auto s = OH_JSVM_IsObject(env_, ctx_value->GetValue(), &isJsObj);
  FOOTSTONE_DCHECK(s == JSVM_OK);
  if (isJsObj) {
    auto key_value = JSHVM::CreateV8String(env_, name);
    auto jsh_key_value = std::static_pointer_cast<JSHCtxValue>(key_value);
    
    bool result = false;
    s = OH_JSVM_HasProperty(env_, ctx_value->GetValue(), jsh_key_value->GetValue(), &result);
    FOOTSTONE_DCHECK(s == JSVM_OK);
    return result;
  }
  
  return false;
}

std::shared_ptr<CtxValue> JSHCtx::CopyNamedProperty(
    const std::shared_ptr<CtxValue>& value,
    const string_view& name) {
  if (!value || StringViewUtils::IsEmpty(name)) {
    return nullptr;
  }
//   v8::HandleScope handle_scope(isolate_);
//   auto context = context_persistent_.Get(isolate_);
//   v8::Context::Scope context_scope(context);
  JSHHandleScope handleScope(env_);
  auto ctx_value = std::static_pointer_cast<JSHCtxValue>(value);
//   const auto& global_value = ctx_value->global_value_;
//   auto handle_value = v8::Local<v8::Value>::New(isolate_, global_value);
//
//   if (handle_value.IsEmpty()) {
//     return nullptr;
//   }

//   if (handle_value->IsMap()) {
//     auto map = v8::Map::Cast(*handle_value);
//     if (!map) {
//       return nullptr;
//     }
//     auto key = V8VM::CreateV8String(isolate_, context, name);
//     if (key.IsEmpty()) {
//       return nullptr;
//     }
//
//     return std::make_shared<V8CtxValue>(isolate_, map->Get(context, key).ToLocalChecked());
//   }
  
  // TODO(hot-js):
  bool isJsObj = false;
  auto s = OH_JSVM_IsObject(env_, ctx_value->GetValue(), &isJsObj);
  FOOTSTONE_DCHECK(s == JSVM_OK);
  if (isJsObj) {
    auto key_value = JSHVM::CreateV8String(env_, name);
    auto jsh_key_value = std::static_pointer_cast<JSHCtxValue>(key_value);
    
    JSVM_Value result = nullptr;
    s = OH_JSVM_GetProperty(env_, ctx_value->GetValue(), jsh_key_value->GetValue(), &result);
    FOOTSTONE_DCHECK(s == JSVM_OK);
    return std::make_shared<JSHCtxValue>(env_, result);
  }

  return nullptr;
}

std::shared_ptr<CtxValue> JSHCtx::GetPropertyNames(const std::shared_ptr<CtxValue>& value) {
  if (!value) {
    return nullptr;
  }
//   v8::HandleScope handle_scope(isolate_);
//   auto context = context_persistent_.Get(isolate_);
//   v8::Context::Scope context_scope(context);
  JSHHandleScope handleScope(env_);
  auto ctx_value = std::static_pointer_cast<JSHCtxValue>(value);
//   const v8::Global<v8::Value>& global_value = ctx_value->global_value_;
//   auto handle_value = v8::Local<v8::Value>::New(isolate_, global_value);
//   auto object = handle_value->ToObject(context);
//   if (object.IsEmpty()) {
//     return nullptr;
//   }
//   auto props = object.ToLocalChecked()->GetPropertyNames(context);
//   if (props.IsEmpty()) {
//     return nullptr;
//   }
  
  auto js_value = ctx_value->GetValue();
  
  bool isJsObj = false;
  auto s = OH_JSVM_IsObject(env_, js_value, &isJsObj);
  FOOTSTONE_DCHECK(s == JSVM_OK);
  if (!isJsObj) {
    return nullptr;  
  }
  
  JSVM_Value propNames = nullptr;
  s = OH_JSVM_GetPropertyNames(env_, js_value, &propNames);
  FOOTSTONE_DCHECK(s == JSVM_OK);
  
  return std::make_shared<JSHCtxValue>(env_, propNames);
}


std::shared_ptr<CtxValue> JSHCtx::GetOwnPropertyNames(const std::shared_ptr<CtxValue>& value) {
  if (!value) {
    return nullptr;
  }
//   v8::HandleScope handle_scope(isolate_);
//   auto context = context_persistent_.Get(isolate_);
//   v8::Context::Scope context_scope(context);
  JSHHandleScope handleScope(env_);
  auto ctx_value = std::static_pointer_cast<JSHCtxValue>(value);
//   const v8::Global<v8::Value>& global_value = ctx_value->global_value_;
//   auto handle_value = v8::Local<v8::Value>::New(isolate_, global_value);
//   auto object = handle_value->ToObject(context);
//   if (object.IsEmpty()) {
//     return nullptr;
//   }
//   auto props = object.ToLocalChecked()->GetOwnPropertyNames(context);
//   if (props.IsEmpty()) {
//     return nullptr;
//   }
//   return std::make_shared<V8CtxValue>(isolate_, props.ToLocalChecked());
  
  auto js_value = ctx_value->GetValue();
  
  bool isJsObj = false;
  auto s = OH_JSVM_IsObject(env_, js_value, &isJsObj);
  FOOTSTONE_DCHECK(s == JSVM_OK);
  if (!isJsObj) {
    return nullptr;  
  }
  
  // TODO(hot-js):
  JSVM_Value propNames = nullptr;
  s = OH_JSVM_GetAllPropertyNames(env_, js_value, JSVM_KEY_OWN_ONLY,
                                static_cast<JSVM_KeyFilter>(JSVM_KEY_ENUMERABLE | JSVM_KEY_SKIP_SYMBOLS),
                                JSVM_KEY_NUMBERS_TO_STRINGS, &propNames);
  FOOTSTONE_DCHECK(s == JSVM_OK);
  
  return std::make_shared<JSHCtxValue>(env_, propNames);
}

bool JSHCtx::IsBoolean(const std::shared_ptr<CtxValue>& value) {
  if (!value) {
    return false;
  }
  auto ctx_value = std::static_pointer_cast<JSHCtxValue>(value);
//   v8::HandleScope handle_scope(isolate_);
//   auto context = context_persistent_.Get(isolate_);
//   v8::Context::Scope context_scope(context);
//
//   const auto& global_value = ctx_value->global_value_;
//   auto handle_value = v8::Local<v8::Value>::New(isolate_, global_value);
//   return handle_value->IsBoolean();
  
  JSHHandleScope handleScope(env_);
  
  bool result = false;
  auto s = OH_JSVM_IsBoolean(env_, ctx_value->GetValue(), &result);
  FOOTSTONE_DCHECK(s == JSVM_OK);
  return result;
}

bool JSHCtx::IsNumber(const std::shared_ptr<CtxValue>& value) {
  if (!value) {
    return false;
  }
  auto ctx_value = std::static_pointer_cast<JSHCtxValue>(value);
//   v8::HandleScope handle_scope(isolate_);
//   auto context = context_persistent_.Get(isolate_);
//   v8::Context::Scope context_scope(context);
//
//   const auto& global_value = ctx_value->global_value_;
//   auto handle_value = v8::Local<v8::Value>::New(isolate_, global_value);
//   return handle_value->IsNumber();
  JSHHandleScope handleScope(env_);
  bool result = false;
  auto s = OH_JSVM_IsNumber(env_, ctx_value->GetValue(), &result);
  FOOTSTONE_DCHECK(s == JSVM_OK);
  return result;
}

bool JSHCtx::IsString(const std::shared_ptr<CtxValue>& value) {
  if (!value) {
    return false;
  }
  auto ctx_value = std::static_pointer_cast<JSHCtxValue>(value);
//   v8::HandleScope handle_scope(isolate_);
//   v8::Local<v8::Context> context = context_persistent_.Get(isolate_);
//   v8::Context::Scope context_scope(context);
//
//   const auto& global_value = ctx_value->global_value_;
//   auto handle_value = v8::Local<v8::Value>::New(isolate_, global_value);
//   return handle_value->IsString();
  
  JSHHandleScope handleScope(env_);
  bool result = false;
  auto s = OH_JSVM_IsString(env_, ctx_value->GetValue(), &result);
  FOOTSTONE_DCHECK(s == JSVM_OK);
  return result;
}

bool JSHCtx::IsFunction(const std::shared_ptr<CtxValue>& value) {
  if (!value) {
    return false;
  }
//   v8::HandleScope handle_scope(isolate_);
//   v8::Local<v8::Context> context = context_persistent_.Get(isolate_);
//   v8::Context::Scope context_scope(context);
  JSHHandleScope handleScope(env_);
  std::shared_ptr<JSHCtxValue> ctx_value = std::static_pointer_cast<JSHCtxValue>(value);
//   const v8::Global<v8::Value>& global_value = ctx_value->global_value_;
//   v8::Local<v8::Value> handle_value = v8::Local<v8::Value>::New(isolate_, global_value);
//
//   if (handle_value.IsEmpty()) {
//     return false;
//   }
//
//   return handle_value->IsFunction();
  
  
  JSVM_ValueType valuetype = (JSVM_ValueType)100;
  auto s = OH_JSVM_Typeof(env_, ctx_value->GetValue(), &valuetype);
  FOOTSTONE_DCHECK(s == JSVM_OK);
//   FOOTSTONE_DLOG(INFO) << "xxx hippy value type, " << valuetype;
  
  bool result = false;
  s = OH_JSVM_IsFunction(env_, ctx_value->GetValue(), &result);
  FOOTSTONE_DCHECK(s == JSVM_OK);
  return result;
}

bool JSHCtx::IsObject(const std::shared_ptr<CtxValue>& value) {
  if (!value) {
    return false;
  }
//   v8::HandleScope handle_scope(isolate_);
//   auto context = context_persistent_.Get(isolate_);
//   v8::Context::Scope context_scope(context);
  JSHHandleScope handleScope(env_);
  std::shared_ptr<JSHCtxValue> ctx_value = std::static_pointer_cast<JSHCtxValue>(value);
//   const v8::Global<v8::Value>& global_value = ctx_value->global_value_;
//   v8::Local<v8::Value> handle_value = v8::Local<v8::Value>::New(isolate_, global_value);
//
//   if (handle_value.IsEmpty()) {
//     return false;
//   }
//
//   return handle_value->IsObject();
  bool result = false;
  auto s = OH_JSVM_IsObject(env_, ctx_value->GetValue(), &result);
  FOOTSTONE_DCHECK(s == JSVM_OK);
  
//     JSVM_ValueType valuetype = (JSVM_ValueType)100;
//     s = OH_JSVM_Typeof(env_, ctx_value->GetValue(), &valuetype);
//     FOOTSTONE_DCHECK(s == JSVM_OK);
//     FOOTSTONE_DLOG(INFO) << "xxx hippy IsObject, real type, " << valuetype;
  
  return result;
}

string_view JSHCtx::CopyFunctionName(const std::shared_ptr<CtxValue>& function) {
  if (!function) {
    return {};
  }
//   v8::HandleScope handle_scope(isolate_);
//   auto context = context_persistent_.Get(isolate_);
//   v8::Context::Scope context_scope(context);
  JSHHandleScope handleScope(env_);
  auto ctx_value = std::static_pointer_cast<JSHCtxValue>(function);
//   const v8::Global<v8::Value>& global_value = ctx_value->global_value_;
//   auto handle_value = v8::Local<v8::Value>::New(isolate_, global_value);

  string_view result;
//   if (handle_value->IsFunction()) {
//     auto v8_str = handle_value->ToString(context).ToLocalChecked();
//     result = V8VM::ToStringView(isolate_, context, v8_str);
//   }
  
  bool isFunc = false;
  auto s = OH_JSVM_IsFunction(env_, ctx_value->GetValue(), &isFunc);
  FOOTSTONE_DCHECK(s == JSVM_OK);
  if (isFunc) {
    // TODO(hot-js):
  }

  return result;
}

bool JSHCtx::SetProperty(std::shared_ptr<CtxValue> object,
                        std::shared_ptr<CtxValue> key,
                        std::shared_ptr<CtxValue> value) {
//   v8::HandleScope handle_scope(isolate_);
//   auto context = context_persistent_.Get(isolate_);
//   v8::Context::Scope context_scope(context);
  JSHHandleScope handleScope(env_);
  auto jsh_object = std::static_pointer_cast<JSHCtxValue>(object);
//   auto handle_v8_object = v8::Local<v8::Value>::New(isolate_, v8_object->global_value_);
  auto jsh_key = std::static_pointer_cast<JSHCtxValue>(key);
//   auto handle_v8_key = v8::Local<v8::Value>::New(isolate_, v8_key->global_value_);
  auto jsh_value = std::static_pointer_cast<JSHCtxValue>(value);
//   auto handle_v8_value = v8::Local<v8::Value>::New(isolate_, v8_value->global_value_);

//   auto handle_object = v8::Local<v8::Object>::Cast(handle_v8_object);
//   return handle_object->Set(context, handle_v8_key, handle_v8_value).FromMaybe(false);
  auto s = OH_JSVM_SetProperty(env_, jsh_object->GetValue(), jsh_key->GetValue(), jsh_value->GetValue());
  FOOTSTONE_DCHECK(s == JSVM_OK);
  return true;
}

bool JSHCtx::SetProperty(std::shared_ptr<CtxValue> object,
                        std::shared_ptr<CtxValue> key,
                        std::shared_ptr<CtxValue> value,
                        const PropertyAttribute& attr) {
  if (!IsString(key)) {
    return false;
  }
//   v8::HandleScope handle_scope(isolate_);
//   auto context = context_persistent_.Get(isolate_);
//   v8::Context::Scope context_scope(context);
  JSHHandleScope handleScope(env_);
  auto jsh_object = std::static_pointer_cast<JSHCtxValue>(object);
//   auto handle_v8_object = v8::Local<v8::Value>::New(isolate_, v8_object->global_value_);
  auto jsh_key = std::static_pointer_cast<JSHCtxValue>(key);
//   auto handle_v8_key = v8::Local<v8::Value>::New(isolate_, v8_key->global_value_);
  auto jsh_value = std::static_pointer_cast<JSHCtxValue>(value);
//   auto handle_v8_value = v8::Local<v8::Value>::New(isolate_, v8_value->global_value_);

//   auto handle_object = v8::Local<v8::Object>::Cast(handle_v8_object);
//   auto v8_attr = v8::PropertyAttribute(attr);
//   return handle_object->DefineOwnProperty(context,
//                                           handle_v8_key->ToString(context).ToLocalChecked(),
//                                           handle_v8_value, v8_attr).FromMaybe(false);
  
  // TODO(hot-js):
  auto s = OH_JSVM_SetProperty(env_, jsh_object->GetValue(), jsh_key->GetValue(), jsh_value->GetValue());
  FOOTSTONE_DCHECK(s == JSVM_OK);
  return true;
}

std::shared_ptr<CtxValue> JSHCtx::CreateObject() {
//   v8::HandleScope handle_scope(isolate_);
//   auto context = context_persistent_.Get(isolate_);
//   v8::Context::Scope context_scope(context);

//   auto object = v8::Object::New(isolate_);
//   return std::make_shared<V8CtxValue>(isolate_, object);
  JSHHandleScope handleScope(env_);
  JSVM_Value obj = nullptr;
  auto s = OH_JSVM_CreateObject(env_, &obj);
  FOOTSTONE_DCHECK(s == JSVM_OK);
  return std::make_shared<JSHCtxValue>(env_, obj);
}

std::shared_ptr<CtxValue> JSHCtx::NewInstance(const std::shared_ptr<CtxValue>& cls,
                                             int argc, std::shared_ptr<CtxValue> argv[],
                                             void* external) {
//   v8::HandleScope handle_scope(isolate_);
//   auto context = context_persistent_.Get(isolate_);
//   v8::Context::Scope context_scope(context);

  JSHHandleScope handleScope(env_);
  auto jsh_cls = std::static_pointer_cast<JSHCtxValue>(cls);
//   auto cls_handle_value = v8::Local<v8::Value>::New(isolate_, v8_cls->global_value_);
//   auto function = v8::Local<v8::Function>::Cast(cls_handle_value);
//   v8::Local<v8::Object> instance;
//   if (argc > 0 && argv) {
//     JSVM_Value jsh_argv[argc];
//     for (auto i = 0; i < argc; ++i) {
//       auto jsh_value = std::static_pointer_cast<JSHCtxValue>(argv[i]);
//       jsh_argv[i] = jsh_value->GetValue();
//     }
//     instance = function->NewInstance(context, argc, v8_argv).ToLocalChecked();
//   } else {
//     instance = function->NewInstance(context).ToLocalChecked();
//   }
//   if (external) {
//     auto external_value = v8::External::New(isolate_, external);
//     instance->SetInternalField(kExternalIndex, external_value);
//   }
//   return std::make_shared<V8CtxValue>(isolate_, instance);
  
//   JSVM_Value testClass = nullptr;
//   JSVM_CallbackStruct *param1 = new JSVM_CallbackStruct();
//   param1->data = jsh_cls->GetValue();
//   param1->callback = [](JSVM_Env env, JSVM_CallbackInfo info) -> JSVM_Value {
//     JSHHandleScope handleScope(env);
//    
//     size_t argc = 10; // TODO(hot-js):
//     JSVM_Value args[10];
//     JSVM_Value thisArg = nullptr;
//     void *data = nullptr;
//     auto s = OH_JSVM_GetCbInfo(env, info, &argc, args, &thisArg, &data);
//     FOOTSTONE_DCHECK(s == JSVM_OK);
//     FOOTSTONE_DLOG(INFO) << "xxx hippy define, data2: " << data << ", thisArg: " << thisArg << ", argc: " << argc;
//     FOOTSTONE_DCHECK(data);
//
//     JSVM_Value result = 0;
//     s = OH_JSVM_CallFunction(env, thisArg, (JSVM_Value)data, argc, args, &result);
//     FOOTSTONE_DCHECK(s == JSVM_OK);
//     return result;
//   };
  
//   FOOTSTONE_DLOG(INFO) << "xxx hippy define, data1: " << param1->data;
//   auto s = OH_JSVM_DefineClass(env_, "TestClass", JSVM_AUTO_LENGTH, param1, 0, nullptr, &testClass);
//   FOOTSTONE_DCHECK(s == JSVM_OK);

  JSVM_Value instanceValue = nullptr;
  
  JSVM_Value *jsh_argv = new JSVM_Value[(size_t)argc];
  for (auto i = 0; i < argc; ++i) {
    auto jsh_value = std::static_pointer_cast<JSHCtxValue>(argv[i]);
    jsh_argv[i] = jsh_value->GetValue();
  }
  
  FOOTSTONE_DLOG(INFO) << "xxx hippy, NewInstance, use cls: " << jsh_cls->GetValue() << ", argc: " << argc;
  FOOTSTONE_DLOG(INFO) << "xxx hippy, NewInstance, use cls: " << jsh_cls->GetValue() << ", argc: " << argc;
  FOOTSTONE_DLOG(INFO) << "xxx hippy, NewInstance, use cls: " << jsh_cls->GetValue() << ", argc: " << argc;
  JSVM_Status s = OH_JSVM_NewInstance(env_, jsh_cls->GetValue(), (size_t)argc, jsh_argv, &instanceValue);
  FOOTSTONE_DCHECK(s == JSVM_OK);
  
  if (external) {
    sEmbedderExternalMap[instanceValue] = external;
    
  }
  FOOTSTONE_DLOG(INFO) << "xxx hippy, NewInstance, after, cls: " << jsh_cls->GetValue();
  FOOTSTONE_DLOG(INFO) << "xxx hippy, NewInstance, after, instanceValue: " << instanceValue;
  
  return std::make_shared<JSHCtxValue>(env_, instanceValue);
}

void* JSHCtx::GetObjectExternalData(const std::shared_ptr<CtxValue>& object) {
//   v8::HandleScope handle_scope(isolate_);
//   auto context = context_persistent_.Get(isolate_);
//   v8::Context::Scope context_scope(context);
//
//   auto v8_object = std::static_pointer_cast<V8CtxValue>(object);
//   auto handle_value = v8::Local<v8::Value>::New(isolate_, v8_object->global_value_);
//   auto handle_object = v8::Local<v8::Object>::Cast(handle_value);
//   return handle_object->GetInternalField(kExternalIndex).As<v8::External>()->Value();
  JSHHandleScope handleScope(env_);
  return nullptr;
}

std::shared_ptr<CtxValue> JSHCtx::DefineProxy(const std::unique_ptr<FunctionWrapper>& constructor_wrapper) {
//   v8::HandleScope handle_scope(isolate_);
//   auto context = context_persistent_.Get(isolate_);
//   v8::Context::Scope context_scope(context);

//   auto func_tpl = v8::FunctionTemplate::New(isolate_);
//   auto obj_tpl = func_tpl->InstanceTemplate();
//   obj_tpl->SetHandler(v8::NamedPropertyHandlerConfiguration(InvokePropertyCallback,
//                                                             nullptr,
//                                                             nullptr,
//                                                             nullptr,
//                                                             nullptr,
//                                                             v8::External::New(isolate_,
//                                                                               reinterpret_cast<void*>(constructor_wrapper.get()))));
//   obj_tpl->SetInternalFieldCount(1);
//   return std::make_shared<V8CtxValue>(isolate_, func_tpl->GetFunction(context).ToLocalChecked());
  JSHHandleScope handleScope(env_);
  return nullptr;
}

std::shared_ptr<CtxValue> JSHCtx::DefineClass(const string_view& name,
                                             const std::shared_ptr<ClassDefinition>& parent,
                                             const std::unique_ptr<FunctionWrapper>& constructor_wrapper,
                                             size_t property_count,
                                             std::shared_ptr<PropertyDescriptor> properties[]) {
//   v8::HandleScope handle_scope(isolate_);
//   auto context = context_persistent_.Get(isolate_);
//   v8::Context::Scope context_scope(context);

  JSHHandleScope handleScope(env_);
//   auto tpl = CreateTemplate(constructor_wrapper);
  if (parent) {
    auto parent_template = std::static_pointer_cast<JSHClassDefinition>(parent);
//     auto parent_template_handle = v8::Local<v8::FunctionTemplate>::New(
//         isolate_,parent_template->GetTemplate());
//     tpl->Inherit(parent_template_handle);
  }
  /*
  auto prototype_template = tpl->PrototypeTemplate();
  tpl->InstanceTemplate()->SetInternalFieldCount(1);
  tpl->SetClassName(V8VM::CreateV8String(isolate_, context, name));
  for (size_t i = 0; i < property_count; i++) {
    const auto& prop_desc = properties[i];
    auto v8_attr = v8::PropertyAttribute(prop_desc->attr);
    auto prop_name = std::static_pointer_cast<V8CtxValue>(prop_desc->name);
    auto property_name = v8::Local<v8::Value>::New(isolate_, prop_name->global_value_);
    auto v8_prop_name = v8::Local<v8::Name>::Cast(property_name);
    if (prop_desc->getter || prop_desc->setter) {
      v8::Local<v8::FunctionTemplate> getter_tpl;
      v8::Local<v8::FunctionTemplate> setter_tpl;
      if (prop_desc->getter) {
        getter_tpl = CreateTemplate(prop_desc->getter);
      }
      if (prop_desc->setter) {
        setter_tpl = CreateTemplate(prop_desc->setter);
      }
      prototype_template->SetAccessorProperty(
          v8_prop_name,
          getter_tpl,
          setter_tpl,
          v8_attr,
          v8::AccessControl::DEFAULT);
    } else if (prop_desc->method) {
      auto method = CreateTemplate(prop_desc->method);
      prototype_template->Set(v8_prop_name, method, v8_attr);
    } else {
      auto v8_ctx = std::static_pointer_cast<V8CtxValue>(prop_desc->value);
      auto handle_value = v8::Local<v8::Value>::New(isolate_, v8_ctx->global_value_);
      prototype_template->Set(v8_prop_name, handle_value, v8_attr);
    }
  }
  // todo(polly) static_property

  template_map_[name] = std::make_shared<V8ClassDefinition>(isolate_, tpl);

  return std::make_shared<V8CtxValue>(isolate_, tpl->GetFunction(context).ToLocalChecked());
  //*/
  
  JSVM_PropertyDescriptor *propParams = new JSVM_PropertyDescriptor[property_count]; // TODO(hot-js):
  JSVM_CallbackStruct *callbackParams = new JSVM_CallbackStruct[property_count];
  for (size_t i = 0; i < property_count; i++) {
    const auto &prop_desc = properties[i];
    auto &propParam = propParams[i];
    auto &callbackParam = callbackParams[i];
    auto prop_name = std::static_pointer_cast<JSHCtxValue>(prop_desc->name);
    propParam.utf8name = nullptr;
    propParam.name = prop_name->GetValue();
    propParam.method = nullptr;
    propParam.value = nullptr;
    propParam.attributes = JSVM_DEFAULT;
    if (prop_desc->getter || prop_desc->setter) {
      if (prop_desc->getter) {
        JSVM_CallbackStruct *callbackP = new JSVM_CallbackStruct();
        callbackP->data = prop_desc->getter.get(); // TODO(hot-js): get and set
        callbackP->callback = InvokeJsCallback;
        propParam.getter = callbackP;
      } else {
        propParam.getter = nullptr;
      }
      if (prop_desc->setter) {
        JSVM_CallbackStruct *callbackP = new JSVM_CallbackStruct();
        callbackP->data = prop_desc->setter.get();
        callbackP->callback = InvokeJsCallback;
        propParam.setter = callbackP;
      } else {
        propParam.setter = nullptr;
      }
    } else if (prop_desc->method) {
      callbackParam.data = prop_desc->method.get();
      callbackParam.callback = InvokeJsCallback;
      propParam.utf8name = nullptr;
      propParam.name = prop_name->GetValue();
      propParam.method = &callbackParam;
      propParam.getter = nullptr;
      propParam.setter = nullptr;
      propParam.value = nullptr;
      propParam.attributes = JSVM_DEFAULT;
    } else {
      auto prop_value = std::static_pointer_cast<JSHCtxValue>(prop_desc->value);
      propParam.utf8name = nullptr;
      propParam.name = prop_name->GetValue();
      propParam.method = nullptr;
      propParam.getter = nullptr;
      propParam.setter = nullptr;
      propParam.value = prop_value->GetValue();
      propParam.attributes = JSVM_DEFAULT;
    }
  }
  
  string_view utf8Name = StringViewUtils::ConvertEncoding(name, string_view::Encoding::Utf8);
  
  JSVM_CallbackStruct *constructorParam = new JSVM_CallbackStruct();
  constructorParam->data = constructor_wrapper.get();
  constructorParam->callback = InvokeJsCallback_Construct;
  
//   JSVM_PropertyDescriptor descriptor[] = {
//             {"consoleinfo", NULL, &constructorParam, NULL, NULL, NULL, JSVM_DEFAULT},
//             {"add", NULL, &constructorParam, NULL, NULL, NULL, JSVM_DEFAULT},
//         };
  
  JSVM_Value testClass = nullptr;
  auto s = OH_JSVM_DefineClass(env_, (const char *)utf8Name.utf8_value().c_str(), JSVM_AUTO_LENGTH, constructorParam, property_count, propParams, &testClass);
//   auto s = OH_JSVM_DefineClass(env_, (const char *)utf8Name.utf8_value().c_str(), JSVM_AUTO_LENGTH, &constructorParam, 2, descriptor, &testClass);
  //   auto s = OH_JSVM_DefineClass(env_, (const char *)utf8Name.utf8_value().c_str(), JSVM_AUTO_LENGTH, &constructorParam, 0, 0, &testClass);
  FOOTSTONE_DCHECK(s == JSVM_OK);
  FOOTSTONE_DCHECK(testClass);
  FOOTSTONE_DLOG(INFO) << "xxx hippy, DefineClass, cls res: " << testClass << ", name: " << name;
  
  template_map_[name] = std::make_shared<JSHClassDefinition>(env_, testClass);

  return std::make_shared<JSHCtxValue>(env_, testClass);
}

bool JSHCtx::Equals(const std::shared_ptr<CtxValue>& lhs, const std::shared_ptr<CtxValue>& rhs) {
  FOOTSTONE_DCHECK(lhs != nullptr && rhs != nullptr);
//   v8::HandleScope handle_scope(isolate_);
//   v8::Local<v8::Context> context = context_persistent_.Get(isolate_);
//   v8::Context::Scope context_scope(context);
  JSHHandleScope handleScope(env_);
  std::shared_ptr<JSHCtxValue> ctx_lhs = std::static_pointer_cast<JSHCtxValue>(lhs);
  std::shared_ptr<JSHCtxValue> ctx_rhs = std::static_pointer_cast<JSHCtxValue>(rhs);

//   const v8::Global<v8::Value>& global_lhs = ctx_lhs->global_value_;
//   FOOTSTONE_DCHECK(!global_lhs.IsEmpty());
//   v8::Local<v8::Value> local_lhs = v8::Local<v8::Value>::New(isolate_, global_lhs);
//
//   const v8::Global<v8::Value>& global_rhs = ctx_rhs->global_value_;
//   FOOTSTONE_DCHECK(!global_rhs.IsEmpty());
//   v8::Local<v8::Value> local_rhs = v8::Local<v8::Value>::New(isolate_, global_rhs);
//
//   FOOTSTONE_DCHECK(!local_lhs.IsEmpty());
//
//   v8::Maybe<bool> maybe = local_lhs->Equals(context, local_rhs);
//   if(maybe.IsNothing()) {
//     return false;
//   }
//   return maybe.FromJust();
  
  bool isEquals = false;
  auto s = OH_JSVM_Equals(env_, ctx_lhs->GetValue(), ctx_rhs->GetValue(), &isEquals);
  FOOTSTONE_DCHECK(s == JSVM_OK);
  return isEquals;
}

bool JSHCtx::IsByteBuffer(const std::shared_ptr<CtxValue>& value) {
//   v8::HandleScope handle_scope(isolate_);
//   v8::Local<v8::Context> context = context_persistent_.Get(isolate_);
//   v8::Context::Scope context_scope(context);
  JSHHandleScope handleScope(env_);
  std::shared_ptr<JSHCtxValue> ctx_value = std::static_pointer_cast<JSHCtxValue>(value);
//   const v8::Global<v8::Value>& persistent_value = ctx_value->global_value_;
//   v8::Local<v8::Value> handle_value =
//       v8::Local<v8::Value>::New(isolate_, persistent_value);
//   if (handle_value.IsEmpty()) {
//     return false;
//   }
//   return handle_value->IsArrayBuffer();
  
  bool result = false;
  auto s = OH_JSVM_IsArraybuffer(env_, ctx_value->GetValue(), &result);
  FOOTSTONE_DCHECK(s == JSVM_OK);
  return result;
}

bool JSHCtx::GetByteBuffer(const std::shared_ptr<CtxValue>& value,
                          void** out_data,
                          size_t& out_length,
                          uint32_t& out_type) {
//   v8::HandleScope handle_scope(isolate_);
//   v8::Local<v8::Context> context = context_persistent_.Get(isolate_);
//   v8::Context::Scope context_scope(context);
  JSHHandleScope handleScope(env_);
  auto ctx_value = std::static_pointer_cast<JSHCtxValue>(value);
//   const v8::Global<v8::Value>& persistent_value = ctx_value->global_value_;
//   auto handle_value = v8::Local<v8::Value>::New(isolate_, persistent_value);
//   if (handle_value.IsEmpty()) {
//     return false;
//   }
//   if (!handle_value->IsArrayBuffer()) {
//     return false;
//   }
//   v8::Local<v8::ArrayBuffer> array_buffer = v8::Local<v8::ArrayBuffer>::Cast(handle_value);
// #if V8_MAJOR_VERSION < 9
//   *out_data = array_buffer->GetContents().Data();
//   out_length = array_buffer->ByteLength();
// #else
//   *out_data = array_buffer->GetBackingStore()->Data();
//   out_length = array_buffer->ByteLength();
// #endif //V8_MAJOR_VERSION < 9
  
  bool isArrayBuffer = false;
  auto s = OH_JSVM_IsArraybuffer(env_, ctx_value->GetValue(), &isArrayBuffer);
  FOOTSTONE_DCHECK(s == JSVM_OK);
  if (!isArrayBuffer) {
    return false;
  }

  void *tmpArrayBufferPtr = nullptr;
  size_t arrayBufferLength = 0;
  s = OH_JSVM_GetArraybufferInfo(env_, ctx_value->GetValue(), &tmpArrayBufferPtr, &arrayBufferLength);
  FOOTSTONE_DCHECK(s == JSVM_OK);
  
  *out_data = tmpArrayBufferPtr;
  out_length = arrayBufferLength;
  
  return true;
}

void  JSHCtx::SetWeak(std::shared_ptr<CtxValue> value,
                     const std::unique_ptr<WeakCallbackWrapper>& wrapper) {
//   v8::HandleScope handle_scope(isolate_);
//   auto context = context_persistent_.Get(isolate_);
//   v8::Context::Scope context_scope(context);
  JSHHandleScope handleScope(env_);
  auto ctx_value = std::static_pointer_cast<JSHCtxValue>(value);
//   auto handle_value = v8::Local<v8::Value>::New(isolate_, ctx_value->global_value_);
//   auto handle_object = v8::Local<v8::Object>::Cast(handle_value);
//   v8::Global<v8::Value> weak(isolate_, handle_object);
//   weak.SetWeak(reinterpret_cast<void*>(wrapper.get()), [](const v8::WeakCallbackInfo<void>& info) {
//     info.SetSecondPassCallback([](const v8::WeakCallbackInfo<void>& info) {
//       auto wrapper = reinterpret_cast<WeakCallbackWrapper*>(info.GetParameter());
//       auto internal = info.GetInternalField(kExternalIndex);
//       wrapper->callback(wrapper->data, internal);
//     });
//   }, v8::WeakCallbackType::kParameter);
  
  // TODO(hot-js):
}

}  // namespace napi
}  // namespace driver
}  // namespace hippy
