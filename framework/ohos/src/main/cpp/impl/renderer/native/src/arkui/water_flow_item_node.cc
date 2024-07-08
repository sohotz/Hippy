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

#include "renderer/arkui/water_flow_item_node.h"
#include "renderer/arkui/native_node_api.h"

namespace hippy {
inline namespace render {
inline namespace native {

static constexpr ArkUI_NodeEventType FLOW_ITEM_NODE_EVENT_TYPES[] = {
  NODE_EVENT_ON_VISIBLE_AREA_CHANGE
};

WaterFlowItemNode::WaterFlowItemNode()
    : ArkUINode(NativeNodeApi::GetInstance()->createNode(ArkUI_NodeType::ARKUI_NODE_FLOW_ITEM)) {
  for (auto eventType : FLOW_ITEM_NODE_EVENT_TYPES) {
    ArkUI_NumberValue value[] = {{.f32 = 0.f},{.f32 = 1.f}};
    ArkUI_AttributeItem item = {value, sizeof(value) / sizeof(ArkUI_NumberValue), nullptr, nullptr};
    MaybeThrow(NativeNodeApi::GetInstance()->registerNodeEvent(nodeHandle_, eventType, 0, &item));
  }    
}

WaterFlowItemNode::~WaterFlowItemNode() {
  for (auto eventType : FLOW_ITEM_NODE_EVENT_TYPES) {
    NativeNodeApi::GetInstance()->unregisterNodeEvent(nodeHandle_, eventType);
  }
}

void WaterFlowItemNode::SetNodeDelegate(FlowItemNodeDelegate* delegate){
    flowItemNodeDelegate_ = delegate;
}
void WaterFlowItemNode::OnNodeEvent(ArkUI_NodeEvent *event) {
  if (flowItemNodeDelegate_ == nullptr) {
    return;
  }
  auto eventType = OH_ArkUI_NodeEvent_GetEventType(event);
//  FOOTSTONE_DLOG(INFO)<<__FUNCTION__<<" event = "<<eventType; 
  auto nodeComponentEvent = OH_ArkUI_NodeEvent_GetNodeComponentEvent(event);

  if (eventType == ArkUI_NodeEventType::NODE_EVENT_ON_VISIBLE_AREA_CHANGE) {
    bool isVisible = nodeComponentEvent->data[0].i32;
    float currentRatio = nodeComponentEvent->data[1].f32;
    flowItemNodeDelegate_->OnItemVisibleAreaChange(itemIndex_,isVisible,currentRatio);
  }
}

void WaterFlowItemNode::SetConstraintSize(float minWidth,float maxWidth,float minHeight,float maxHeight) {
  ArkUI_NumberValue value[] = {{.f32 = minWidth},{.f32 = maxWidth},{.f32 = minHeight},{.f32 = maxHeight}};
  ArkUI_AttributeItem item = {value, sizeof(value) / sizeof(ArkUI_NumberValue), nullptr, nullptr};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_WATER_FLOW_ITEM_CONSTRAINT_SIZE, &item));
}

void WaterFlowItemNode::SetPadding(float top, float right, float bottom, float left){
  ArkUI_NumberValue value[] = {{.f32 = top}, {.f32 = right}, {.f32 = bottom}, {.f32 = left}};
  ArkUI_AttributeItem item = {value, sizeof(value) / sizeof(ArkUI_NumberValue), nullptr, nullptr};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_PADDING, &item));
}

} // namespace native
} // namespace render
} // namespace hippy
