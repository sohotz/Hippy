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

#include "renderer/arkui/arkui_node.h"

namespace hippy {
inline namespace render {
inline namespace native {

class ScrollNodeDelegate {
public:
  virtual ~ScrollNodeDelegate() = default;
  virtual void OnScroll(float xOffset, float yOffset) {}
  virtual void OnScrollStart() {}
  virtual void OnScrollStop() {}
  virtual float OnScrollFrameBegin(float offset, int32_t scrollState) { return offset; }
  virtual void OnTouch(int32_t actionType) {}
};

class ScrollNode : public ArkUINode {
protected:
  ScrollNodeDelegate *scrollNodeDelegate_;
  bool showScrollIndicator_;
  bool pagingEnabled_;
  float friction_;
  bool scrollEnabled_;
  ArkUI_ScrollDirection axis_;
  float initialContentOffset_;
  float scrollEventThrottle_;
  float scrollMinOffset_;
  void ScrollToContentOffset(float contentOffset);

public:
  ScrollNode();
  ~ScrollNode();

  ScrollNode &SetShowScrollIndicator(bool showScrollIndicator);
  ScrollNode &SetPagingEnabled(bool pagingEnabled);
  ScrollNode &SetFlingEnabled(bool flingEnabled);
  ScrollNode &SetContentOffset4Reuse(HRPoint const &contentOffset4Reuse);
  ScrollNode &SetScrollEnabled(bool scrollEnabled);
  ScrollNode &SetHorizontal(bool horizontal);
  ScrollNode &SetInitialContentOffset(float initialContentOffset);
  ScrollNode &SetScrollEventThrottle(float scrollEventThrottle);
  ScrollNode &SetScrollMinOffset(float scrollMinOffset);
  
  void ScrollTo(float x, float y, bool animated, int32_t duration = 0);
  void OnNodeEvent(ArkUI_NodeEvent *event) override;
  void SetNodeDelegate(ScrollNodeDelegate *scrollNodeDelegate);
  HRPoint GetScrollContentOffset() const;
  ArkUI_ScrollDirection GetAxis() const;
  float GetScrollMinOffset() const;
  float GetScrollEventThrottle() const;
};

} // namespace native
} // namespace render
} // namespace hippy