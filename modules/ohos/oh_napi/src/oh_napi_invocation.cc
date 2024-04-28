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

#include "oh_napi/oh_napi_invocation.h"
#include "oh_napi/ark_ts.h"
#include <ace/xcomponent/native_interface_xcomponent.h>
#include <napi/native_api.h>
#include "footstone/logging.h"

namespace hippy {
inline namespace framework {
inline namespace ohnapi {

std::shared_ptr<OhNapiInvocation> OhNapiInvocation::GetInstance() {
  static std::shared_ptr<OhNapiInvocation> instance = nullptr;
  static std::once_flag flag;
  std::call_once(flag, [] {
    instance = std::make_shared<OhNapiInvocation>();
  });
  return instance;
}

napi_value OhNapiInvocation::OhNapi_OnLoad(napi_env env, napi_value exports) {
  for (const auto& func: oh_napi_onloads_) {
    func(env, exports);
  }
  return exports;
}

static void RegisterNativeXComponent(napi_env env, napi_value exports) {
  if ((env == nullptr) || (exports == nullptr)) {
    FOOTSTONE_LOG(ERROR) << "RegisterNativeXComponent: env or exports is null";
    return;
  }

  napi_value exportInstance = nullptr;
  if (napi_get_named_property(env, exports, OH_NATIVE_XCOMPONENT_OBJ, &exportInstance) != napi_ok) {
    FOOTSTONE_LOG(ERROR) << "RegisterNativeXComponent: napi_get_named_property fail";
    return;
  }

  OH_NativeXComponent *nativeXComponent = nullptr;
  if (napi_unwrap(env, exportInstance, reinterpret_cast<void **>(&nativeXComponent)) != napi_ok) {
    FOOTSTONE_LOG(ERROR) << "RegisterNativeXComponent: napi_get_named_property fail";
    return;
  }

  char idStr[OH_XCOMPONENT_ID_LEN_MAX + 1] = {'\0'};
  uint64_t idSize = OH_XCOMPONENT_ID_LEN_MAX + 1;
  if (OH_NativeXComponent_GetXComponentId(nativeXComponent, idStr, &idSize) != OH_NATIVEXCOMPONENT_RESULT_SUCCESS) {
    FOOTSTONE_LOG(ERROR) << "RegisterNativeXComponent: OH_NativeXComponent_GetXComponentId fail";
    return;
  }
  std::string xcomponentStr(idStr);
  std::stringstream ss(xcomponentStr);
  std::string instanceId;
//   std::getline(ss, instanceId, '_');
//   std::string rootId;
//   std::getline(ss, rootId, '_');
//   size_t instanceIdNum = std::stoul(instanceId, nullptr);

  // TODO:
  
  FOOTSTONE_LOG(INFO) << "RegisterNativeXComponent: id = " << instanceId;
}

}  // namespace ohnapi
}  // namespace framework
}  // namespace hippy

EXTERN_C_START
static napi_value HippyModuleRegisterFunc(napi_env env, napi_value exports) {
  hippy::OhNapiInvocation::GetInstance()->OhNapi_OnLoad(env, exports);
  hippy::RegisterNativeXComponent(env, exports);
  return exports;
}
EXTERN_C_END

static napi_module hippyModule = {
  .nm_version = 1,
  .nm_flags = 0,
  .nm_filename = nullptr,
  .nm_register_func = HippyModuleRegisterFunc,
  .nm_modname = "hippy_module",
  .nm_priv = ((void *)0),
  .reserved = {0},
};

extern "C" __attribute__((constructor))
void RegisterHippyModule(void) {
  napi_module_register(&hippyModule);
}
