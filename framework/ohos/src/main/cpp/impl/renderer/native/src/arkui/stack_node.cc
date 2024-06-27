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

#include "renderer/arkui/stack_node.h"
#include "renderer/arkui/native_node_api.h"

namespace hippy {
inline namespace render {
inline namespace native {

StackNode::StackNode() 
    : ArkUINode(NativeNodeApi::GetInstance()->createNode(ArkUI_NodeType::ARKUI_NODE_STACK)),
      stackNodeDelegate_(nullptr) {
  SetStackAlignContent(ARKUI_ALIGNMENT_TOP_START);
}

StackNode::~StackNode() {}

StackNode &StackNode::SetStackAlignContent(ArkUI_Alignment align) {
  ArkUI_NumberValue value[] = {{.i32 = align}};
  ArkUI_AttributeItem item = {.value = value, .size = 1};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_STACK_ALIGN_CONTENT, &item));
  return *this;
}

void StackNode::SetStackNodeDelegate(StackNodeDelegate *stackNodeDelegate) { stackNodeDelegate_ = stackNodeDelegate; }

void StackNode::OnNodeEvent(ArkUI_NodeEvent *event) {
  if (stackNodeDelegate_ == nullptr) {
    return;
  }
  
  auto eventType = OH_ArkUI_NodeEvent_GetEventType(event);
  if (eventType == ArkUI_NodeEventType::NODE_ON_CLICK) {
    stackNodeDelegate_->OnClick();
  } else if (eventType == ArkUI_NodeEventType::NODE_EVENT_ON_APPEAR) {
    stackNodeDelegate_->OnAppear();
  } else if (eventType == ArkUI_NodeEventType::NODE_EVENT_ON_DISAPPEAR) {
    stackNodeDelegate_->OnDisappear();
  } else if (eventType == ArkUI_NodeEventType::NODE_EVENT_ON_AREA_CHANGE) {
    auto nodeComponentEvent = OH_ArkUI_NodeEvent_GetNodeComponentEvent(event);
    ArkUI_NumberValue* data = nodeComponentEvent->data;   
    stackNodeDelegate_->OnAreaChange(data);    
  }
}


} // namespace native
} // namespace render
} // namespace hippy
