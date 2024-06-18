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
  FOOTSTONE_DLOG(INFO) << "PagerView initialized.";
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
    initialPage_ = HRValueUtils::GetInt32(propValue);
    index_ = initialPage_;
    GetLocalRootArkUINode().SetSwiperIndex(index_);
    return true;
  } else if (propKey == "scrollEnabled") {
    bool enable;
    propValue.ToBoolean(enable);
    disableSwipe_ = !enable;
    GetLocalRootArkUINode().SetEnabled(enable);
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

void PagerView::OnChildRemoved(std::shared_ptr<BaseView> const &childView) {
  BaseView::OnChildRemoved(childView);
  swiperNode_.RemoveChild(childView->GetLocalRootArkUINode());
}

void PagerView::OnChange(const int32_t &index) {
  HippyValueObjectType selectedPayload = {{"position", HippyValue{index}}};
  std::shared_ptr<HippyValue> selectedParams = std::make_shared<HippyValue>(selectedPayload);
  HREventUtils::SendComponentEvent(ctx_, tag_, HREventUtils::EVENT_PAGE_SELECTED, selectedParams);

  HippyValueObjectType changedPayload = {{"pageScrollState", HippyValue{"idle"}}};
  std::shared_ptr<HippyValue> changedParams = std::make_shared<HippyValue>(changedPayload);
  HREventUtils::SendComponentEvent(ctx_, tag_, HREventUtils::EVENT_PAGE_SCROLL_STATE_CHANGED,
                                   changedParams);
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

void PagerView::OnAnimationEnd(const int32_t &currentIndex, const float_t &finalOffset) {
  FOOTSTONE_DLOG(INFO) << "PagerView::OnAnimationEnd - Index: " << currentIndex
                       << ", Final offset: " << finalOffset;
}

void PagerView::OnContentDidScroll(const int32_t &swiperPageIndex, const int32_t &windowPageIndex,
                                   const float_t &pageMoveRatio, const float_t &pageAxisSize) {
  FOOTSTONE_DLOG(INFO) << "PagerView::OnContentDidScroll - SwiperPageIndex: " << swiperPageIndex
                       << ", WindowPageIndex: " << windowPageIndex
                       << ", Move ratio: " << pageMoveRatio << ", Page size: " << pageAxisSize;
}

void PagerView::OnGestureSwipe(const int32_t &swiperPageIndex,
                               const float_t &elementOffsetFromStart) {
  HippyValueObjectType type = {{"position", HippyValue{swiperPageIndex}},
                               {"offset", HippyValue{elementOffsetFromStart}}};
  std::shared_ptr<HippyValue> params = std::make_shared<HippyValue>(type);
  HREventUtils::SendComponentEvent(ctx_, tag_, HREventUtils::EVENT_PAGE_SCROLL, params);
}

void PagerView::OnTouchIntercept(const int32_t &eventEnum) {
    //No specific actions required
}

void PagerView::SendScrollStateChangeEvent(const std::string &state) {
  HippyValueObjectType payload = {{"pageScrollState", HippyValue{state}}};
  auto params = std::make_shared<HippyValue>(payload);
  HREventUtils::SendComponentEvent(ctx_, tag_, HREventUtils::EVENT_PAGE_SCROLL_STATE_CHANGED,
                                   params);
}

void PagerView::OnNodeTouchEvent(const ArkUI_UIInputEvent *inputEvent) {
  int32_t touchAction = OH_ArkUI_UIInputEvent_GetAction(inputEvent);
  switch (touchAction) {
  case UI_TOUCH_EVENT_ACTION_CANCEL:
    // No specific action needed for cancel, logging suffices.
    break;
  case UI_TOUCH_EVENT_ACTION_DOWN:
    SendScrollStateChangeEvent("dragging");
    break;
  case UI_TOUCH_EVENT_ACTION_MOVE:
    // If needed, handle move action here in the future.
    break;
  case UI_TOUCH_EVENT_ACTION_UP:
    SendScrollStateChangeEvent("settling");
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
    GetLocalRootArkUINode().SetSwiperIndex(index_);
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
    FOOTSTONE_DLOG(INFO) << "Unknown function name: " << method;
  }
}
} // namespace native
} // namespace render
} // namespace hippy
