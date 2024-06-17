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

#include "renderer/components/waterfall_view.h"
#include "renderer/components/waterfall_item_view.h"
#include "renderer/utils/hr_value_utils.h"
#include "renderer/dom_node/hr_node_props.h"
#include "renderer/utils/hr_event_utils.h"

namespace hippy {
inline namespace render {
inline namespace native {

WaterfallView::WaterfallView(std::shared_ptr<NativeRenderContext> &ctx) : BaseView(ctx),
  padding_(10.0f, 20.0f, 10.0f, 20.0f) {
  colNode_.AddChild(refreshNode_);
  refreshNode_.AddChild(scrollNode_);
  scrollNode_.AddChild(columNode_);
  scrollNode_.SetNodeDelegate(this);
  columNode_.AddChild(stackNode_);
  columNode_.AddChild(waterFlowNode_);
}

WaterfallView::~WaterfallView() {}

void WaterfallView::Init() {
  HandleOnChildrenUpdated();
}
ColumnNode &WaterfallView::GetLocalRootArkUINode() { return colNode_; }

bool WaterfallView::SetProp(const std::string &propKey, const HippyValue &propValue) {
  std::lock_guard<std::mutex> lock(mutex_);
  if (propKey == "bounces") {
    bool b = HRValueUtils::GetBool(propValue, true);
    waterFlowNode_.SetScrollEdgeEffect(b);
    return true;
  } else if (propKey == "contentInset") {
      HippyValueObjectType data;
      data["top"] = HippyValue(0);
      data["bottom"] = HippyValue(0);
      data["left"] = HippyValue(0);
      data["right"] = HippyValue(0);
      return true;
  } else if (propKey == "scrollEventThrottle") {
    scrollEventThrottle_ = HRValueUtils::GetInt32(propValue, 30);
    return true;
  } else if (propKey == "preloadItemNumber") {
    preloadItemNumber_ = HRValueUtils::GetInt32(propValue);
    return true;
  } else if (propKey == "interItemSpacing") {
    int spacing = HRValueUtils::GetInt32(propValue, 0);
    interItemSpacing_ = spacing;
    return true;
  } else if (propKey == "columnSpacing") {
    int columnSpace = HRValueUtils::GetInt32(propValue, 0);
    columnSpacing_ = columnSpace;
    return true;
  } else if (propKey == "numberOfColumns") {
    int columns = HRValueUtils::GetInt32(propValue, 2);
    std::string columnsTemplate = "1fr";
    auto spaceCount = static_cast<std::string::size_type>(columns - 1);
    columnsTemplate.append(spaceCount, ' ');
    columnsTemplate += "1fr";
    waterFlowNode_.SetColumnsTemplate(columnsTemplate);
    return true;
}
  return BaseView::SetProp(propKey, propValue);
}

void WaterfallView::Call(const std::string &method, const std::vector<HippyValue> params,
                    std::function<void(const HippyValue &result)> callback) {
  FOOTSTONE_DLOG(INFO) << "WaterfallView call: method " << method << ", params: " << params.size();
  if (method == "scrollToIndex") {
    int xIndex = HRValueUtils::GetInt32(params[0]);
    int yIndex = HRValueUtils::GetInt32(params[1]);
    bool animated = HRValueUtils::GetBool(params[2], false);
    waterFlowNode_.ScrollToIndex(xIndex, yIndex, animated);
  } else if (method == "scrollToContentOffset") {
    auto xOffset = HRValueUtils::GetFloat(params[0]);
    auto yOffset = HRValueUtils::GetFloat(params[1]);
    bool animated = HRValueUtils::GetBool(params[2], false);
    waterFlowNode_.ScrollTo(xOffset, yOffset, animated);
  } else if (method == "scrollToTop") {
    waterFlowNode_.ScrollToIndex(0, 0, true);
  }
}

void WaterfallView::OnChildInserted(std::shared_ptr<BaseView> const &childView, int32_t index) {
  BaseView::OnChildInserted(childView, index);
  colNode_.InsertChild(childView->GetLocalRootArkUINode(), index);
}

void WaterfallView::OnChildRemoved(std::shared_ptr<BaseView> const &childView) {
  BaseView::OnChildRemoved(childView);
  colNode_.RemoveChild(childView->GetLocalRootArkUINode());
}

void WaterfallView::HandleOnChildrenUpdated() {
  waterFlowNode_.RemoveAllChildren();
  for (uint32_t i = 0; i < children_.size(); i++) {
    waterFlowNode_.AddChild(children_[i]->GetLocalRootArkUINode());

    // auto waterFlowView = std::static_pointer_cast<WaterfallItemView>(children_[i]);
    // waterFlowView->GetLocalRootArkUINode().SetStackNodeDelegate(this);
    // waterFlowView->GetLocalRootArkUINode().SetItemIndex(static_cast<int32_t>(i));
  }
  auto childrenCount = children_.size();
  if (childrenCount > 0) {
    if (children_[0]->GetViewType() == PULL_HEADER_VIEW_TYPE) {
      headerView_ = std::static_pointer_cast<PullHeaderView>(children_[0]);
    }
    if (childrenCount > 1 && !dynamic_cast<WaterfallItemView*>(children_[1].get())) {
      bannerView_ = children_[1];
    } else if (!dynamic_cast<WaterfallItemView*>(children_[0].get())) {
                bannerView_ = children_[0];
    }
    if (children_[childrenCount - 1]->GetViewType() == PULL_FOOTER_VIEW_TYPE) {
      footerView_ = std::static_pointer_cast<PullFooterView>(children_[childrenCount - 1]);
    }
    if (bannerView_) {
      colNode_.SetPosition(HRPosition(0, 0));
    }
  }
}

void WaterfallView::EmitScrollEvent(const std::string& eventType) {
  if (!HREventUtils::CheckRegisteredEvent(headerView_->GetCtx(), headerView_->GetTag(), eventType)) {
    return;
  }
  HippyValueObjectType contentInsetObj; // 内边距
  contentInsetObj.insert_or_assign("top", 0);
  contentInsetObj.insert_or_assign("bottom", 0);
  contentInsetObj.insert_or_assign("left", 0);
  contentInsetObj.insert_or_assign("right", 0);

  auto offset = waterFlowNode_.GetScrollOffset();

  HippyValueObjectType contentOffsetObj; // 偏移量
  contentOffsetObj.insert_or_assign("x", offset.x);
  contentOffsetObj.insert_or_assign("y", offset.y);

  HippyValueObjectType contentSizeObj;
  HRSize layoutSize = waterFlowNode_.GetSize();
  HRSize size = children_.size() > 0 ? colNode_.GetSize() : layoutSize;
  contentSizeObj.insert_or_assign("width", size.width);
  contentSizeObj.insert_or_assign("height", size.height);

  HippyValueObjectType layoutMeasurementObj;
  layoutMeasurementObj.insert_or_assign("width", layoutSize.width);
  layoutMeasurementObj.insert_or_assign("height", layoutSize.height);


  HippyValueObjectType paramsObj;
  paramsObj.insert_or_assign("contentInset", contentInsetObj);
  paramsObj.insert_or_assign("contentOffset", contentOffsetObj);
  paramsObj.insert_or_assign("contentSize", contentSizeObj);
  paramsObj.insert_or_assign("layoutMeasurement", layoutMeasurementObj);

  std::shared_ptr<HippyValue> params = std::make_shared<HippyValue>(paramsObj);
  HREventUtils::SendComponentEvent(headerView_->GetCtx(), headerView_->GetTag(), eventType, params);
}

void WaterfallView::SendOnReachedEvent() {
  HREventUtils::SendComponentEvent(headerView_->GetCtx(), headerView_->GetTag(), HREventUtils::EVENT_RECYCLER_END_REACHED, nullptr);
  HREventUtils::SendComponentEvent(headerView_->GetCtx(), headerView_->GetTag(), HREventUtils::EVENT_RECYCLER_LOAD_MORE, nullptr);
}

void WaterfallView::onHeadRefreshFinish() {
  FOOTSTONE_DLOG(INFO) << "WaterfallView onHeadRefreshFinish: onHeadRefreshFinish=";
}
void WaterfallView::onStartRefresh() {
  isRefreshing_ = true;
  HREventUtils::SendComponentEvent(headerView_->GetCtx(), headerView_->GetTag(), "onRefreshStart", nullptr);
}

void WaterfallView::onEndRefresh() {
  isRefreshing_ = false; // 标记刷新结束
  HREventUtils::SendComponentEvent(headerView_->GetCtx(), headerView_->GetTag(), "onRefreshEnd", nullptr);
}

void WaterfallView::onStateChange(RefreshStatus refreshStatus) {
  if (headerView_ && refreshStatus == RefreshStatus::Drag) {
    HippyValueArrayType params;
    params.emplace_back(CONTENT_OFFSET, headerView_->GetHeight());
    HREventUtils::SendComponentEvent(headerView_->GetCtx(), headerView_->GetTag(),
                                    HREventUtils::EVENT_PULL_HEADER_PULLING, std::make_shared<HippyValue>(params));
  }
}

void WaterfallView::onRefreshing() {
  if (headerView_) {
    HREventUtils::SendComponentEvent(headerView_->GetCtx(), headerView_->GetTag(),
                                    HREventUtils::EVENT_PULL_HEADER_RELEASED, nullptr);
  }
}
void WaterfallView::onScrollIndex(int32_t firstIndex, int32_t lastIndex) {
  // 更新滚动逻辑，例如：
  CheckSendReachEndEvent(lastIndex);
}

//检查是否滚动到底部
void WaterfallView::CheckSendReachEndEvent(int32_t lastIndex) {
  bool isThisTimeReachEnd = IsReachEnd(lastIndex);
  if (!isLastTimeReachEnd_ && isThisTimeReachEnd) {
    SendOnReachedEvent();
  }
  isLastTimeReachEnd_ = isThisTimeReachEnd;
}

bool WaterfallView::IsReachEnd(int32_t lastIndex) {
  if (preloadItemNumber_ > 0 && lastIndex >= (static_cast<int32_t>(children_.size()) - preloadItemNumber_)) {
    return true;
  } else {
    return false;
  }
}

void WaterfallView::OnReachEnd() {
  SendOnReachedEvent();
}

void WaterfallView::OnScrollStart() {
  FOOTSTONE_DLOG(INFO) << "WaterfallView onScroll: onScrollStart=";
}

void WaterfallView::OnScroll(float xOffset, float yOffset) {
  int64_t now = GetTimeMilliSeconds();
  float minOffset = scrollNode_.GetScrollMinOffset();
  std::string scrollEventName = HREventUtils::EVENT_SCROLLER_ON_SCROLL;
  if (scrollNode_.GetAxis() == ARKUI_SCROLL_DIRECTION_VERTICAL) {
    if (minOffset > 0 && fabsf(yOffset - lastScrollOffset_) > minOffset) {
      lastScrollOffset_ = yOffset;
      EmitScrollEvent(scrollEventName);
      return;
    }
  } else {
    if (minOffset > 0 && fabsf(xOffset - lastScrollOffset_) > minOffset) {
      lastScrollOffset_ = xOffset;
      EmitScrollEvent(scrollEventName);
      return;
    }
  }
  float gap = static_cast<float>(now - lastScrollTime_);
  if (minOffset <= 0 && gap > scrollNode_.GetScrollEventThrottle()) {
    lastScrollTime_ = now;
    EmitScrollEvent(scrollEventName);
  }
}

void WaterfallView::OnScrollStop() {
  FOOTSTONE_DLOG(INFO) << "WaterfallView OnScrollStop: onScrollStop=";
  if (momentumScrollEndEventEnable_) {
    EmitScrollEvent(HREventUtils::EVENT_SCROLLER_MOMENTUM_END);
  }
  if (onScrollEventEnable_) {
    EmitScrollEvent(HREventUtils::EVENT_SCROLLER_ON_SCROLL);
  }
}

void WaterfallView::OnTouch(int32_t actionType) {
  FOOTSTONE_DLOG(INFO) << "WaterfallView OnTouch: OnTouch=";
}

} // namespace native
} // namespace render
} // namespace hippy
