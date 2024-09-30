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

#include "renderer/components/pager_view.h"
#include "renderer/utils/hr_value_utils.h"
#include "renderer/utils/hr_event_utils.h"
#include "footstone/logging.h"

namespace hippy {
inline namespace render {
inline namespace native {

PagerView::PagerView(std::shared_ptr<NativeRenderContext> &ctx) : BaseView(ctx) {
  swiperNode_.SetNodeDelegate(this);
  GetLocalRootArkUINode().SetShowIndicator(false);
  GetLocalRootArkUINode().SetSwiperLoop(0);
}

PagerView::~PagerView() {
  if (!children_.empty()) {
    for (const auto &child : children_) {
      swiperNode_.RemoveChild(child->GetLocalRootArkUINode());
    }
    children_.clear();
  }
}

SwiperNode &PagerView::GetLocalRootArkUINode() { return swiperNode_; }

bool PagerView::SetProp(const std::string &propKey, const HippyValue &propValue) {
  if (propKey == "initialPage") {
    if (!initialPageUsed_) {
      initialPageUsed_ = true;
      initialPage_ = HRValueUtils::GetInt32(propValue);
      index_ = initialPage_;
      GetLocalRootArkUINode().SetSwiperIndex(index_);
    }
    return true;
  } else if (propKey == "scrollEnabled") {
    bool enable = HRValueUtils::GetBool(propValue, true);
    disableSwipe_ = !enable;
    GetLocalRootArkUINode().SetSwiperDisableSwipe(disableSwipe_);
    return true;
  } else if (propKey == "direction") {
    std::string directionVal;
    propValue.ToString(directionVal);
    if (directionVal == "vertical") {
      vertical_ = true;
      GetLocalRootArkUINode().SetSwiperVertical(1);
    }
    return true;
  } else if (propKey == "vertical") {
    vertical_ = true;
    GetLocalRootArkUINode().SetSwiperVertical(1);
    return true;
  } else if (propKey == "pageMargin") {
    prevMargin_ = nextMargin_ = HRValueUtils::GetFloat(propValue);
    GetLocalRootArkUINode().SetSwiperPrevMargin(prevMargin_);
    GetLocalRootArkUINode().SetSwiperNextMargin(nextMargin_);
    return true;
  }
  return BaseView::SetProp(propKey, propValue);
}

void PagerView::OnChildInserted(std::shared_ptr<BaseView> const &childView, int32_t index) {
  BaseView::OnChildInserted(childView, index);
  swiperNode_.InsertChild(childView->GetLocalRootArkUINode(), index);
}

void PagerView::OnChildRemoved(std::shared_ptr<BaseView> const &childView, int32_t index) {
  BaseView::OnChildRemoved(childView, index);
  swiperNode_.RemoveChild(childView->GetLocalRootArkUINode());
}

void PagerView::OnChange(const int32_t &index) {
  HippyValueObjectType selectedPayload = {{PAGE_ITEM_POSITION, HippyValue{index}}};
  std::shared_ptr<HippyValue> selectedParams = std::make_shared<HippyValue>(selectedPayload);
  HREventUtils::SendComponentEvent(ctx_, tag_, HREventUtils::EVENT_PAGE_SELECTED, selectedParams);
  OnViewComponentEvent(HREventUtils::EVENT_PAGE_SELECTED, selectedPayload);

  HippyValueObjectType changedPayload = {{PAGE_SCROLL_STATE, HippyValue{SCROLL_STATE_IDLE}}};
  std::shared_ptr<HippyValue> changedParams = std::make_shared<HippyValue>(changedPayload);
  HREventUtils::SendComponentEvent(ctx_, tag_, HREventUtils::EVENT_PAGE_SCROLL_STATE_CHANGED,
                                   changedParams);
  OnViewComponentEvent(HREventUtils::EVENT_PAGE_SCROLL_STATE_CHANGED, changedPayload);
  
  index_ = index;
}

void PagerView::OnAnimationStart(const int32_t &currentIndex, const int32_t &targetIndex,
                                 const float_t &currentOffset, const float_t &targetOffset,
                                 const float_t &swipeVelocity) {
  FOOTSTONE_DLOG(INFO) << "PagerView::OnAnimationStart - From index: " << currentIndex
                       << ", To index: " << targetIndex << ", Current offset: " << currentOffset
                       << ", Target offset: " << targetOffset
                       << ", Swipe velocity: " << swipeVelocity;
}

void PagerView::OnAnimationEnd(const int32_t &currentIndex, const float_t &currentOffset) {
  FOOTSTONE_DLOG(INFO) << "PagerView::OnAnimationEnd - Index: " << currentIndex
                       << ", Final offset: " << currentOffset;
}

void PagerView::OnContentDidScroll(const int32_t currentIndex, const int32_t pageIndex, const float_t pageOffset) {
  // position: Position index of the target page.
  // offset: Value from [-1, 1] indicating the offset from the page at position.
  auto position = pageIndex;
  auto offset = pageOffset;

  // filter the illegal values
  if (offset < -1.f || offset > 1.f) {
    return;
  }
  
  if (pageIndex > currentIndex) {
    if (offset > 0.001 && offset < 0.999) {
      position = pageIndex;
      offset = 1.f - offset;
    } else {
      return;
    }
  } else if (pageIndex < currentIndex) {
    if (offset < -0.001 && offset > -0.999) {
      position = pageIndex;
      offset = - (1.f + offset);
    } else {
      return;
    }
  } else {
    // no need to handle current page params
    return;
  }
  
  // FOOTSTONE_DLOG(INFO) << "PagerView on scroll, position: " << position << ", offset: " << offset;
  
  HippyValueObjectType type = {{PAGE_ITEM_POSITION, HippyValue{position}},
                               {PAGE_ITEM_OFFSET, HippyValue{offset}}};
  std::shared_ptr<HippyValue> params = std::make_shared<HippyValue>(type);
  HREventUtils::SendComponentEvent(ctx_, tag_, HREventUtils::EVENT_PAGE_SCROLL, params);
  OnViewComponentEvent(HREventUtils::EVENT_PAGE_SCROLL, type);
}

void PagerView::SendScrollStateChangeEvent(const std::string &state) {
  HippyValueObjectType payload = {{PAGE_SCROLL_STATE, HippyValue{state}}};
  auto params = std::make_shared<HippyValue>(payload);
  HREventUtils::SendComponentEvent(ctx_, tag_, HREventUtils::EVENT_PAGE_SCROLL_STATE_CHANGED,
                                   params);
  OnViewComponentEvent(HREventUtils::EVENT_PAGE_SCROLL_STATE_CHANGED, payload);
}

void PagerView::OnNodeTouchEvent(const ArkUI_UIInputEvent *inputEvent) {
  int32_t touchAction = OH_ArkUI_UIInputEvent_GetAction(inputEvent);
  switch (touchAction) {
  case UI_TOUCH_EVENT_ACTION_CANCEL:
    // No specific action needed for cancel, logging suffices.
    break;
  case UI_TOUCH_EVENT_ACTION_DOWN:
    SendScrollStateChangeEvent(SCROLL_STATE_DRAGGING);
    break;
  case UI_TOUCH_EVENT_ACTION_MOVE:
    // If needed, handle move action here in the future.
    break;
  case UI_TOUCH_EVENT_ACTION_UP:
    SendScrollStateChangeEvent(SCROLL_STATE_SETTLING);
    break;
  default:
    FOOTSTONE_DLOG(INFO) << "Unknown touch action received: " << touchAction;
    break;
  }
}

void PagerView::Call(const std::string &method, const std::vector<HippyValue> params,
                     std::function<void(const HippyValue &result)> callback) {
  if (method == "setPage") {
    if (params.empty()) {
      return;
    }
    index_ = HRValueUtils::GetInt32(params[0]);
    GetLocalRootArkUINode().SetSwiperSwipeToIndex(index_, 1);
  } else if (method == "setPageWithoutAnimation") {
    if (params.empty()) {
      return;
    }
    index_ = HRValueUtils::GetInt32(params[0]);
    GetLocalRootArkUINode().SetSwiperSwipeToIndex(index_, 0);
  } else if (method == "next") {
    int32_t total = static_cast<int32_t>(GetLocalRootArkUINode().GetTotalChildCount());
    if (total < 1) {
      return;
    }
    if (index_ < total - 1) {
      GetLocalRootArkUINode().SetSwiperSwipeToIndex(++index_, 1);
    }
  } else if (method == "prev") {
    if (index_ > 0) {
      GetLocalRootArkUINode().SetSwiperSwipeToIndex(--index_, 1);
    }
  } else if (method == "setIndex") {
    HippyValueObjectType map;
    if (params.empty() || !params[0].IsObject()) {
      FOOTSTONE_DLOG(INFO) << "Unknown params";
      return;
    }
    bool r = params[0].ToObject(map);
    if (r && map.size() > 0) {
      index_ = HRValueUtils::GetInt32(map["index"]);
      int32_t animated = HRValueUtils::GetBool(map["animated"], 0);
      GetLocalRootArkUINode().SetSwiperSwipeToIndex(index_, animated);
    }
  } else {
    BaseView::Call(method, params, callback);
  }
}

void PagerView::UpdateRenderViewFrame(const HRRect &frame, const HRPadding &padding) {
//   HRRect rc{frame.x, frame.y, frame.width, 200};
  if (frame.height > 600 && frame.height < 610) {
    FOOTSTONE_LOG(INFO) << "xxx hippy, pager - update frame";
  }
  if (tag_ == 86) {
    FOOTSTONE_LOG(INFO) << "xxx hippy, pager 86 - update frame";
  }
  BaseView::UpdateRenderViewFrame(frame, padding);
}

} // namespace native
} // namespace render
} // namespace hippy
