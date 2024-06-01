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

#include "renderer/components/list_view.h"
#include "renderer/utils/hr_value_utils.h"

namespace hippy {
inline namespace render {
inline namespace native {

ListView::ListView(std::shared_ptr<NativeRenderContext> &ctx) : BaseView(ctx) {
  stackNode_.AddChild(listNode_);
  listNode_.SetNodeDelegate(this);
}

ListView::~ListView() {
  ctx_->GetNativeRender().lock()->RemoveEndBatchCallback(ctx_->GetRootId(), end_batch_callback_id_);
}

void ListView::Init() {
  auto weak_view = weak_from_this();
  end_batch_callback_id_ = ctx_->GetNativeRender().lock()->AddEndBatchCallback(ctx_->GetRootId(), [weak_view]() {
    auto view = weak_view.lock();
    if (view) {
      auto listView = std::static_pointer_cast<ListView>(view);
      listView->HandleOnChildrenUpdated();
    }
  });
}

StackNode &ListView::GetLocalRootArkUINode() { return stackNode_; }

bool ListView::SetProp(const std::string &propKey, const HippyValue &propValue) {
  if (propKey == "nestedScrollTopPriority") {
    return true;
  } else if (propKey == "horizontal") {
    auto value = HRValueUtils::GetBool(propValue, false);
    if (value) {
      
    }
    return true;
  } else if (propKey == "scrollEnabled") {
    return true;
  } else if (propKey == "initialContentOffset") {
    return true;
  } else if (propKey == "itemViewCacheSize") {
    return true;
  } else if (propKey == "scrollEventThrottle") {
    return true;
  } else if (propKey == "preloadItemNumber") {
    return true;
  } else if (propKey == "exposureEventEnabled") {
    return true;
  } else if (propKey == "rowShouldSticky") {
    return true;
  } else if (propKey == "bounces") {
    return true;
  } else if (propKey == "scrollbegindrag") {
    return true;
  } else if (propKey == "scrollenddrag") {
    return true;
  } else if (propKey == "momentumscrollbegin") {
    return true;
  } else if (propKey == "momentumscrollend") {
    return true;
  } else if (propKey == "scroll") {
    return true;
  }
  return BaseView::SetProp(propKey, propValue);
}

void ListView::Call(const std::string &method, const std::vector<HippyValue> params,
                    std::function<void(const HippyValue &result)> callback) {
  FOOTSTONE_DLOG(INFO) << "ListView call: method " << method << ", params: " << params.size();
  if (method == "scrollToIndex") {
    
  } else if (method == "scrollToContentOffset") {
    
  } else if (method == "scrollToTop") {
    
  }
}

void ListView::OnChildInserted(std::shared_ptr<BaseView> const &childView, int32_t index) {
  BaseView::OnChildInserted(childView, index);
}

void ListView::OnChildRemoved(std::shared_ptr<BaseView> const &childView) {
  BaseView::OnChildRemoved(childView);
}

void ListView::OnAppear() {
  
}

void ListView::OnDisappear() {
  
}

void ListView::OnScroll(float scrollOffsetX, float scrollOffsetY) {
  
}

void ListView::OnScrollStart() {
  
}

void ListView::OnScrollStop() {
  
}

void ListView::HandleOnChildrenUpdated() {
  for (uint32_t i = 0; i < children_.size(); i++) {
    listNode_.AddChild(children_[i]->GetLocalRootArkUINode());
  }
}

void ListView::EmitScrollEvent(const std::string &eventName) {
  
}

void ListView::CheckSendOnScrollEvent() {
  
}

void ListView::CheckSendReachEndEvent(int32_t lastIndex) {
  
}

bool ListView::IsReachEnd(int32_t lastIndex) {
  return false;
}

void ListView::SendOnReachedEvent() {
  
}

void ListView::CheckBeginDrag() {
  
}

void ListView::CheckEndDrag() {
  
}

void ListView::CheckPullOnItemVisibleAreaChange(int32_t index, bool isVisible, float currentRatio) {
  
}

void ListView::CheckPullOnScroll() {
  
}

void ListView::CheckStickyOnItemVisibleAreaChange(int32_t index, bool isVisible, float currentRatio) {
  
}

} // namespace native
} // namespace render
} // namespace hippy
