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

#include <ace/xcomponent/native_interface_xcomponent.h>
#include "renderer/native_render_provider_capi.h"
#include "renderer/native_render_manager.h"
#include "oh_napi/ark_ts.h"
#include "oh_napi/oh_napi_object.h"
#include "oh_napi/oh_napi_task_runner.h"
#include "oh_napi/oh_napi_invocation.h"
#include "oh_napi/oh_napi_register.h"
#include "oh_napi/oh_measure_text.h"
#include "footstone/hippy_value.h"
#include "dom/render_manager.h"
#include "dom/root_node.h"
#include "dom/scene.h"

using DomArgument = hippy::dom::DomArgument;
using DomEvent = hippy::dom::DomEvent;
using DomManager = hippy::dom::DomManager;
using HippyValue = footstone::value::HippyValue;
using NativeRenderManager = hippy::render::native::NativeRenderManager;
using RenderManager = hippy::dom::RenderManager;
using RootNode = hippy::dom::RootNode;
using Scene = hippy::dom::Scene;

namespace hippy {
inline namespace framework {
inline namespace renderer {
inline namespace native {

void NativeRenderProvider_UpdateRootSize(uint32_t render_manager_id, uint32_t root_id, float width, float height) {
  auto &map = NativeRenderManager::PersistentMap();
  std::shared_ptr<NativeRenderManager> render_manager;
  bool ret = map.Find(render_manager_id, render_manager);
  if (!ret) {
    FOOTSTONE_DLOG(WARNING) << "UpdateRootSize render_manager_id invalid";
    return;
  }

  std::shared_ptr<DomManager> dom_manager = render_manager->GetDomManager();
  if (dom_manager == nullptr) {
    FOOTSTONE_DLOG(WARNING) << "UpdateRootSize dom_manager is nullptr";
    return;
  }

  auto &root_map = RootNode::PersistentMap();
  std::shared_ptr<RootNode> root_node;
  ret = root_map.Find(root_id, root_node);
  if (!ret) {
    FOOTSTONE_DLOG(WARNING) << "UpdateRootSize root_node is nullptr";
    return;
  }

  std::vector<std::function<void()>> ops;
  ops.emplace_back([dom_manager, root_node, width, height] {
    FOOTSTONE_LOG(INFO) << "update root size width = " << width << ", height = " << height << std::endl;
    dom_manager->SetRootSize(root_node, width, height);
    dom_manager->DoLayout(root_node);
    dom_manager->EndBatch(root_node);
  });
  dom_manager->PostTask(Scene(std::move(ops)));
}

void NativeRenderProvider_UpdateNodeSize(uint32_t render_manager_id, uint32_t root_id, uint32_t node_id, float width, float height) {
  auto &map = NativeRenderManager::PersistentMap();
  std::shared_ptr<NativeRenderManager> render_manager;
  bool ret = map.Find(render_manager_id, render_manager);
  if (!ret) {
    FOOTSTONE_DLOG(WARNING) << "UpdateNodeSize render_manager_id invalid";
    return;
  }

  std::shared_ptr<DomManager> dom_manager = render_manager->GetDomManager();
  if (dom_manager == nullptr) {
    FOOTSTONE_DLOG(WARNING) << "UpdateNodeSize dom_manager is nullptr";
    return;
  }

  auto &root_map = RootNode::PersistentMap();
  std::shared_ptr<RootNode> root_node;
  ret = root_map.Find(root_id, root_node);
  if (!ret) {
    FOOTSTONE_DLOG(WARNING) << "UpdateNodeSize root_node is nullptr";
    return;
  }

  auto node = dom_manager->GetNode(root_node, node_id);
  if (node == nullptr) {
    FOOTSTONE_DLOG(WARNING) << "UpdateNodeSize DomNode not found for id: " << node_id;
    return;
  }

  std::unordered_map<std::string, std::shared_ptr<HippyValue>> update_style;
  std::shared_ptr<HippyValue> width_value = std::make_shared<HippyValue>(width);
  std::shared_ptr<HippyValue> height_value = std::make_shared<HippyValue>(height);
  update_style.insert({"width", width_value});
  update_style.insert({"height", height_value});

  std::vector<std::function<void()>> ops = {[dom_manager, root_node, node, update_style] {
    node->UpdateDomNodeStyleAndParseLayoutInfo(update_style);
    dom_manager->EndBatch(root_node);
  }};
  dom_manager->PostTask(Scene(std::move(ops)));
}

void NativeRenderProvider_OnReceivedEvent(uint32_t render_manager_id, uint32_t root_id, uint32_t node_id,
            const std::string &event_name, const std::shared_ptr<HippyValue> &params, bool capture, bool bubble) {
  auto &map = NativeRenderManager::PersistentMap();
  std::shared_ptr<NativeRenderManager> render_manager;
  bool ret = map.Find(render_manager_id, render_manager);
  if (!ret) {
    FOOTSTONE_DLOG(WARNING) << "OnReceivedEvent render_manager_id invalid";
    return;
  }

  auto &root_map = RootNode::PersistentMap();
  std::shared_ptr<RootNode> root_node;
  ret = root_map.Find(root_id, root_node);
  if (!ret) {
    FOOTSTONE_DLOG(WARNING) << "OnReceivedEvent root_node is nullptr";
    return;
  }
  
  render_manager->ReceivedEvent(root_node, node_id, event_name, params, capture, bubble);
}

void NativeRenderProvider_DoCallBack(uint32_t render_manager_id, int32_t result, const std::string &func_name,
            uint32_t root_id, uint32_t node_id, uint32_t cb_id, const HippyValue &params) {
  auto &map = NativeRenderManager::PersistentMap();
  std::shared_ptr<NativeRenderManager> render_manager;
  bool ret = map.Find(render_manager_id, render_manager);
  if (!ret) {
    FOOTSTONE_DLOG(WARNING) << "DoCallBack render_manager_id invalid";
    return;
  }

  std::shared_ptr<DomManager> dom_manager = render_manager->GetDomManager();
  if (dom_manager == nullptr) {
    FOOTSTONE_DLOG(WARNING) << "DoCallBack dom_manager is nullptr";
    return;
  }
  
  std::vector<std::function<void()>> ops = {[root_id, node_id, cb_id, func_name, dom_manager, params] {
    auto &root_map = RootNode::PersistentMap();
    std::shared_ptr<RootNode> root_node;
    bool ret = root_map.Find(root_id, root_node);
    if (!ret) {
      FOOTSTONE_DLOG(WARNING) << "DoCallBack root_node is nullptr";
      return;
    }

    auto node = dom_manager->GetNode(root_node, node_id);
    if (node == nullptr) {
      FOOTSTONE_DLOG(WARNING) << "DoCallBack DomNode not found for id: " << node_id;
      return;
    }

    auto callback = node->GetCallback(func_name, cb_id);
    if (callback == nullptr) {
      FOOTSTONE_DLOG(WARNING) << "DoCallBack Callback not found for func_name: " << func_name;
      return;
    }

    callback(std::make_shared<DomArgument>(params));
  }};
  dom_manager->PostTask(Scene(std::move(ops)));
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
  std::getline(ss, instanceId, '_');
  std::string rootId;
  std::getline(ss, rootId, '_');
  std::string nodeId;
  std::getline(ss, nodeId, '_');
  uint32_t render_manager_id = static_cast<uint32_t>(std::stoul(instanceId, nullptr));
  uint32_t root_id = static_cast<uint32_t>(std::stoul(rootId, nullptr));
  uint32_t node_id = nodeId.size() > 0 ? static_cast<uint32_t>(std::stoul(nodeId, nullptr)) : 0;

  auto &map = NativeRenderManager::PersistentMap();
  std::shared_ptr<NativeRenderManager> render_manager;
  bool ret = map.Find(render_manager_id, render_manager);
  if (!ret) {
    FOOTSTONE_DLOG(WARNING) << "RegisterNativeXComponent: render_manager_id invalid";
    return;
  }

  render_manager->RegisterNativeXComponentHandle(nativeXComponent, root_id, node_id);
}

napi_value OhNapi_OnLoad(napi_env env, napi_value exports) {
  hippy::RegisterNativeXComponent(env, exports);
  return exports;
}

REGISTER_OH_NAPI_ONLOAD(hippy::OhNapi_OnLoad)

static napi_value DestroyRoot(napi_env env, napi_callback_info info) {
  ArkTS arkTs(env);
  auto args = arkTs.GetCallbackArgs(info);
  uint32_t render_manager_id = static_cast<uint32_t>(arkTs.GetInteger(args[0]));
  uint32_t root_id = static_cast<uint32_t>(arkTs.GetInteger(args[1]));
  
  auto &map = NativeRenderManager::PersistentMap();
  std::shared_ptr<NativeRenderManager> render_manager;
  bool ret = map.Find(render_manager_id, render_manager);
  if (!ret) {
    FOOTSTONE_DLOG(WARNING) << "DestroyRoot: render_manager_id invalid";
    return arkTs.GetUndefined();
  }
  
  render_manager->DestroyRoot(root_id);

  return arkTs.GetUndefined();
}

REGISTER_OH_NAPI("NativeRenderProvider", "NativeRenderProvider_DestroyRoot", DestroyRoot)

}
}
}
}
