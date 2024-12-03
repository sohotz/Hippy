/*
 *
 * Tencent is pleased to support the open source community by making
 * Hippy available.
 *
 * Copyright (C) 2019 THL A29 Limited, a Tencent company.
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

#include "connector/arkts_turbo_module.h"

#include "driver/napi/js_ctx.h"
#include "driver/napi/js_ctx_value.h"
#include "driver/napi/callback_info.h"
#include "driver/scope.h"
#include "footstone/logging.h"
#include "footstone/string_view.h"
#include "footstone/string_view_utils.h"

// using string_view = footstone::string_view;
// using StringViewUtils = footstone::StringViewUtils;
// using Ctx = hippy::Ctx;
// using CtxValue = hippy::CtxValue;
// using CallbackInfo = hippy::CallbackInfo;
// using JNIEnvironment = hippy::JNIEnvironment;
// using ScopeWrapper = hippy::ScopeWrapper;
// using JavaRef = hippy::JavaRef;
// using JniUtils = hippy::JniUtils;

namespace hippy {
inline namespace framework {
inline namespace turbo {

// static jclass argument_utils_clazz;
// static jmethodID get_methods_signature;

std::shared_ptr<CtxValue> ArkTsTurboModule::InvokeArkTsMethod(const std::shared_ptr<CtxValue>& prop_name,
                                                            CallbackInfo& info,
                                                            void* data) {
  auto wrapper = reinterpret_cast<TurboWrapper*>(data);
  FOOTSTONE_CHECK(wrapper && wrapper->module && wrapper->name);
      auto scope_wrapper = reinterpret_cast<ScopeWrapper*>(std::any_cast<void*>(info.GetSlot()));
  auto scope = scope_wrapper->scope.lock();
      auto context = scope->GetContext();
        return context->CreateUndefined();
//   auto result = wrapper->module->InvokeArkTsMethod(wrapper->name, info, data);

//     auto result = turboManager.Call(prop_name, args);
//   });
//   return module;
}

ArkTsTurboModule::ArkTsTurboModule(const std::string& name,
                                 std::shared_ptr<Turbo>& impl,
                                 const std::shared_ptr<Ctx>& ctx)
  : name(name), impl(impl) {
  auto getter = std::make_unique<FunctionWrapper>([](CallbackInfo& info, void* data) {
    auto scope_wrapper = reinterpret_cast<ScopeWrapper*>(std::any_cast<void*>(info.GetSlot()));
    auto scope = scope_wrapper->scope.lock();
    FOOTSTONE_CHECK(scope);
    auto ctx = scope->GetContext();
    auto module = reinterpret_cast<ArkTsTurboModule*>(data);
    auto name = info[0];
    if (!name) {
      return;
    }
    auto func_object = module->func_map[name];
    if (func_object) {
      info.GetReturnValue()->Set(func_object);
      return;
    }
    auto turbo_wrapper = std::make_unique<TurboWrapper>(module, name);
    auto func_wrapper = std::make_unique<FunctionWrapper>([](CallbackInfo& info, void* data) {
      auto scope_wrapper = reinterpret_cast<ScopeWrapper*>(std::any_cast<void*>(info.GetSlot()));
      auto scope = scope_wrapper->scope.lock();
      FOOTSTONE_CHECK(scope);
      auto ctx = scope->GetContext();
      auto wrapper = reinterpret_cast<TurboWrapper*>(data);
      FOOTSTONE_CHECK(wrapper && wrapper->module && wrapper->name);
      auto result = wrapper->module->InvokeArkTsMethod(wrapper->name, info, data);
      info.GetReturnValue()->Set(result);
    }, turbo_wrapper.get());
    func_object = ctx->CreateFunction(func_wrapper);
    turbo_wrapper->SetFunctionWrapper(std::move(func_wrapper));
    module->turbo_wrapper_map[name] = std::move(turbo_wrapper);
    module->func_map[name] = func_object;
    info.GetReturnValue()->Set(func_object);
  }, this);
  constructor = ctx->DefineProxy(getter);
  constructor_wrapper = std::move(getter);
}

}
}
}
