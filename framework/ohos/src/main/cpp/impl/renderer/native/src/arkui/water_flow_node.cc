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

namespace hippy {
inline namespace render {
inline namespace native {

static constexpr ArkUI_NodeEventType WATER_FLOW_NODE_EVENT_TYPES[] = {
  NODE_WATER_FLOW_ON_SCROLL_INDEX,
  NODE_WATER_FLOW_ON_DID_SCROLL,
  NODE_ON_WILL_SCROLL,
  NODE_SCROLL_EVENT_ON_SCROLL_START,
  NODE_SCROLL_EVENT_ON_SCROLL_STOP,
  NODE_SCROLL_EVENT_ON_REACH_START,
  NODE_SCROLL_EVENT_ON_REACH_END,
  NODE_TOUCH_EVENT,
};

WaterFlowNode::WaterFlowNode()
    : ArkUINode(NativeNodeApi::GetInstance()->createNode(ArkUI_NodeType::ARKUI_NODE_WATER_FLOW)) {
  for (auto eventType : WATER_FLOW_NODE_EVENT_TYPES) {
    MaybeThrow(NativeNodeApi::GetInstance()->registerNodeEvent(nodeHandle_, eventType, 0, nullptr));
  }    
}

WaterFlowNode::~WaterFlowNode() {
  for (auto eventType : WATER_FLOW_NODE_EVENT_TYPES) {
    NativeNodeApi::GetInstance()->unregisterNodeEvent(nodeHandle_, eventType);
  }
}

void WaterFlowNode::SetNodeDelegate(WaterFlowNodeDelegate *delegate) {
  waterFlowNodeDelegate_ = delegate;
}

void WaterFlowNode::OnNodeEvent(ArkUI_NodeEvent *event) {
  if (waterFlowNodeDelegate_ == nullptr) {
    return;
  }
  auto eventType = OH_ArkUI_NodeEvent_GetEventType(event);
//  FOOTSTONE_DLOG(INFO)<<__FUNCTION__<<" event = "<<eventType; 
  auto nodeComponentEvent = OH_ArkUI_NodeEvent_GetNodeComponentEvent(event);
  if (eventType == ArkUI_NodeEventType::NODE_WATER_FLOW_ON_SCROLL_INDEX) {
    int32_t firstIndex = nodeComponentEvent->data[0].i32;
    int32_t lastIndex = nodeComponentEvent->data[1].i32;
    waterFlowNodeDelegate_->OnWaterFlowScrollIndex(firstIndex, lastIndex);
  } else if(eventType == ArkUI_NodeEventType::NODE_WATER_FLOW_ON_DID_SCROLL){
    float_t offset = nodeComponentEvent->data[0].f32;
    int32_t state = nodeComponentEvent->data[1].i32;  
    waterFlowNodeDelegate_->OnWaterFlowDidScroll(offset, (ArkUI_ScrollState)state);    
  } else if(eventType == ArkUI_NodeEventType::NODE_ON_WILL_SCROLL){
    float_t offset = nodeComponentEvent->data[0].f32;
    int32_t state = nodeComponentEvent->data[1].i32; 
    int32_t source = nodeComponentEvent->data[2].i32; 
    waterFlowNodeDelegate_->OnWaterFlowWillScroll(offset, ArkUI_ScrollState(state), source);
  } else if (eventType == ArkUI_NodeEventType::NODE_TOUCH_EVENT) {
    ArkUI_UIInputEvent *inputEvent = OH_ArkUI_NodeEvent_GetInputEvent(event);
    auto type = OH_ArkUI_UIInputEvent_GetType(inputEvent);
    if (type == ARKUI_UIINPUTEVENT_TYPE_TOUCH) {
      auto action = OH_ArkUI_UIInputEvent_GetAction(inputEvent);
      waterFlowNodeDelegate_->OnTouch(action);
    }
  } else if (eventType == ArkUI_NodeEventType::NODE_SCROLL_EVENT_ON_SCROLL_START){
    waterFlowNodeDelegate_->OnScrollStart();    
  } else if (eventType == ArkUI_NodeEventType::NODE_SCROLL_EVENT_ON_SCROLL_STOP){
    waterFlowNodeDelegate_->OnScrollStop();
  } else if (eventType == ArkUI_NodeEventType::NODE_SCROLL_EVENT_ON_REACH_START){
    waterFlowNodeDelegate_->OnReachStart();
  } else if (eventType == ArkUI_NodeEventType::NODE_SCROLL_EVENT_ON_REACH_END){
    waterFlowNodeDelegate_->OnReachEnd();
  } else{
    FOOTSTONE_DLOG(INFO)<<__FUNCTION__<<" event = "<<eventType;    
  }
    
}

HRPoint WaterFlowNode::GetScrollOffset() {
  auto item = NativeNodeApi::GetInstance()->getAttribute(nodeHandle_, NODE_SCROLL_OFFSET);
  float x = item->value[0].f32;
  float y = item->value[1].f32;
  return HRPoint(x, y);
}

void WaterFlowNode::SetScrollEdgeEffect(ArkUI_EdgeEffect effect) {
  ArkUI_NumberValue value[] = {{.i32 = effect},};
  ArkUI_AttributeItem item = {value, sizeof(value) / sizeof(ArkUI_NumberValue), nullptr, nullptr};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_SCROLL_EDGE_EFFECT, &item));
}

void WaterFlowNode::ScrollToIndex(int32_t index, bool animated,ArkUI_ScrollAlignment align) {
  ArkUI_NumberValue value[] = {{.i32 = index},
                               {.i32 = animated},
                               {.i32 = align }};
  ArkUI_AttributeItem item = {value, sizeof(value) / sizeof(ArkUI_NumberValue), nullptr, nullptr};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_WATER_FLOW_SCROLL_TO_INDEX, &item));
}

