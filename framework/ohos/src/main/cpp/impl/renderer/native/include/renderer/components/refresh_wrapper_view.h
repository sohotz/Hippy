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

#include "renderer/arkui/list_item_node.h"
#include "renderer/components/base_view.h"
#include "renderer/arkui/stack_node.h"
#include "renderer/arkui/list_node.h"

namespace hippy {
inline namespace render {
inline namespace native {

class RefreshWrapperView : public BaseView, public ListNodeDelegate {
public:
  RefreshWrapperView(std::shared_ptr<NativeRenderContext> &ctx);
  ~RefreshWrapperView();
  
  void Init() override;

  ListNode &GetLocalRootArkUINode() override;
  bool SetProp(const std::string &propKey, const HippyValue &propValue) override;
  void Call(const std::string &method, const std::vector<HippyValue> params,
                    std::function<void(const HippyValue &result)> callback) override;
  
  void OnChildInserted(std::shared_ptr<BaseView> const &childView, int32_t index) override;
  void OnChildRemoved(std::shared_ptr<BaseView> const &childView, int32_t index) override;
  
  void OnScrollIndex(int32_t firstIndex, int32_t lastIndex, int32_t centerIndex) override;
  void OnWillScroll(float offset, ArkUI_ScrollState state) override;
  void OnScroll(float scrollOffsetX, float scrollOffsetY) override;
  void OnScrollStart() override;
  void OnScrollStop() override;
  void OnReachStart() override;
  void OnReachEnd() override;
  void OnTouch(int32_t actionType) override;
  
private:
  void BounceToHead();
  void StartRefresh();
  void RefreshComplected();
  
  void SendOnScrollEvent(float y);
  
  ListNode listNode_;
  std::vector<std::shared_ptr<ListItemNode>> listItemNodes_;
  
  int32_t bounceTime_ = 300;
  bool needRefresh_ = false;
  bool refreshBarVisible_ = false;
  
  bool scrollEventEnable_ = true;
  int32_t scrollEventThrottle_ = 400;
  int64_t lastScrollEventTimeStamp_ = -1;
  
  uint64_t end_batch_callback_id_ = 0;
};

} // namespace native
} // namespace render
} // namespace hippy
