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

#include "renderer/arkui/arkui_node.h"
#include <algorithm>
#include "renderer/arkui/arkui_node_registry.h"
#include "renderer/arkui/native_node_api.h"
#include "renderer/utils/hr_convert_utils.h"

namespace hippy {
inline namespace render {
inline namespace native {

ArkUINode::ArkUINode(ArkUI_NodeHandle nodeHandle) : nodeHandle_(nodeHandle) {
  ArkUINodeRegistry::GetInstance().RegisterNode(this);
}

ArkUINode::~ArkUINode() {
  if (nodeHandle_ != nullptr) {
    ArkUINodeRegistry::GetInstance().UnregisterNode(this);
    NativeNodeApi::GetInstance()->disposeNode(nodeHandle_);
  }
}

ArkUINode::ArkUINode(ArkUINode &&other) noexcept : nodeHandle_(std::move(other.nodeHandle_)) {
  other.nodeHandle_ = nullptr;
}

ArkUINode &ArkUINode::operator=(ArkUINode &&other) noexcept {
  std::swap(nodeHandle_, other.nodeHandle_);
  return *this;
}

ArkUI_NodeHandle ArkUINode::GetArkUINodeHandle() { return nodeHandle_; }

void ArkUINode::OnNodeEvent(ArkUI_NodeEvent *event) {}

void ArkUINode::MarkDirty() {
  NativeNodeApi::GetInstance()->markDirty(GetArkUINodeHandle(), ArkUI_NodeDirtyFlag::NODE_NEED_RENDER);
  NativeNodeApi::GetInstance()->markDirty(GetArkUINodeHandle(), ArkUI_NodeDirtyFlag::NODE_NEED_LAYOUT);
  NativeNodeApi::GetInstance()->markDirty(GetArkUINodeHandle(), ArkUI_NodeDirtyFlag::NODE_NEED_MEASURE);
}

ArkUINode &ArkUINode::SetPosition(const HRPosition &position) {
  ArkUI_NumberValue value[] = {{position.x}, {position.y}};
  ArkUI_AttributeItem item = {value, sizeof(value) / sizeof(ArkUI_NumberValue), nullptr, nullptr};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_POSITION, &item));
  return *this;
}

ArkUINode &ArkUINode::SetSize(const HRSize &size) {
  ArkUI_NumberValue widthValue[] = {{size.width}};
  ArkUI_AttributeItem widthItem = {widthValue, sizeof(widthValue) / sizeof(ArkUI_NumberValue), nullptr, nullptr};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_WIDTH, &widthItem));

  ArkUI_NumberValue heightValue[] = {{size.height}};
  ArkUI_AttributeItem heightItem = {heightValue, sizeof(heightValue) / sizeof(ArkUI_NumberValue), nullptr, nullptr};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_HEIGHT, &heightItem));
  return *this;
}

ArkUINode &ArkUINode::SetBorderWidth(float top, float right, float bottom, float left) {
  top = std::max(top, 0.0f);
  right = std::max(right, 0.0f);
  bottom = std::max(bottom, 0.0f);
  left = std::max(left, 0.0f);
  ArkUI_NumberValue borderWidthValue[] = {{top}, {right}, {bottom}, {left}};
  ArkUI_AttributeItem borderWidthItem = {borderWidthValue, sizeof(borderWidthValue) / sizeof(ArkUI_NumberValue), nullptr, nullptr};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_BORDER_WIDTH, &borderWidthItem));
  return *this;
}

ArkUINode &ArkUINode::SetBorderColor(uint32_t top, uint32_t right, uint32_t bottom, uint32_t left) {
  uint32_t borderTopColor = 0xff000000;
  uint32_t bordeRightColor = 0xff000000;
  uint32_t borderBottomColor = 0xff000000;
  uint32_t borderLeftColor = 0xff000000;
  if (top) {
    borderTopColor = top;
  }
  if (right) {
    bordeRightColor = right;
  }
  if (bottom) {
    borderBottomColor = bottom;
  }
  if (left) {
    borderLeftColor = left;
  }
  ArkUI_NumberValue borderColorValue[] = {
    {.u32 = borderTopColor}, {.u32 = bordeRightColor}, {.u32 = borderBottomColor}, {.u32 = borderLeftColor}};
  ArkUI_AttributeItem borderColorItem = {borderColorValue, sizeof(borderColorValue) / sizeof(ArkUI_NumberValue), nullptr, nullptr};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_BORDER_COLOR, &borderColorItem));
  return *this;
}

ArkUINode &ArkUINode::SetBorderRadius(float topLeft, float topRight, float bottomLeft, float bottomRight) {
  ArkUI_NumberValue borderRadiusValue[] = {
    {topLeft}, {topRight},
    {bottomLeft}, {bottomRight}
  };

  ArkUI_AttributeItem borderRadiusItem = {borderRadiusValue, sizeof(borderRadiusValue) / sizeof(ArkUI_NumberValue), nullptr, nullptr};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_BORDER_RADIUS, &borderRadiusItem));
  return *this;
}

ArkUINode &ArkUINode::SetBorderStyle(std::string &top, std::string &right, std::string &bottom, std::string &left) {
  ArkUI_NumberValue borderStyleValue[] = {
    {.i32 = static_cast<int32_t>(HRConvertUtils::BorderStyleToArk(top))},
    {.i32 = static_cast<int32_t>(HRConvertUtils::BorderStyleToArk(right))},
    {.i32 = static_cast<int32_t>(HRConvertUtils::BorderStyleToArk(bottom))},
    {.i32 = static_cast<int32_t>(HRConvertUtils::BorderStyleToArk(left))}
  };
  ArkUI_AttributeItem borderStyleItem = {borderStyleValue, sizeof(borderStyleValue) / sizeof(ArkUI_NumberValue), nullptr, nullptr};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_BORDER_STYLE, &borderStyleItem));
  return *this;
}

ArkUINode &ArkUINode::SetBackgroundColor(uint32_t color) {
  ArkUI_NumberValue preparedColorValue[] = {{.u32 = color}};
  ArkUI_AttributeItem colorItem = {preparedColorValue, sizeof(preparedColorValue) / sizeof(ArkUI_NumberValue), nullptr, nullptr};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_BACKGROUND_COLOR, &colorItem));
  return *this;
}

} // namespace native
} // namespace render
} // namespace hippy
