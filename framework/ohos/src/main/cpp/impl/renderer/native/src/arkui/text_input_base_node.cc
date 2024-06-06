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

#include "renderer/arkui/text_input_base_node.h"
#include "renderer/arkui/native_node_api.h"

namespace hippy {
inline namespace render {
inline namespace native {

TextInputBaseNode::TextInputBaseNode(ArkUI_NodeType nodeType)
    : ArkUINode(NativeNodeApi::GetInstance()->createNode(nodeType)) {}

TextInputBaseNode::~TextInputBaseNode() {}

void TextInputBaseNode::SetPadding(float left, float top, float right, float bottom) {
  ArkUI_NumberValue value[] = {{top}, {right}, {bottom}, {left}};
  ArkUI_AttributeItem item = {value, sizeof(value) / sizeof(ArkUI_NumberValue), nullptr, nullptr};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_PADDING, &item));
}

// void TextInputBaseNode::SetFocusable(bool const &focusable) {
//   int32_t focusableValue = 1;
//   if (!focusable) {
//     focusableValue = 0;
//   }
//   ArkUI_NumberValue value[] = {{.i32 = focusableValue}};
//   ArkUI_AttributeItem item = {value, sizeof(value) / sizeof(ArkUI_NumberValue), nullptr, nullptr};
//   MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_FOCUSABLE, &item));
// }

void TextInputBaseNode::SetAutoFocus(bool autoFocus) {
  ArkUI_NumberValue value = {.i32 = static_cast<int32_t>(autoFocus)};
  ArkUI_AttributeItem item = {&value, 1, nullptr, nullptr};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_FOCUS_STATUS, &item));
}

void TextInputBaseNode::SetResponseRegion(HRPosition const &position, HRSize const &size) {
  ArkUI_NumberValue value[] = {{0.0f}, {0.0f}, {size.width}, {size.height}};
  ArkUI_AttributeItem item = {value, sizeof(value) / sizeof(ArkUI_NumberValue), nullptr, nullptr};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_RESPONSE_REGION, &item));
}

void TextInputBaseNode::SetFontColor(uint32_t const &color) {
  uint32_t colorValue = color;
  ArkUI_NumberValue preparedColorValue[] = {{.u32 = colorValue}};
  ArkUI_AttributeItem colorItem = {preparedColorValue, sizeof(preparedColorValue) / sizeof(ArkUI_NumberValue), nullptr, nullptr};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_FONT_COLOR, &colorItem));
}

void TextInputBaseNode::SetTextAlign(ArkUI_TextAlignment textAlign){
  ArkUI_NumberValue value[] = {{.i32 = textAlign}};
  ArkUI_AttributeItem item = {.value = value, .size = 1};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_TEXT_ALIGN, &item));
}

void TextInputBaseNode::SetTextAlignVertical(ArkUI_Alignment alignment){
  ArkUI_NumberValue value[] = {{.i32 = alignment}};
  ArkUI_AttributeItem item = {.value = value, .size = 1};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_ALIGNMENT, &item));
}

void TextInputBaseNode::SetFontWeight(ArkUI_FontWeight weight){
  ArkUI_NumberValue value[] = {{.i32 = weight}};
  ArkUI_AttributeItem item = {.value = value, .size = 1};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_FONT_WEIGHT, &item));
}

void TextInputBaseNode::SetFontStyle(ArkUI_FontStyle style){
  ArkUI_NumberValue value[] = {{.i32 = style}};
  ArkUI_AttributeItem item = {.value = value, .size = 1};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_FONT_STYLE, &item));
}

void TextInputBaseNode::SetFontSize(float_t size){
  ArkUI_NumberValue value[] = {{.f32 = size}};
  ArkUI_AttributeItem item = {.value = value, .size = 1};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_FONT_SIZE, &item));
}

void TextInputBaseNode::SetFontFamily(std::string family){
  ArkUI_AttributeItem textItem = {.string = family.c_str()};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_FONT_FAMILY, &textItem));
}

void TextInputBaseNode::SetMaxLines(int32_t const lines){
  ArkUI_NumberValue value[] = {{.i32 = lines}};
  ArkUI_AttributeItem item = {.value = value, .size = 1};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_TEXT_MAX_LINES, &item));
}

} // namespace native
} // namespace render
} // namespace hippy
