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

#include "renderer/arkui/row_node.h"
#include "renderer/components/base_view.h"
#include "renderer/arkui/stack_node.h"
#include "renderer/arkui/column_node.h"
#include "renderer/arkui/water_flow_node.h"
#include "renderer/arkui/refresh_node.h"
#include "renderer/arkui/scroll_node.h"
#include "renderer/arkui/list_node.h"
#include "renderer/arkui/water_flow_item_node.h"
#include "renderer/components/pull_footer_view.h"
#include "renderer/components/pull_header_view.h"

namespace hippy {
inline namespace render {
inline namespace native {

class WaterfallView : public BaseView ,public WaterFlowNodeDelegate,public RefreshNodeDelegate,public FlowItemNodeDelegate{
public:
  WaterfallView(std::shared_ptr<NativeRenderContext> &ctx);
  ~WaterfallView();

  //baseview override
  ArkUINode &GetLocalRootArkUINode() override;
  void Init() override;      
  bool SetProp(const std::string &propKey, const HippyValue &propValue) override;
  void OnSetPropsEnd() override;
  void OnChildInserted(std::shared_ptr<BaseView> const &childView, int32_t index) override;
  void OnChildRemoved(std::shared_ptr<BaseView> const &childView, int32_t index) override;
  void UpdateRenderViewFrame(const HRRect &frame, const HRPadding &padding) override;
  void Call(const std::string &method, const std::vector<HippyValue> params,
              std::function<void(const HippyValue &result)> callback) override;
    
  //WaterFlowNodeDelegate override
  void OnWaterFlowScrollIndex(int32_t firstIndex, int32_t lastIndex) override;
  void OnWaterFlowDidScroll(float_t offset, ArkUI_ScrollState state) override;
  void OnWaterFlowWillScroll(float_t offset, ArkUI_ScrollState state, int32_t source) override;
  void OnTouch(int32_t actionType) override;
  void OnScrollStart() override;
  void OnScrollStop() override;    
  void OnReachStart() override;       
  void OnReachEnd() override ;
    
  //RefreshNodeDelegate override
  void OnRefreshing() override;
  void OnStateChange(int32_t state) override;   

  //ArkUINodeDelegate override
  void OnAppear() override;
  void OnDisappear() override;  
    
  //FlowItemNodeDelegate
  void OnItemVisibleAreaChange(int32_t index, bool isVisible, float currentRatio) override ;
  void OnHeadRefreshFinish(int32_t delay);
  void OnHeadRefresh();
private:

  void HandleOnChildrenUpdated();
  void SendOnReachedEvent();

  void build();  
  ColumnNode colNode_;
  RefreshNode refreshNode_;
  RowNode rowNode_;  
  ScrollNode scrollNode_;
  ColumnNode colInnerNode_;
    
  StackNode stackNode_;  
  WaterFlowNode flowNode_;
  StackNode bannerNode_;  

  ArkUI_EdgeEffect edgeEffect_ = ArkUI_EdgeEffect::ARKUI_EDGE_EFFECT_SPRING;  
  HRPadding padding_ = {0, 0, 0, 0};
  float_t scrollEventThrottle_ = 30.0;
  int32_t preloadItemNumber_ = 0;
  float_t interItemSpacing_ = 0;
  float_t columnSpacing_ = 0;
  std::string columnsTemplate_ = "1fr 1fr";
    
  uint64_t cbID;  
  bool isRefreshing = false;
  std::shared_ptr<PullHeaderView> headerView = nullptr;
  std::shared_ptr<BaseView> bannerView = nullptr;
  std::shared_ptr<PullFooterView> footerView = nullptr;
  float width_ = 0;
  float height_ = 0;
  bool scrollEnable_ = false;
  float headerViewHeight_ = 0;  
  int32_t lastIndex_ = 0;  
};

} // namespace native
} // namespace render
} // namespace hippy
