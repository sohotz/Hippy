
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

static constexpr ArkUI_NodeEventType WATER_FLOW_ITEM_NODE_EVENT_TYPES[] = {
  NODE_EVENT_ON_VISIBLE_AREA_CHANGE
};

WaterFlowItemNode::WaterFlowItemNode() : ArkUINode(NativeNodeApi::GetInstance()->createNode(ArkUI_NodeType::ARKUI_NODE_FLOW_ITEM)) {
  for (auto eventType : NODE_EVENT_ON_VISIBLE_AREA_CHANGE) {
    NativeNodeApi::GetInstance()->registerNodeEvent(nodeHandle_, eventType, 0, &item);
  }
}

WaterFlowItemNode::~WaterFlowItemNode() {
  for (auto eventType : WATER_FLOW_ITEM_NODE_EVENT_TYPES) {
    NativeNodeApi::GetInstance()->unregisterNodeEvent(nodeHandle_, eventType);
  }
}

void WaterFlowItemNode::AddChild(ArkUINode &child) {
  MaybeThrow(NativeNodeApi::GetInstance()->addChild(nodeHandle_, child.GetArkUINodeHandle()));
}

void WaterFlowItemNode::InsertChild(ArkUINode &child, int32_t index) {
  MaybeThrow(
    NativeNodeApi::GetInstance()->insertChildAt(nodeHandle_, child.GetArkUINodeHandle(), static_cast<int32_t>(index)));
}

void WaterFlowItemNode::RemoveChild(ArkUINode &child) {
  MaybeThrow(NativeNodeApi::GetInstance()->removeChild(nodeHandle_, child.GetArkUINodeHandle()));
}

void WaterFlowItemNode::OnNodeEvent(ArkUI_NodeEvent *event) {
  if (WaterFlowItemNodeDelegate_ == nullptr) {
    return;
  }

  auto eventType = OH_ArkUI_NodeEvent_GetEventType(event);
  auto nodeComponentEvent = OH_ArkUI_NodeEvent_GetNodeComponentEvent(event);
  if (eventType == ArkUI_NodeEventType::NODE_EVENT_ON_VISIBLE_AREA_CHANGE) {
    bool isVisible = nodeComponentEvent->data[0].i32;
    float currentRatio = nodeComponentEvent->data[1].f32;
    WaterFlowItemNodeDelegate_->OnItemVisibleAreaChange(itemIndex_, isVisible, currentRatio);
  }
}

void WaterFlowItemNode::SetNodeDelegate(WaterFlowItemNodeDelegate *nodeDelegate) { WaterFlowItemNodeDelegate_ = nodeDelegate; }

} // namespace native
} // namespace render
} // namespace hippy
