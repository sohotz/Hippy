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

#include "renderer/arkui/list_node.h"
#include "renderer/arkui/native_node_api.h"

namespace hippy {
inline namespace render {
inline namespace native {

static constexpr ArkUI_NodeEventType LIST_NODE_EVENT_TYPES[] = {
  NODE_EVENT_ON_APPEAR,
  NODE_EVENT_ON_DISAPPEAR,
  NODE_LIST_ON_SCROLL_INDEX,
  NODE_SCROLL_EVENT_ON_SCROLL,
  NODE_SCROLL_EVENT_ON_SCROLL_START,
  NODE_SCROLL_EVENT_ON_SCROLL_STOP,
  NODE_SCROLL_EVENT_ON_REACH_START,
  NODE_SCROLL_EVENT_ON_REACH_END
};

ListNode::ListNode()
    : ArkUINode(NativeNodeApi::GetInstance()->createNode(ArkUI_NodeType::ARKUI_NODE_LIST)) {
  for (auto eventType : LIST_NODE_EVENT_TYPES) {
    MaybeThrow(NativeNodeApi::GetInstance()->registerNodeEvent(nodeHandle_, eventType, 0, nullptr));
  }
}

ListNode::~ListNode() {
  for (auto eventType : LIST_NODE_EVENT_TYPES) {
    NativeNodeApi::GetInstance()->unregisterNodeEvent(nodeHandle_, eventType);
  }
}

void ListNode::AddChild(ArkUINode &child) {
  MaybeThrow(NativeNodeApi::GetInstance()->addChild(nodeHandle_, child.GetArkUINodeHandle()));
}

void ListNode::InsertChild(ArkUINode &child, int32_t index) {
  MaybeThrow(
    NativeNodeApi::GetInstance()->insertChildAt(nodeHandle_, child.GetArkUINodeHandle(), static_cast<int32_t>(index)));
}

void ListNode::RemoveChild(ArkUINode &child) {
  MaybeThrow(NativeNodeApi::GetInstance()->removeChild(nodeHandle_, child.GetArkUINodeHandle()));
}

void ListNode::RemoveAllChildren() {
  uint32_t count = NativeNodeApi::GetInstance()->getTotalChildCount(nodeHandle_);
  for (int32_t i = static_cast<int32_t>(count) - 1; i >= 0; i--) {
    ArkUI_NodeHandle childHandle = NativeNodeApi::GetInstance()->getChildAt(nodeHandle_, i);
    if (childHandle) {
      MaybeThrow(NativeNodeApi::GetInstance()->removeChild(nodeHandle_, childHandle));
    }
  }
}

void ListNode::OnNodeEvent(ArkUI_NodeEvent *event) {
  if (listNodeDelegate_ == nullptr) {
    return;
  }

  auto eventType = OH_ArkUI_NodeEvent_GetEventType(event);
  auto nodeComponentEvent = OH_ArkUI_NodeEvent_GetNodeComponentEvent(event);
  if (eventType == ArkUI_NodeEventType::NODE_EVENT_ON_APPEAR) {
    listNodeDelegate_->OnAppear();
  } else if (eventType == ArkUI_NodeEventType::NODE_EVENT_ON_DISAPPEAR) {
    listNodeDelegate_->OnDisappear();
  } else if (eventType == ArkUI_NodeEventType::NODE_LIST_ON_SCROLL_INDEX) {
    int32_t firstIndex = nodeComponentEvent->data[0].i32;
    int32_t lastIndex = nodeComponentEvent->data[1].i32;
    int32_t centerIndex = nodeComponentEvent->data[2].i32;
    listNodeDelegate_->OnScrollIndex(firstIndex, lastIndex, centerIndex);
  } else if (eventType == ArkUI_NodeEventType::NODE_SCROLL_EVENT_ON_SCROLL) {
    float x = nodeComponentEvent->data[0].f32;
    float y = nodeComponentEvent->data[1].f32;
    listNodeDelegate_->OnScroll(x, y);
  } else if (eventType == ArkUI_NodeEventType::NODE_SCROLL_EVENT_ON_SCROLL_START) {
    listNodeDelegate_->OnScrollStart();
  } else if (eventType == ArkUI_NodeEventType::NODE_SCROLL_EVENT_ON_SCROLL_STOP) {
    listNodeDelegate_->OnScrollStop();
  } else if (eventType == ArkUI_NodeEventType::NODE_SCROLL_EVENT_ON_REACH_START) {
    listNodeDelegate_->OnReachStart();
  } else if (eventType == ArkUI_NodeEventType::NODE_SCROLL_EVENT_ON_REACH_END) {
    listNodeDelegate_->OnReachEnd();
  }
}

void ListNode::SetNodeDelegate(ListNodeDelegate *listNodeDelegate) { listNodeDelegate_ = listNodeDelegate; }

} // namespace native
} // namespace render
} // namespace hippy
