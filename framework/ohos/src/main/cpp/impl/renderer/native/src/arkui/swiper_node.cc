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

#include "renderer/arkui/swiper_node.h"
#include "renderer/arkui/native_node_api.h"
#include <bits/alltypes.h>
#include "footstone/logging.h"
#include "renderer/utils/hr_event_utils.h"
#include "renderer/arkui/arkui_node_registry.h"

namespace hippy {
inline namespace render {
inline namespace native {

static constexpr ArkUI_NodeEventType SWIPER_NODE_EVENT_TYPES[] = {
    NODE_SWIPER_EVENT_ON_CHANGE,        
    NODE_SWIPER_EVENT_ON_ANIMATION_START,
    NODE_SWIPER_EVENT_ON_ANIMATION_END, 
    NODE_SWIPER_EVENT_ON_CONTENT_DID_SCROLL,
    NODE_SWIPER_EVENT_ON_GESTURE_SWIPE, 
    NODE_TOUCH_EVENT
};

SwiperNode::SwiperNode()
    : ArkUINode(NativeNodeApi::GetInstance()->createNode(ArkUI_NodeType::ARKUI_NODE_SWIPER)) {
  for (auto eventType : SWIPER_NODE_EVENT_TYPES) {
    MaybeThrow(NativeNodeApi::GetInstance()->registerNodeEvent(nodeHandle_, eventType, 0, nullptr));
  }
}

SwiperNode::~SwiperNode() {
  for (auto eventType : SWIPER_NODE_EVENT_TYPES) {
    NativeNodeApi::GetInstance()->unregisterNodeEvent(nodeHandle_, eventType);
  }
}

void SwiperNode::SetNodeDelegate(SwiperNodeDelegate *swiperNodeDelegate) {
  swiperNodeDelegate_ = swiperNodeDelegate;
}

void SwiperNode::OnNodeEvent(ArkUI_NodeEvent *event) {
  if (swiperNodeDelegate_ == nullptr) {
    return;
  }
  auto eventType = OH_ArkUI_NodeEvent_GetEventType(event);
  auto nodeComponentEvent = OH_ArkUI_NodeEvent_GetNodeComponentEvent(event);
  if (eventType == ArkUI_NodeEventType::NODE_SWIPER_EVENT_ON_CHANGE) {
    int32_t index = nodeComponentEvent->data[0].i32;
    swiperNodeDelegate_->OnChange(index);
  } else if (eventType == ArkUI_NodeEventType::NODE_SWIPER_EVENT_ON_ANIMATION_START) {
    int32_t currentIndex = nodeComponentEvent->data[0].i32;
    int32_t targetIndex = nodeComponentEvent->data[1].i32;
    float_t currentOffset = nodeComponentEvent->data[2].f32;
    float_t targetOffset = nodeComponentEvent->data[3].f32;
    float_t swipeVelocity = nodeComponentEvent->data[4].f32;
    swiperNodeDelegate_->OnAnimationStart(currentIndex, targetIndex, currentOffset, targetOffset,
                                          swipeVelocity);
  } else if (eventType == ArkUI_NodeEventType::NODE_SWIPER_EVENT_ON_ANIMATION_END) {
    int32_t currentIndex = nodeComponentEvent->data[0].i32;
    float_t finalOffset = nodeComponentEvent->data[1].f32;
    swiperNodeDelegate_->OnAnimationEnd(currentIndex, finalOffset);
  } else if (eventType == ArkUI_NodeEventType::NODE_SWIPER_EVENT_ON_CONTENT_DID_SCROLL) {
    int32_t swiperPageIndex = nodeComponentEvent->data[0].i32;
    int32_t windowPageIndex = nodeComponentEvent->data[1].i32;
    float_t pageMoveRatio = nodeComponentEvent->data[2].f32;
    float_t pageAxisSize = nodeComponentEvent->data[3].f32;
    swiperNodeDelegate_->OnContentDidScroll(swiperPageIndex, windowPageIndex, pageMoveRatio,
                                            pageAxisSize);
  } else if (eventType == ArkUI_NodeEventType::NODE_SWIPER_EVENT_ON_GESTURE_SWIPE) {
    int32_t swiperPageIndex = nodeComponentEvent->data[0].i32;
    float_t elementOffsetFromStart = nodeComponentEvent->data[1].f32;
    swiperNodeDelegate_->OnGestureSwipe(swiperPageIndex, elementOffsetFromStart);
  } else if (eventType == ArkUI_NodeEventType::NODE_TOUCH_EVENT) {
    ArkUI_UIInputEvent *inputEvent = OH_ArkUI_NodeEvent_GetInputEvent(event);
    swiperNodeDelegate_->OnNodeTouchEvent(inputEvent);
  }
}

void SwiperNode::SetShowIndicator(bool show) {
  ArkUI_NumberValue value = {.i32 = int32_t(show)};
  ArkUI_AttributeItem item = {&value, 1, nullptr, nullptr};
  MaybeThrow(
      NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_SWIPER_SHOW_INDICATOR, &item));
}

void SwiperNode::SetSwiperIndex(int32_t index) {
  ArkUI_NumberValue value = {.i32 = int32_t(index)};
  ArkUI_AttributeItem item = {&value, 1, nullptr, nullptr};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_SWIPER_INDEX, &item));
}

void SwiperNode::SetSwiperSwipeToIndex(int32_t index, int32_t animation) {
  ArkUI_NumberValue value[] = {{.i32 = int32_t(index)}, {.i32 = int32_t(animation)}};
  ArkUI_AttributeItem item = {value, sizeof(value) / sizeof(ArkUI_NumberValue), nullptr, nullptr};
  MaybeThrow(
      NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_SWIPER_SWIPE_TO_INDEX, &item));
}

void SwiperNode::SetSwiperVertical(int32_t direction) {
  ArkUI_NumberValue value = {.i32 = int32_t(direction)};
  ArkUI_AttributeItem item = {&value, 1, nullptr, nullptr};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_SWIPER_VERTICAL, &item));
}

void SwiperNode::SetSwiperPrevMargin(float fValue) {
  ArkUI_NumberValue value = {.f32 = fValue};
  ArkUI_AttributeItem item = {&value, 1, nullptr, nullptr};
  MaybeThrow(
      NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_SWIPER_PREV_MARGIN, &item));
}

void SwiperNode::SetSwiperNextMargin(float fValue) {
  ArkUI_NumberValue value = {.f32 = fValue};
  ArkUI_AttributeItem item = {&value, 1, nullptr, nullptr};
  MaybeThrow(
      NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_SWIPER_NEXT_MARGIN, &item));
}

void SwiperNode::SetSwiperLoop(int32_t enable) {
  ArkUI_NumberValue value = {.i32 = enable};
  ArkUI_AttributeItem item = {&value, 1, nullptr, nullptr};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_SWIPER_LOOP, &item));
}

} // namespace native
} // namespace render
} // namespace hippy
