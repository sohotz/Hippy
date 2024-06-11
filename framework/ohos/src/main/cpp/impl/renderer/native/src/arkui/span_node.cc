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

#include "renderer/arkui/span_node.h"
#include "renderer/arkui/native_node_api.h"

namespace hippy {
inline namespace render {
inline namespace native {

SpanNode::SpanNode()
    : ArkUINode(NativeNodeApi::GetInstance()->createNode(ArkUI_NodeType::ARKUI_NODE_SPAN)) {
}

SpanNode::~SpanNode() {}

void SpanNode::SetSpanNodeDelegate(SpanNodeDelegate *spanNodeDelegate) { spanNodeDelegate_ = spanNodeDelegate; }

void SpanNode::OnNodeEvent(ArkUI_NodeEvent *event) {
  if (spanNodeDelegate_ == nullptr) {
    return;
  }
  
  if (OH_ArkUI_NodeEvent_GetEventType(event) == ArkUI_NodeEventType::NODE_ON_CLICK) {
    spanNodeDelegate_->OnClick();
  }
}

SpanNode &SpanNode::SetSpanContent(const std::string &text) {
  ArkUI_AttributeItem item = {.string = text.c_str()};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_SPAN_CONTENT, &item));
  return *this;
}

SpanNode &SpanNode::SetFontColor(uint32_t color) {
  uint32_t colorValue = color;
  if (colorValue >> 24 == 0) {
    colorValue |= ((uint32_t)0xff << 24);
  }
  ArkUI_NumberValue value[] = {{.u32 = colorValue}};
  ArkUI_AttributeItem item = {.value = value, .size = 1};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_FONT_COLOR, &item));
  return *this;
}

SpanNode &SpanNode::SetFontSize(float fontSize) {
  ArkUI_NumberValue value[] = {{.f32 = fontSize}};
  ArkUI_AttributeItem item = {.value = value, .size = 1};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_FONT_SIZE, &item));
  return *this;
}

SpanNode &SpanNode::SetFontStyle(int32_t fontStyle) {
  ArkUI_NumberValue value[] = {{.i32 = fontStyle}};
  ArkUI_AttributeItem item = {.value = value, .size = 1};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_FONT_STYLE, &item));
  return *this;
}

SpanNode &SpanNode::SetFontWeight(int32_t fontWeight) {
  ArkUI_NumberValue value[] = {{.i32 = fontWeight}};
  ArkUI_AttributeItem item = {.value = value, .size = 1};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_FONT_WEIGHT, &item));
  return *this;
}

SpanNode &SpanNode::SetTextLineHeight(float textLineHeight) {
  ArkUI_NumberValue value[] = {{.f32 = textLineHeight}};
  ArkUI_AttributeItem item = {.value = value, .size = 1};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_TEXT_LINE_HEIGHT, &item));
  return *this;
}

SpanNode &SpanNode::SetTextDecoration(int32_t decorationStyle, uint32_t decorationColor /*= 0xFFFF0000*/) {
  ArkUI_NumberValue value[] = {{.i32 = decorationStyle}, {.u32 = decorationColor}};
  ArkUI_AttributeItem item = {.value = value, .size = sizeof(value) / sizeof(ArkUI_NumberValue)};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_TEXT_DECORATION, &item));
  return *this;
}

SpanNode &SpanNode::SetTextCase(int32_t textCase) {
  ArkUI_NumberValue value[] = {{.i32 = textCase}};
  ArkUI_AttributeItem item = {.value = value, .size = 1};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_TEXT_CASE, &item));
  return *this;
}

SpanNode &SpanNode::SetTextLetterSpacing(float textLetterSpacing) {
  ArkUI_NumberValue value[] = {{.f32 = textLetterSpacing}};
  ArkUI_AttributeItem item = {.value = value, .size = 1};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_TEXT_LETTER_SPACING, &item));
  return *this;
}

SpanNode &SpanNode::SetFontFamily(const std::string &fontFamily) {
  ArkUI_AttributeItem item = {.string = fontFamily.c_str()};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_FONT_FAMILY, &item));
  return *this;
}

SpanNode &SpanNode::SetTextShadow(float textShadowRadius, int32_t textShadowType, uint32_t textShadowColor,
                                  float textShadowOffsetX, float textShadowOffsetY) {
  ArkUI_NumberValue value[] = {{.f32 = textShadowRadius},
                               {.i32 = textShadowType},
                               {.u32 = textShadowColor},
                               {.f32 = textShadowOffsetX},
                               {.f32 = textShadowOffsetY}};
  ArkUI_AttributeItem item = {.value = value, .size = sizeof(value) / sizeof(ArkUI_NumberValue)};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_TEXT_TEXT_SHADOW, &item));
  return *this;
}

SpanNode &SpanNode::SetBackgroundStyle(uint32_t color) {
  ArkUI_NumberValue value[] = {{.u32 = color}, {.f32 = 0}};
  ArkUI_AttributeItem item = {.value = value, .size = sizeof(value) / sizeof(ArkUI_NumberValue)};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_SPAN_TEXT_BACKGROUND_STYLE, &item));
  return *this;
}

} // namespace native
} // namespace render
} // namespace hippy
