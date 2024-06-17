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

class WaterFlowNodeDelegate {
public:
  virtual ~WaterFlowNodeDelegate() = default;
  virtual void onScrollIndex(int32_t firstIndex, int32_t lastIndex) {}
  virtual void OnReachEnd() {}
};
class WaterFlowNode : public ArkUINode {
protected:
  WaterFlowNodeDelegate *WaterFlowNodeDelegate_ = nullptr;
public:
  WaterFlowNode();
  ~WaterFlowNode();

  void AddChild(ArkUINode &child);
  void InsertChild(ArkUINode &child, int32_t index);
  void RemoveChild(ArkUINode &child);
  void RemoveAllChildren();
  HRPoint GetScrollOffset();
  void SetScrollEdgeEffect(bool hasEffect);
  void ScrollTo(float offsetX, float offsetY, bool animated);
  void ScrollToIndex(int32_t index, bool animated, bool isScrollAlignStart);
  void SetColumnsTemplate(std::string columnsTemplate);
  void SetNodeDelegate(WaterFlowNodeDelegate *waterFlowNodeDelegate);
  // void OnNodeEvent(ArkUI_NodeEvent *event);
  // void OnTouchEvent(ArkUI_UIInputEvent *event);
};

} // namespace native
} // namespace render
} // namespace hippy
