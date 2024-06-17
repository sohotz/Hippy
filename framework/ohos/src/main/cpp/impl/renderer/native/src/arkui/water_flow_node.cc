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

#include "renderer/arkui/water_flow_node.h"
#include "renderer/arkui/native_node_api.h"
#include "renderer/arkui/arkui_node_registry.h"

namespace hippy {
inline namespace render {
inline namespace native {

WaterFlowNode::WaterFlowNode()
    : ArkUINode(NativeNodeApi::GetInstance()->createNode(ArkUI_NodeType::ARKUI_NODE_WATER_FLOW)) {
}

WaterFlowNode::~WaterFlowNode() {}

void WaterFlowNode::AddChild(ArkUINode &child) {
  MaybeThrow(NativeNodeApi::GetInstance()->addChild(nodeHandle_, child.GetArkUINodeHandle()));
}

void WaterFlowNode::InsertChild(ArkUINode &child, int32_t index) {
  MaybeThrow(
    NativeNodeApi::GetInstance()->insertChildAt(nodeHandle_, child.GetArkUINodeHandle(), static_cast<int32_t>(index)));
}

void WaterFlowNode::RemoveChild(ArkUINode &child) {
  MaybeThrow(NativeNodeApi::GetInstance()->removeChild(nodeHandle_, child.GetArkUINodeHandle()));
}

void WaterFlowNode::RemoveAllChildren() {
  uint32_t count = NativeNodeApi::GetInstance()->getTotalChildCount(nodeHandle_);
  for (int32_t i = static_cast<int32_t>(count) - 1; i >= 0; i--) {
    ArkUI_NodeHandle childHandle = NativeNodeApi::GetInstance()->getChildAt(nodeHandle_, i);
    if (childHandle) {
      MaybeThrow(NativeNodeApi::GetInstance()->removeChild(nodeHandle_, childHandle));
    }
  }
}
// void WaterFlowNode::OnNodeEvent(ArkUI_NodeEvent *event) {
//   if (WaterFlowNodeDelegate_ == nullptr) {
//     return;
//   }
//   auto eventType = OH_ArkUI_NodeEvent_GetEventType(event);
//   auto nodeComponentEvent = OH_ArkUI_NodeEvent_GetNodeComponentEvent(event);
//   if (eventType == ArkUI_NodeEventType::NODE_WATER_FLOW_ON_SCROLL_INDEX) {
//     int32_t firstIndex = nodeComponentEvent->data[0].i32;
//     int32_t lastIndex = nodeComponentEvent->data[1].i32;
//     int32_t centerIndex = nodeComponentEvent->data[2].i32;
//     WaterFlowNodeDelegate_->OnScrollIndex(firstIndex, lastIndex, centerIndex);
//   } else if (eventType == ArkUI_NodeEventType::NODE_SCROLL_EVENT_ON_SCROLL) {
//     float x = nodeComponentEvent->data[0].f32;
//     float y = nodeComponentEvent->data[1].f32;
//     WaterFlowNodeDelegate_->OnScroll(x, y);
//   } else if (eventType == ArkUI_NodeEventType::NODE_SCROLL_EVENT_ON_SCROLL_START) {
//     WaterFlowNodeDelegate_->OnScrollStart();
//   } else if (eventType == ArkUI_NodeEventType::NODE_SCROLL_EVENT_ON_SCROLL_STOP) {
//     WaterFlowNodeDelegate_->OnScrollStop();
//   } else if (eventType == ArkUI_NodeEventType::NODE_SCROLL_EVENT_ON_REACH_START) {
//     WaterFlowNodeDelegate_->OnReachStart();
//   } else if (eventType == ArkUI_NodeEventType::NODE_SCROLL_EVENT_ON_REACH_END) {
//     WaterFlowNodeDelegate_->OnReachEnd();
//   }
// }
HRPoint WaterFlowNode::GetScrollOffset() {
  auto item = NativeNodeApi::GetInstance()->getAttribute(nodeHandle_, NODE_SCROLL_OFFSET);
  float x = item->value[0].f32;
  float y = item->value[1].f32;
  return HRPoint(x, y);
}

void WaterFlowNode::SetScrollEdgeEffect(bool hasEffect) {
  ArkUI_NumberValue value[] = {{.i32 = hasEffect ? ARKUI_EDGE_EFFECT_SPRING : ARKUI_EDGE_EFFECT_NONE},
                               {.i32 = hasEffect ? 1 : 0}};
  ArkUI_AttributeItem item = {value, sizeof(value) / sizeof(ArkUI_NumberValue), nullptr, nullptr};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_SCROLL_EDGE_EFFECT, &item));
}

void WaterFlowNode::ScrollTo(float offsetX, float offsetY, bool animated) {
  ArkUI_NumberValue value[] = {{.f32 = offsetX},
                               {.f32 = offsetY},
                               {.i32 = animated ? 1000 : 0},
                               {.i32 = ArkUI_AnimationCurve::ARKUI_CURVE_SMOOTH},
                               {.i32 = 0}};
  ArkUI_AttributeItem item = {value, sizeof(value) / sizeof(ArkUI_NumberValue), nullptr, nullptr};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_SCROLL_OFFSET, &item));
}

void WaterFlowNode::ScrollToIndex(int32_t index, bool animated, bool isScrollAlignStart) {
  ArkUI_NumberValue value[] = {{.i32 = index},
                               {.i32 = animated},
                               {.i32 = isScrollAlignStart ? ARKUI_SCROLL_ALIGNMENT_START : ARKUI_SCROLL_ALIGNMENT_END}};
  ArkUI_AttributeItem item = {value, sizeof(value) / sizeof(ArkUI_NumberValue), nullptr, nullptr};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_LIST_SCROLL_TO_INDEX, &item));
}

void WaterFlowNode::SetColumnsTemplate(std::string columnsTemplate) {
//   ArkUI_NumberValue value[] = {{.string  = columnsTemplate}};
  ArkUI_AttributeItem item = {.string  = columnsTemplate.c_str()};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_WATER_FLOW_COLUMN_TEMPLATE, &item));
}

void WaterFlowNode::SetNodeDelegate(WaterFlowNodeDelegate *waterFlowNodeDelegate) {
  WaterFlowNodeDelegate_ = waterFlowNodeDelegate;
}

// void WaterFlowNode::OnTouchEvent(ArkUI_UIInputEvent *event) {
//   if (WaterFlowNodeDelegate_ == nullptr) {
//     return;
//   }
//   auto type = OH_ArkUI_UIInputEvent_GetType(event);
//   WaterFlowNodeDelegate_->OnTouch(type);
// }
} // namespace native
} // namespace render
} // namespace hippy
