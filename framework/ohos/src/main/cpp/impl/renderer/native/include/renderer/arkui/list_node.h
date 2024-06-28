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
#include "renderer/arkui/arkui_node_registry.h"

namespace hippy {
inline namespace render {
inline namespace native {

class ListNodeDelegate {
public:
  virtual ~ListNodeDelegate() = default;
  virtual void OnScrollIndex(int32_t firstIndex, int32_t lastIndex, int32_t centerIndex) {}
  virtual void OnScroll(float scrollOffsetX, float scrollOffsetY) {}
  virtual void OnScrollStart() {}
  virtual void OnScrollStop() {}
  virtual void OnReachStart() {}
  virtual void OnReachEnd() {}
  virtual void OnTouch(int32_t actionType) {}
};

class ListNode : public ArkUINode {
protected:
  ListNodeDelegate *listNodeDelegate_ = nullptr;

public:
  ListNode();
  ~ListNode();

  void RemoveAllChildren();
  
  HRPoint GetScrollOffset();
  void ScrollTo(float offsetX, float offsetY, bool animated);
  void ScrollToIndex(int32_t index, bool animated, bool isScrollAlignStart);
  void SetListDirection(bool isVertical);
  void SetScrollEdgeEffect(bool hasEffect);
  void SetScrollNestedScroll(ArkUI_ScrollNestedMode scrollForward, ArkUI_ScrollNestedMode scrollBackward);
  void SetEnableScrollInteraction(bool enabled);
  void SetListCachedCount(int32_t count);
  void SetScrollBarDisplayMode(ArkUI_ScrollBarDisplayMode mode);

  void OnNodeEvent(ArkUI_NodeEvent *event) override;
  void SetNodeDelegate(ListNodeDelegate *listNodeDelegate);
};

} // namespace native
} // namespace render
} // namespace hippy
