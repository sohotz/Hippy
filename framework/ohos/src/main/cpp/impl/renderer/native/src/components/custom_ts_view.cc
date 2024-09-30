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
#include "renderer/components/custom_ts_view.h"
#include "oh_napi/ark_ts.h"
#include "oh_napi/oh_napi_object.h"
#include "oh_napi/oh_napi_object_builder.h"
#include "renderer/utils/hr_value_utils.h"

namespace hippy {
inline namespace render {
inline namespace native {

CustomTsView::CustomTsView(std::shared_ptr<NativeRenderContext> &ctx, ArkUI_NodeHandle nodeHandle) : BaseView(ctx), tsNode_(nodeHandle) {
  containerNode_.AddChild(tsNode_);
  containerNode_.AddChild(subContainerNode_);
  subContainerNode_.SetWidthPercent(1.f);
  subContainerNode_.SetHeightPercent(1.f);
  subContainerNode_.SetHitTestMode(ARKUI_HIT_TEST_MODE_NONE);
  subContainerNode_.SetClip(true);
}

CustomTsView::~CustomTsView() {
  if (!children_.empty()) {
    for (const auto &child : children_) {
      subContainerNode_.RemoveChild(child->GetLocalRootArkUINode());
    }
    children_.clear();
  }
  containerNode_.RemoveChild(tsNode_);
  containerNode_.RemoveChild(subContainerNode_);
}

StackNode &CustomTsView::GetLocalRootArkUINode() {
  return containerNode_;
}

bool CustomTsView::SetProp(const std::string &propKey, const HippyValue &propValue) {
  return BaseView::SetProp(propKey, propValue);
}

void CustomTsView::UpdateRenderViewFrame(const HRRect &frame, const HRPadding &padding) {
  BaseView::UpdateRenderViewFrame(frame, padding);
}

void CustomTsView::OnChildInserted(std::shared_ptr<BaseView> const &childView, int32_t index) {
  BaseView::OnChildInserted(childView, index);
  subContainerNode_.InsertChild(childView->GetLocalRootArkUINode(), index);
  
  OnCustomTsViewChildInserted(tag_, childView, index);
}

void CustomTsView::OnChildRemoved(std::shared_ptr<BaseView> const &childView, int32_t index) {
  BaseView::OnChildRemoved(childView, index);
  subContainerNode_.RemoveChild(childView->GetLocalRootArkUINode());
  
  OnCustomTsViewChildRemoved(tag_, childView, index);
}

void CustomTsView::OnCustomTsViewChildInserted(uint32_t tag, std::shared_ptr<BaseView> const &childView, int32_t index) {
  ArkTS arkTs(ts_env_);

  auto params_builder = arkTs.CreateObjectBuilder();
  params_builder.AddProperty("rootTag", ctx_->GetRootId());
  params_builder.AddProperty("tag", tag);
  params_builder.AddProperty("childTag", childView->GetTag());
  params_builder.AddProperty("childViewName", childView->GetViewType());
  params_builder.AddProperty("childIndex", index);
  
  std::vector<napi_value> args = {
    params_builder.Build()
  };
  
  auto delegateObject = arkTs.GetObject(ts_render_provider_ref_);
  delegateObject.Call("onChildInsertedForCApi", args);
}

void CustomTsView::OnCustomTsViewChildRemoved(uint32_t tag, std::shared_ptr<BaseView> const &childView, int32_t index) {
  ArkTS arkTs(ts_env_);

  auto params_builder = arkTs.CreateObjectBuilder();
  params_builder.AddProperty("rootTag", ctx_->GetRootId());
  params_builder.AddProperty("tag", tag);
  params_builder.AddProperty("childTag", childView->GetTag());
  params_builder.AddProperty("childViewName", childView->GetViewType());
  params_builder.AddProperty("childIndex", index);
  
  std::vector<napi_value> args = {
    params_builder.Build()
  };
  
  auto delegateObject = arkTs.GetObject(ts_render_provider_ref_);
  delegateObject.Call("onChildRemovedForCApi", args);
}

} // namespace native
} // namespace render
} // namespace hippy
