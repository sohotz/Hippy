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

#include "renderer/arkui/refresh_node.h"
#include "renderer/arkui/native_node_api.h"

namespace hippy {
inline namespace render {
inline namespace native {

RefreshNode::RefreshNode()
    : ArkUINode(NativeNodeApi::GetInstance()->createNode(ArkUI_NodeType::ARKUI_NODE_REFRESH)) {
}

RefreshNode::~RefreshNode() {}

void RefreshNode::SetNodeDelegate(RefreshNodeDelegate *refreshNodeDelegate){
   refreshNodeDelegate_ =  refreshNodeDelegate;
}

void RefreshNode::OnNodeEvent(ArkUI_NodeEvent *event) {
  if (!refreshNodeDelegate_) {
    return;
  }
  auto eventType = OH_ArkUI_NodeEvent_GetEventType(event);
  auto nodeComponentEvent = OH_ArkUI_NodeEvent_GetNodeComponentEvent(event);
  if (eventType == ArkUI_NodeEventType::NODE_REFRESH_STATE_CHANGE) {
    int32_t state = nodeComponentEvent->data[0].i32;    
    refreshNodeDelegate_->OnStateChange(state);
  } else if(eventType == ArkUI_NodeEventType::NODE_REFRESH_ON_REFRESH){
    refreshNodeDelegate_->OnRefreshing();    
  } else if(eventType == ArkUI_NodeEventType::NODE_REFRESH_ON_OFFSET_CHANGE){
    float offset = nodeComponentEvent->data[0].f32;    
    refreshNodeDelegate_->OnOffsetChange(offset);
  }
}

} // namespace native
} // namespace render
} // namespace hippy
