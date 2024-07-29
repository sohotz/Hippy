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

#include "renderer/components/refresh_wrapper_view.h"
#include "renderer/components/list_view.h"
#include "renderer/utils/hr_event_utils.h"
#include "renderer/utils/hr_value_utils.h"

namespace hippy {
inline namespace render {
inline namespace native {

RefreshWrapperView::RefreshWrapperView(std::shared_ptr<NativeRenderContext> &ctx) : BaseView(ctx) {
  listNode_.SetScrollBarDisplayMode(ARKUI_SCROLL_BAR_DISPLAY_MODE_OFF);
  listNode_.SetNodeDelegate(this);
}

RefreshWrapperView::~RefreshWrapperView() {
  ctx_->GetNativeRender().lock()->RemoveEndBatchCallback(ctx_->GetRootId(), end_batch_callback_id_);
  if (!children_.empty()) {
    for (const auto &itemNode : listItemNodes_) {
      listNode_.RemoveChild(*itemNode);
    }
    listItemNodes_.clear();
    children_.clear();
  }
}

void RefreshWrapperView::Init() {
  BaseView::Init();
  auto weak_view = weak_from_this();
  end_batch_callback_id_ = ctx_->GetNativeRender().lock()->AddEndBatchCallback(ctx_->GetRootId(), [weak_view]() {
    auto view = weak_view.lock();
    if (view) {
      auto refreshView = std::static_pointer_cast<RefreshWrapperView>(view);

    }
  });
}

ListNode &RefreshWrapperView::GetLocalRootArkUINode() { return listNode_; }

bool RefreshWrapperView::SetProp(const std::string &propKey, const HippyValue &propValue) {
  if (propKey == "bounceTime") {
    bounceTime_ = HRValueUtils::GetInt32(propValue);
    return true;
  } else if (propKey == "onScrollEnable") {
    scrollEventEnable_ = HRValueUtils::GetBool(propValue, false);
    return true;
  } else if (propKey == "scrollEventThrottle") {
    scrollEventThrottle_ = HRValueUtils::GetInt32(propValue, 30);
    return true;
  }
  return BaseView::SetProp(propKey, propValue);
}

void RefreshWrapperView::Call(const std::string &method, const std::vector<HippyValue> params,
                              std::function<void(const HippyValue &result)> callback) {
  FOOTSTONE_DLOG(INFO) << "RefreshWrapperView call: method " << method << ", params: " << params.size();
  if (method == "refreshComplected") {
    RefreshComplected();
  } else if (method == "startRefresh") {
    StartRefresh();
  } else {
    BaseView::Call(method, params, callback);
  }
}

void RefreshWrapperView::OnChildInserted(std::shared_ptr<BaseView> const &childView, int32_t index) {
  BaseView::OnChildInserted(childView, index);
  
  if (index == 0 && childView->GetViewType() == "RefreshWrapperItemView") {
    listNode_.SetListInitialIndex(1);
  }
  
  if (index == 1 && childView->GetViewType() == "ListView") {
    auto listView = std::static_pointer_cast<ListView>(childView);
    listView->SetScrollNestedMode(ARKUI_SCROLL_NESTED_MODE_SELF_FIRST, ARKUI_SCROLL_NESTED_MODE_PARENT_FIRST);
  }
  
  auto itemNode = std::make_shared<ListItemNode>();
  listItemNodes_.insert(listItemNodes_.begin() + index, itemNode);
  
  itemNode->AddChild(childView->GetLocalRootArkUINode());
  listNode_.InsertChild(*itemNode, index);
}

void RefreshWrapperView::OnChildRemoved(std::shared_ptr<BaseView> const &childView, int32_t index) {
  BaseView::OnChildRemoved(childView, index);
  
  auto itemNode = listItemNodes_[(uint32_t)index];
  listNode_.RemoveChild(*itemNode);
  
  listItemNodes_.erase(listItemNodes_.begin() + index);
}

void RefreshWrapperView::OnScrollIndex(int32_t firstIndex, int32_t lastIndex, int32_t centerIndex) {
  FOOTSTONE_DLOG(INFO) << "RefreshWrapperView on scroll index, start: " << firstIndex;
  if (firstIndex == 0) {
    refreshBarVisible_ = true;
  }
}

void RefreshWrapperView::OnWillScroll(float offset, ArkUI_ScrollState state) {

}

void RefreshWrapperView::OnScroll(float scrollOffsetX, float scrollOffsetY) {
  SendOnScrollEvent(-scrollOffsetY);
}

void RefreshWrapperView::OnScrollStart() {
  
}

void RefreshWrapperView::OnScrollStop() {
  // FOOTSTONE_DLOG(INFO) << "RefreshWrapperView on scroll stop, needRefresh: " << needRefresh_ << ", visible: " << refreshBarVisible_;
  if (needRefresh_) {
    StartRefresh();
  } else if (refreshBarVisible_) {
    BounceToHead();
  }
}

void RefreshWrapperView::OnReachStart() {
  FOOTSTONE_DLOG(INFO) << "RefreshWrapperView on reach start";
  needRefresh_ = true;
}

void RefreshWrapperView::OnReachEnd() {

}

void RefreshWrapperView::OnTouch(int32_t actionType) {

}

void RefreshWrapperView::BounceToHead() {
  listNode_.ScrollToIndex(1, true, true);
  refreshBarVisible_ = false;
  
  // TODO(hot): setTimeout bounceTime
}

void RefreshWrapperView::StartRefresh() {
  listNode_.ScrollToIndex(0, true, true);
  HREventUtils::SendComponentEvent(ctx_, tag_, HREventUtils::EVENT_REFRESH_WRAPPER_REFRESH, nullptr);
}

void RefreshWrapperView::RefreshComplected() {
  BounceToHead();
  needRefresh_ = false;
}

void RefreshWrapperView::SendOnScrollEvent(float y) {
  if (scrollEventEnable_) {
    auto currTime = GetTimeMilliSeconds();
    if (currTime - lastScrollEventTimeStamp_ < scrollEventThrottle_) {
      return;
    }
    HippyValueObjectType contentOffset;
    contentOffset["x"] = HippyValue(0);
    contentOffset["y"] = HippyValue(y);
    HippyValueObjectType event;
    event["contentOffset"] = contentOffset;
    HREventUtils::SendComponentEvent(ctx_, tag_, HREventUtils::EVENT_REFRESH_WRAPPER_SCROLL, std::make_shared<HippyValue>(contentOffset));
    lastScrollEventTimeStamp_ = currTime;
  }
}

} // namespace native
} // namespace render
} // namespace hippy
