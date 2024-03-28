/*
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
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "connector/native_renderer_napi.h"
#include <js_native_api.h>
#include <js_native_api_types.h>
#include "oh_napi/data_holder.h"
#include "oh_napi/oh_napi_register.h"
#include "oh_napi/ark_ts.h"
#include "renderer/native_render_manager.h"
#include "dom/render_manager.h"
#include "dom/root_node.h"
#include "dom/scene.h"

using DomArgument = hippy::dom::DomArgument;
using DomEvent = hippy::dom::DomEvent;
using DomManager = hippy::dom::DomManager;
using HippyValue = footstone::value::HippyValue;
using NativeRenderManager = hippy::NativeRenderManager;
using RenderManager = hippy::dom::RenderManager;
using RootNode = hippy::dom::RootNode;
using Scene = hippy::dom::Scene;

namespace hippy {
inline namespace framework {
inline namespace connector {
inline namespace renderer {
inline namespace native {

static napi_value CreateNativeRenderManager(napi_env env, napi_callback_info info) {
  ArkTS arkTs(env);
  auto args = arkTs.GetCallbackArgs(info);
  auto ts_render_provider_ref = arkTs.CreateReference(args[0]);
  auto density = arkTs.GetDouble(args[1]);
  auto render_manager = std::make_shared<NativeRenderManager>();

  render_manager->SetRenderDelegate(env, ts_render_provider_ref);
  render_manager->InitDensity(density);
  auto render_id = hippy::global_data_holder_key.fetch_add(1);
  auto flag = hippy::global_data_holder.Insert(render_id,
                                               std::static_pointer_cast<RenderManager>(render_manager));
  FOOTSTONE_CHECK(flag);
  return arkTs.CreateInt(static_cast<int>(render_id));
}

static napi_value DestroyNativeRenderManager(napi_env env, napi_callback_info info) {
  ArkTS arkTs(env);
  auto args = arkTs.GetCallbackArgs(info, 1);
  uint32_t render_manager_id = static_cast<uint32_t>(arkTs.GetInteger(args[0]));
  auto flag = hippy::global_data_holder.Erase(render_manager_id);
  FOOTSTONE_DCHECK(flag);
  return arkTs.GetUndefined();
}

static napi_value SetDomManager(napi_env env, napi_callback_info info) {
  ArkTS arkTs(env);
  auto args = arkTs.GetCallbackArgs(info, 2);
  uint32_t render_id = static_cast<uint32_t>(arkTs.GetInteger(args[0]));
  uint32_t dom_manager_id = static_cast<uint32_t>(arkTs.GetInteger(args[1]));

  std::any render_manager;
  auto flag = hippy::global_data_holder.Find(render_id, render_manager);
  FOOTSTONE_CHECK(flag);
  auto render_manager_object = std::any_cast<std::shared_ptr<RenderManager>>(render_manager);
  auto native_render_manager = std::static_pointer_cast<NativeRenderManager>(render_manager_object);

  std::any dom_manager;
  flag = hippy::global_data_holder.Find(dom_manager_id, dom_manager);
  FOOTSTONE_CHECK(flag);
  auto dom_manager_object = std::any_cast<std::shared_ptr<DomManager>>(dom_manager);
  native_render_manager->SetDomManager(dom_manager_object);
  return arkTs.GetUndefined();
}

REGISTER_OH_NAPI("NativeRenderer", "NativeRenderer_CreateNativeRenderManager", CreateNativeRenderManager)
REGISTER_OH_NAPI("NativeRenderer", "NativeRenderer_DestroyNativeRenderManager", DestroyNativeRenderManager)
REGISTER_OH_NAPI("NativeRenderer", "NativeRenderer_SetDomManager", SetDomManager)

}
}
}
}
}
