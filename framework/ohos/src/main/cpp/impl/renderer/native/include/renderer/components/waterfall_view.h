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

#pragma once

#include "renderer/components/base_view.h"
#include "renderer/arkui/stack_node.h"
#include "renderer/arkui/column_node.h"
#include "renderer/arkui/water_flow_node.h"
#include "renderer/arkui/refresh_node.h"
#include "renderer/arkui/scroll_node.h"
#include "renderer/components/pull_header_view.h"
#include "renderer/components/pull_footer_view.h"

namespace hippy {
inline namespace render {
inline namespace native {

using HippyValue = footstone::HippyValue;
using HippyValueObjectType = footstone::value::HippyValue::HippyValueObjectType;
using HippyValueArrayType = footstone::value::HippyValue::HippyValueArrayType;

class WaterfallView : public BaseView, public WaterFlowNodeDelegate, public ScrollNodeDelegate, public RefreshNodeDelegate{
public:
  WaterfallView(std::shared_ptr<NativeRenderContext> &ctx);
  ~WaterfallView();
  void Init();
  ColumnNode &GetLocalRootArkUINode() override;
  bool SetProp(const std::string &propKey, const HippyValue &propValue) override;

  void OnChildInserted(std::shared_ptr<BaseView> const &childView, int32_t index) override;
  void OnChildRemoved(std::shared_ptr<BaseView> const &childView) override;
  void Call(const std::string &method, const std::vector<HippyValue> params,
              std::function<void(const HippyValue &result)> callback) override;

  void onScrollIndex(int32_t firstIndex, int32_t lastIndex) override;
  void OnReachEnd() override;
  void OnScroll(float xOffset, float yOffset) override;
  void OnScrollStart() override;
  void OnScrollStop() override;
  void OnTouch(int32_t actionType) override;
  void onHeadRefreshFinish() override;
  void onStartRefresh() override;
  void onEndRefresh() override;
  void onStateChange(RefreshStatus refreshStatus) override;
  void onRefreshing() override;

private:
  void HandleOnChildrenUpdated();
  void EmitScrollEvent(const std::string& eventType);
  void SendOnReachedEvent();
  void CheckSendReachEndEvent(int32_t lastIndex);
  bool IsReachEnd(int32_t lastIndex);

  constexpr static const char *CONTENT_OFFSET = "contentOffset";
  constexpr static const char *PULL_HEADER_VIEW_TYPE = "PullHeaderView";
  constexpr static const char *PULL_FOOTER_VIEW_TYPE = "PullFooterView";

  ColumnNode colNode_;
  ColumnNode columNode_;
  RefreshNode refreshNode_;
  WaterFlowNode waterFlowNode_;
  ScrollNode scrollNode_;
  StackNode stackNode_;

  HRPadding padding_ = HRPadding(10.0f, 20.0f, 10.0f, 20.0f);
  int32_t scrollEventThrottle_ = 30;
  int32_t preloadItemNumber_ = 0;
  int32_t interItemSpacing_ = 0;
  int32_t columnSpacing_ = 0;
  std::string columnsTemplate_ = "";

  std::shared_ptr<PullHeaderView> headerView_ = nullptr;
  std::shared_ptr<PullFooterView> footerView_ = nullptr;
  std::shared_ptr<BaseView> bannerView_ = nullptr;
  std::vector<std::shared_ptr<BaseView>> children_;

  std::mutex mutex_;

  bool isRefreshing_;
  int64_t lastScrollTime_ = 0;
  bool isLastTimeReachEnd_ = false;
  bool momentumScrollEndEventEnable_;
  bool onScrollEventEnable_;
  float lastScrollOffset_;
};

} // namespace native
} // namespace render
} // namespace hippy
