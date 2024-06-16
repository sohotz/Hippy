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

#include <arkui/native_node_napi.h>
#include "renderer/uimanager/hr_view_manager.h"
#include "oh_napi/ark_ts.h"
#include "oh_napi/oh_napi_object_builder.h"
#include "renderer/components/custom_ts_view.h"
#include "renderer/components/custom_view.h"
#include "renderer/components/hippy_render_view_creator.h"
#include "renderer/native_render_context.h"
#include "footstone/logging.h"

namespace hippy {
inline namespace render {
inline namespace native {

HRViewManager::HRViewManager(uint32_t instance_id, uint32_t root_id, std::shared_ptr<NativeRender> &native_render)
  : nativeXComponent_(nullptr) {
  root_id_ = root_id;
  ctx_ = std::make_shared<NativeRenderContext>(instance_id, root_id, native_render);
  root_view_ = std::make_shared<RootView>(ctx_);
  root_view_->SetTag(root_id);
  std::string root_view_type = "RootView";
  root_view_->SetViewType(root_view_type);
  view_registry_[root_id] = root_view_;
}

void HRViewManager::AttachToNativeXComponent(OH_NativeXComponent *nativeXComponent) {
  if (nativeXComponent == nativeXComponent_) {
    return;
  }
  MaybeDetachRootNode(nativeXComponent_, root_view_);
  nativeXComponent_ = nativeXComponent;
  MaybeAttachRootNode(nativeXComponent, root_view_);
}

void HRViewManager::MaybeAttachRootNode(OH_NativeXComponent *nativeXComponent, std::shared_ptr<RootView> &rootView) {
  if (nativeXComponent != nullptr) {
    FOOTSTONE_DLOG(INFO) << "Attaching root view to nativeXComponent with id: " << rootView->GetTag();
    OH_NativeXComponent_AttachNativeRootNode(nativeXComponent, rootView->GetLocalRootArkUINode().GetArkUINodeHandle());
  }
}

void HRViewManager::MaybeDetachRootNode(OH_NativeXComponent *nativeXComponent, std::shared_ptr<RootView> &rootView) {
}

void HRViewManager::RegisterCustomTsRenderViews(const std::set<std::string> &views, napi_ref builderCallbackRef, napi_env env) {
  custom_ts_render_views_ = views;
  ts_custom_builder_callback_ref_ = builderCallbackRef;
  ts_env_ = env;
}

void HRViewManager::AddMutations(std::shared_ptr<HRMutation> &m) {
  mutations_.push_back(m);
}

void HRViewManager::ApplyMutations() {
  for (auto it = mutations_.begin(); it != mutations_.end(); it++) {
    ApplyMutation(*it);
  }
  mutations_.clear();
}

void HRViewManager::ApplyMutation(std::shared_ptr<HRMutation> &m) {
  if (m->type_ == HRMutationType::CREATE) {
    auto tm = std::static_pointer_cast<HRCreateMutation>(m);
    auto view = CreateRenderView(tm->tag_, tm->view_name_, tm->is_parent_text_);
    if (view) {
      InsertSubRenderView(tm->parent_tag_, view, tm->index_);
      UpdateProps(view, tm->props_);
    }
  } else if (m->type_ == HRMutationType::UPDATE) {
    auto tm = std::static_pointer_cast<HRUpdateMutation>(m);
    UpdateProps(tm->tag_, tm->props_, tm->delete_props_);
  } else if (m->type_ == HRMutationType::MOVE) {
    auto tm = std::static_pointer_cast<HRMoveMutation>(m);
    MoveRenderView(tm->node_infos_, tm->parent_tag_);
  } else if (m->type_ == HRMutationType::MOVE2) {
    auto tm = std::static_pointer_cast<HRMove2Mutation>(m);
    Move2RenderView(tm->tags_, tm->to_parent_tag_, tm->from_parent_tag_, tm->index_);
  } else if (m->type_ == HRMutationType::DELETE) {
    auto tm = std::static_pointer_cast<HRDeleteMutation>(m);
    RemoveRenderView(tm->tag_);
  } else if (m->type_ == HRMutationType::UPDATE_LAYOUT) {
    auto tm = std::static_pointer_cast<HRUpdateLayoutMutation>(m);
    SetRenderViewFrame(
      tm->tag_,
      HRRect(tm->left_, tm->top_, tm->width_, tm->height_),
      HRPadding(tm->padding_left_, tm->padding_top_, tm->padding_right_, tm->padding_bottom_)
    );
  } else if (m->type_ == HRMutationType::UPDATE_EVENT_LISTENER) {
    auto tm = std::static_pointer_cast<HRUpdateEventListenerMutation>(m);
    UpdateProps(tm->tag_, tm->props_);
    UpdateEventListener(tm->tag_, tm->props_);
  }
}

std::shared_ptr<BaseView> HRViewManager::CreateRenderView(uint32_t tag, std::string &view_name, bool is_parent_text) {
  std::shared_ptr<BaseView> view = nullptr;
  
  // custom ts view
  view = CreateCustomTsRenderView(tag, view_name, is_parent_text);
  if (view) {
    return view;
  }
  
  // custom cpp view
  view = CreateCustomRenderView(tag, view_name, is_parent_text);
  if (view) {
    return view;
  }
  
  // build-in view
  view = HippyCreateRenderView(view_name, is_parent_text, ctx_);
  if (view) {
    view->SetTag(tag);
    view->SetViewType(view_name);
    view_registry_[tag] = view;
    return view;
  } else {
    FOOTSTONE_DLOG(INFO) << "CreateRenderView failed, " << view_name;
  }
  return nullptr;
}

std::shared_ptr<BaseView> HRViewManager::CreateCustomTsRenderView(uint32_t tag, std::string &view_name, bool is_parent_text) {
  if (custom_ts_render_views_.find(view_name) != custom_ts_render_views_.end()) {
    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(ts_env_, &scope);
    if (scope == nullptr) {
      return nullptr;
    }
    
    ArkTS arkTs(ts_env_);
    auto params_builder = arkTs.CreateObjectBuilder();
    params_builder.AddProperty("", "");
    
    napi_value callback = arkTs.GetReferenceValue(ts_custom_builder_callback_ref_);
    std::vector<napi_value> args = {
      params_builder.Build()
    };
    napi_value global;
    napi_get_global(ts_env_, &global);
    napi_value tsNode = arkTs.Call(callback, args, global);
    ArkUI_NodeHandle nodeHandle = nullptr;
    OH_ArkUI_GetNodeHandleFromNapiValue(ts_env_, tsNode, &nodeHandle);
    
    napi_close_handle_scope(ts_env_, scope);
    
    auto view = std::make_shared<CustomTsView>(ctx_, nodeHandle);
    view->SetTag(tag);
    view->SetViewType(view_name);
    view_registry_[tag] = view;
    return view;
  }
  return nullptr;
}

std::shared_ptr<BaseView> HRViewManager::CreateCustomRenderView(uint32_t tag, std::string &view_name, bool is_parent_text) {
  return nullptr;
}

void HRViewManager::RemoveRenderView(uint32_t tag) {
  auto it = view_registry_.find(tag);
  std::shared_ptr<BaseView> renderView = it != view_registry_.end() ? it->second : nullptr;
  if (renderView) {
    renderView->RemoveFromParentView();
    RemoveFromRegistry(renderView);
  }
}

void HRViewManager::RemoveFromRegistry(std::shared_ptr<BaseView> &renderView) {
  auto &children = renderView->GetChildren();
  for (uint32_t i = 0; i < children.size(); i++) {
    RemoveFromRegistry(children[i]);
  }
  view_registry_.erase(renderView->GetTag());
}

void HRViewManager::InsertSubRenderView(uint32_t parentTag, std::shared_ptr<BaseView> &childView, int32_t index) {
  auto it = view_registry_.find(parentTag);
  std::shared_ptr<BaseView> parentView = it != view_registry_.end() ? it->second : nullptr;
  if (parentView && childView) {
    parentView->AddSubRenderView(childView, index);
  } else {
    FOOTSTONE_DLOG(INFO) << "InsertSubRenderView parentTag:" << parentTag << ", child:" << childView->GetTag();
  }
}

static bool SortMoveNodeInfo(const HRMoveNodeInfo &lhs, const HRMoveNodeInfo &rhs) {
  return lhs.index_ < rhs.index_;
}

void HRViewManager::MoveRenderView(std::vector<HRMoveNodeInfo> nodeInfos, uint32_t parentTag) {
  auto it = view_registry_.find(parentTag);
  std::shared_ptr<BaseView> parentView = it != view_registry_.end() ? it->second : nullptr;
  if (!parentView) {
    FOOTSTONE_LOG(WARNING) << "MoveRenderView fail";
    return;
  }
  
  std::sort(nodeInfos.begin(), nodeInfos.end(), SortMoveNodeInfo);
  for (uint32_t i = 0; i < nodeInfos.size(); i++) {
    auto &info = nodeInfos[i];
    auto childIt = view_registry_.find(info.tag_);
    if (childIt != view_registry_.end()) {
      auto &child = childIt->second;
      child->RemoveFromParentView();
      parentView->AddSubRenderView(child, info.index_);
    }
  }
}

void HRViewManager::Move2RenderView(std::vector<uint32_t> tags, uint32_t newParentTag, uint32_t oldParentTag, int index) {
  auto it1 = view_registry_.find(oldParentTag);
  std::shared_ptr<BaseView> oldParent = it1 != view_registry_.end() ? it1->second : nullptr;
  auto it2 = view_registry_.find(newParentTag);
  std::shared_ptr<BaseView> newParent = it2 != view_registry_.end() ? it2->second : nullptr;
  if (!oldParent || !newParent) {
    FOOTSTONE_LOG(WARNING) << "Move2RenderView fail, oldParent=" << oldParentTag << ", newParent=" << newParentTag;
    return;
  }
  
  for (uint32_t i = 0; i < tags.size(); i++) {
    auto childIt = view_registry_.find(tags[i]);
    if (childIt != view_registry_.end()) {
      auto &child = childIt->second;
      child->RemoveFromParentView();
      newParent->AddSubRenderView(child, (int)i + index);
    }
  }
}

void HRViewManager::UpdateProps(std::shared_ptr<BaseView> &view, const HippyValueObjectType &props, const std::vector<std::string> &deleteProps) {
  if (view) {
    if (props.size() > 0) {
      for (auto it = props.begin(); it != props.end(); it++) {
        // value maybe empty string / false / 0
        auto &key = it->first;
        if (key.length() > 0) {
          view->SetProp(key, it->second);
        }
      }
    }
    if (deleteProps.size() > 0) {
      for (auto it = deleteProps.begin(); it != deleteProps.end(); it++) {
        auto &key = *it;
        if (key.length() > 0) {
          view->SetProp(key, HippyValue::Null());
        }
      }
    }
    view->OnSetPropsEnd();
  }
}

void HRViewManager::UpdateCustomTsProps(std::shared_ptr<BaseView> &view, const HippyValueObjectType &props, const std::vector<std::string> &deleteProps) {
  
}

void HRViewManager::UpdateProps(uint32_t tag, const HippyValueObjectType &props, const std::vector<std::string> &deleteProps) {
  auto it = view_registry_.find(tag);
  std::shared_ptr<BaseView> renderView = it != view_registry_.end() ? it->second : nullptr;
  UpdateProps(renderView, props, deleteProps);
}

void HRViewManager::UpdateEventListener(uint32_t tag, HippyValueObjectType &props) {
  auto it = view_registry_.find(tag);
  std::shared_ptr<BaseView> renderView = it != view_registry_.end() ? it->second : nullptr;
  if (renderView) {
    renderView->UpdateEventListener(props);
  }
}

bool HRViewManager::CheckRegisteredEvent(uint32_t tag, std::string &eventName) {
  auto it = view_registry_.find(tag);
  std::shared_ptr<BaseView> renderView = it != view_registry_.end() ? it->second : nullptr;
  if (renderView) {
    return renderView->CheckRegisteredEvent(eventName);
  }
  return false;
}

void HRViewManager::SetRenderViewFrame(uint32_t tag, const HRRect &frame, const HRPadding &padding) {
  auto it = view_registry_.find(tag);
  std::shared_ptr<BaseView> renderView = it != view_registry_.end() ? it->second : nullptr;
  if (renderView) {
    renderView->SetRenderViewFrame(frame, padding);
  }
}

void HRViewManager::CallViewMethod(uint32_t tag, const std::string &method, const std::vector<HippyValue> params,
                    std::function<void(const HippyValue &result)> callback) {
  auto it = view_registry_.find(tag);
  std::shared_ptr<BaseView> renderView = it != view_registry_.end() ? it->second : nullptr;
  if (renderView) {
    renderView->Call(method, params, callback);
  }
}

uint64_t HRViewManager::AddEndBatchCallback(const EndBatchCallback &cb) {
  ++end_batch_callback_id_count_;
  end_batch_callback_map_[end_batch_callback_id_count_] = std::move(cb);
  return end_batch_callback_id_count_;
}

void HRViewManager::RemoveEndBatchCallback(uint64_t cbId) {
  end_batch_callback_map_.erase(cbId);
}

void HRViewManager::NotifyEndBatchCallbacks() {
  for (const auto &callback : end_batch_callback_map_) {
    auto &cb = callback.second;
    cb();
  }
}

} // namespace native
} // namespace render
} // namespace hippy