void WaterFlowNode::SetColumnsTemplate(std::string columnsTemplate) {
  ArkUI_AttributeItem item = {.string  = columnsTemplate.c_str()};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_WATER_FLOW_COLUMN_TEMPLATE, &item));
}

void WaterFlowNode::SetRowTemplate(std::string rowsTemplate){
  ArkUI_AttributeItem item = {.string  = rowsTemplate.c_str()};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_WATER_FLOW_ROW_TEMPLATE, &item));
    
}

void WaterFlowNode::SetColumnGap(float_t gap){
  ArkUI_NumberValue value[] = {{.f32 = gap},};
  ArkUI_AttributeItem item = {value, sizeof(value) / sizeof(ArkUI_NumberValue), nullptr, nullptr};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_WATER_FLOW_COLUMN_GAP, &item));
}

void WaterFlowNode::SetRowGap(float_t gap){
  ArkUI_NumberValue value[] = {{.f32 = gap},};
  ArkUI_AttributeItem item = {value, sizeof(value) / sizeof(ArkUI_NumberValue), nullptr, nullptr};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_WATER_FLOW_ROW_GAP, &item));
}

void WaterFlowNode::SetCachedCount(int32_t count){
  ArkUI_NumberValue value[] = {{.i32 = count},};
  ArkUI_AttributeItem item = {value, sizeof(value) / sizeof(ArkUI_NumberValue), nullptr, nullptr};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_WATER_FLOW_CACHED_COUNT, &item));
}

void WaterFlowNode::SetLayoutDirection(ArkUI_FlexDirection direction){
  ArkUI_NumberValue value[] = {{.i32 = direction},};
  ArkUI_AttributeItem item = {value, sizeof(value) / sizeof(ArkUI_NumberValue), nullptr, nullptr};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_WATER_FLOW_LAYOUT_DIRECTION, &item));
}

void WaterFlowNode::SetScrollEnableInteraction(bool bEnable){
  ArkUI_NumberValue value[] = {{.i32 = bEnable},};
  ArkUI_AttributeItem item = {value, sizeof(value) / sizeof(ArkUI_NumberValue), nullptr, nullptr};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_SCROLL_ENABLE_SCROLL_INTERACTION, &item));
}

void WaterFlowNode::SetNestedScroll(ArkUI_ScrollNestedMode forward, ArkUI_ScrollNestedMode backward){
  ArkUI_NumberValue value[] = {{.i32 = forward},{.i32 = backward}}; 
  ArkUI_AttributeItem item = {value, sizeof(value) / sizeof(ArkUI_NumberValue), nullptr, nullptr};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_SCROLL_NESTED_SCROLL, &item));
}

void WaterFlowNode::SetScrollBarDisplayMode(ArkUI_ScrollBarDisplayMode mode){
  ArkUI_NumberValue value[] = {{.i32 = mode},}; 
  ArkUI_AttributeItem item = {value, sizeof(value) / sizeof(ArkUI_NumberValue), nullptr, nullptr};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_SCROLL_BAR_DISPLAY_MODE, &item));
}

void WaterFlowNode::SetFooter(ArkUI_NodeHandle footer){
  ArkUI_AttributeItem item = {nullptr, 0, nullptr, footer};  
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_WATER_FLOW_FOOTER, &item));
}

} // namespace native
} // namespace render
} // namespace hippy
