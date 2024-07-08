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

#include "renderer/arkui/text_node.h"
#include "renderer/arkui/native_node_api.h"

namespace hippy {

inline namespace render {
inline namespace native {

TextNode::TextNode() : ArkUINode(NativeNodeApi::GetInstance()->createNode(ArkUI_NodeType::ARKUI_NODE_TEXT)) {
}

TextNode::~TextNode() {}

void TextNode::InsertChild(ArkUINode &child, int32_t index) {
  MaybeThrow(
    NativeNodeApi::GetInstance()->insertChildAt(nodeHandle_, child.GetArkUINodeHandle(), static_cast<int32_t>(index)));
}

void TextNode::RemoveChild(ArkUINode &child) {
  MaybeThrow(NativeNodeApi::GetInstance()->removeChild(nodeHandle_, child.GetArkUINodeHandle()));
}

TextNode &TextNode::SetTextContent(const std::string &text) {
  ArkUI_AttributeItem item = {.string = text.c_str()};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_TEXT_CONTENT, &item));
  return *this;
}

TextNode &TextNode::SetFontColor(uint32_t color) {
  uint32_t colorValue = color;
  if (colorValue >> 24 == 0) {
    colorValue |= ((uint32_t)0xff << 24);
  }
  ArkUI_NumberValue value[] = {{.u32 = colorValue}};
  ArkUI_AttributeItem item = {.value = value, .size = 1};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_FONT_COLOR, &item));
  return *this;
}

TextNode &TextNode::ResetFontColor() {
  MaybeThrow(NativeNodeApi::GetInstance()->resetAttribute(nodeHandle_, NODE_FONT_COLOR));
  return *this;
}

TextNode &TextNode::SetFontSize(float fontSize) {
  ArkUI_NumberValue value[] = {{.f32 = fontSize}};
  ArkUI_AttributeItem item = {.value = value, .size = 1};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_FONT_SIZE, &item));
  return *this;
}

TextNode &TextNode::SetFontStyle(int32_t fontStyle) {
  ArkUI_NumberValue value[] = {{.i32 = fontStyle}};
  ArkUI_AttributeItem item = {.value = value, .size = 1};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_FONT_STYLE, &item));
  return *this;
}

TextNode &TextNode::SetFontWeight(ArkUI_FontWeight fontWeight) {
  ArkUI_NumberValue value[] = {{.i32 = fontWeight}};
  ArkUI_AttributeItem item = {.value = value, .size = 1};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_FONT_WEIGHT, &item));
  return *this;
}

TextNode &TextNode::SetTextLineHeight(float textLineHeight) {
  ArkUI_NumberValue value[] = {{.f32 = textLineHeight}};
  ArkUI_AttributeItem item = {.value = value, .size = 1};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_TEXT_LINE_HEIGHT, &item));
  return *this;
}

TextNode &TextNode::SetTextDecoration(ArkUI_TextDecorationType decorationType, uint32_t decorationColor, ArkUI_TextDecorationStyle decorationStyle) {
  ArkUI_NumberValue value[] = {{.i32 = decorationType}, {.u32 = decorationColor}, {.i32 = decorationStyle}};
  ArkUI_AttributeItem item = {.value = value, .size = sizeof(value) / sizeof(ArkUI_NumberValue)};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_TEXT_DECORATION, &item));
  return *this;
}

TextNode &TextNode::SetTextCase(int32_t textCase) {
  ArkUI_NumberValue value[] = {{.i32 = textCase}};
  ArkUI_AttributeItem item = {.value = value, .size = 1};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_TEXT_CASE, &item));
  return *this;
}

TextNode &TextNode::SetTextLetterSpacing(float textLetterSpacing) {
  ArkUI_NumberValue value[] = {{.f32 = textLetterSpacing}};
  ArkUI_AttributeItem item = {.value = value, .size = 1};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_TEXT_LETTER_SPACING, &item));
  return *this;
}

TextNode &TextNode::SetTextMaxLines(int32_t textMaxLines) {
  ArkUI_NumberValue value[] = {{.i32 = textMaxLines}};
  ArkUI_AttributeItem item = {.value = value, .size = 1};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_TEXT_MAX_LINES, &item));
  return *this;
}

TextNode &TextNode::ResetTextMaxLines() {
  MaybeThrow(NativeNodeApi::GetInstance()->resetAttribute(nodeHandle_, NODE_TEXT_MAX_LINES));
  return *this;
}

TextNode &TextNode::SetTextAlign(ArkUI_TextAlignment align) {
  ArkUI_NumberValue value[] = {{.i32 = align}};
  ArkUI_AttributeItem item = {.value = value, .size = 1};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_TEXT_ALIGN, &item));
  return *this;
}

TextNode &TextNode::SetTextEllipsisMode(ArkUI_EllipsisMode ellipsisMode) {
  ArkUI_NumberValue value[] = {{.i32 = ellipsisMode}};
  ArkUI_AttributeItem item = {.value = value, .size = 1};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_TEXT_ELLIPSIS_MODE, &item));
  return *this;
}

TextNode &TextNode::SetTextOverflow(ArkUI_TextOverflow textOverflow) {
  ArkUI_NumberValue value[] = {{.i32 = textOverflow}};
  ArkUI_AttributeItem item = {.value = value, .size = 1};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_TEXT_OVERFLOW, &item));
  return *this;
}

TextNode &TextNode::SetWordBreak(ArkUI_WordBreak workBreak) {
  ArkUI_NumberValue value[] = {{.i32 = workBreak}};
  ArkUI_AttributeItem item = {.value = value, .size = 1};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_TEXT_WORD_BREAK, &item));
  return *this;
}

TextNode &TextNode::SetFontFamily(const std::string &fontFamily) {
  ArkUI_AttributeItem item = {.string = fontFamily.c_str()};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_FONT_FAMILY, &item));
  return *this;
}

TextNode &TextNode::SetTextCopyOption(int32_t textCopyOption) {
  if (!initFlag_[FLAG_COPYOPTION] || textCopyOption != textCopyOption_) {
    FOOTSTONE_DLOG(INFO) << "TextNode SetTextCopyOption flag = " << initFlag_[FLAG_COPYOPTION];
    ArkUI_NumberValue value[] = {{.i32 = textCopyOption}};
    ArkUI_AttributeItem item = {.value = value, .size = 1};
    MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_TEXT_COPY_OPTION, &item));
    initFlag_[FLAG_COPYOPTION] = true;
    textCopyOption_ = textCopyOption;
  }
  return *this;
}

TextNode &TextNode::SetTextBaselineOffset(float textBaselineOffset) {
  ArkUI_NumberValue value[] = {{.f32 = textBaselineOffset}};
  ArkUI_AttributeItem item = {.value = value, .size = 1};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_TEXT_BASELINE_OFFSET, &item));
  return *this;
}

TextNode &TextNode::SetTextShadow(float textShadowRadius, ArkUI_ShadowType textShadowType, uint32_t textShadowColor,
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

TextNode &TextNode::SetMinFontSize(float minFontSize) {
  if (!initFlag_[FLAG_MINFONTSIZE] || minFontSize != minFontSize_) {
    FOOTSTONE_DLOG(INFO) << "TextNode SetMinFontSize flag = " << initFlag_[FLAG_MINFONTSIZE];
    ArkUI_NumberValue value[] = {{.f32 = minFontSize}};
    ArkUI_AttributeItem item = {.value = value, .size = 1};
    MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_TEXT_MIN_FONT_SIZE, &item));
    initFlag_[FLAG_MINFONTSIZE] = true;
    minFontSize_ = minFontSize;
  }
  return *this;
}

TextNode &TextNode::SetMaxFontSize(float maxFontSize) {
  if (!initFlag_[FLAG_MAXFONTSIZE] || maxFontSize != maxFontSize_) {
    FOOTSTONE_DLOG(INFO) << "TextNode SetMaxFontSize flag = " << initFlag_[FLAG_MAXFONTSIZE];
    ArkUI_NumberValue value[] = {{.f32 = maxFontSize}};
    ArkUI_AttributeItem item = {.value = value, .size = 1};
    MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_TEXT_MAX_FONT_SIZE, &item));
    initFlag_[FLAG_MAXFONTSIZE] = true;
    maxFontSize_ = maxFontSize;
  }
  return *this;
}

TextNode &TextNode::SetTextFont(float fontSize, int32_t fontWeight /*= ARKUI_FONT_WEIGHT_NORMAL*/,
                                int32_t fontStyle /*= ARKUI_FONT_STYLE_NORMAL*/,
                                const std::string &fontFamily /*= std::string()*/) {
  ArkUI_NumberValue value[] = {{.f32 = fontSize}, {.i32 = fontWeight}, {.i32 = fontStyle}};
  ArkUI_AttributeItem item = {.value = value, .size = sizeof(value) / sizeof(ArkUI_NumberValue)};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_TEXT_FONT, &item));
  return *this;
}

TextNode &TextNode::SetTextHeightAdaptivePolicy(int32_t policyType) {
  ArkUI_NumberValue value[] = {{.i32 = policyType}};
  ArkUI_AttributeItem item = {.value = value, .size = 1};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_TEXT_HEIGHT_ADAPTIVE_POLICY, &item));
  return *this;
}

TextNode &TextNode::SetTextIndent(float textIndent) {
  ArkUI_NumberValue value[] = {{.f32 = textIndent}};
  ArkUI_AttributeItem item = {.value = value, .size = 1};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_TEXT_INDENT, &item));
  return *this;
}

TextNode &TextNode::SetTextEnable(bool enableFlag) {
  if (!initFlag_[FLAG_ENABLE] || enableFlag != enableFlag_) {
    FOOTSTONE_DLOG(INFO) << "TextNode SetTextEnable flag = " << initFlag_[FLAG_ENABLE];
    ArkUI_NumberValue value[] = {{.i32 = enableFlag}};
    ArkUI_AttributeItem item = {value, 1, nullptr, nullptr};
    MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_ENABLED, &item));
    initFlag_[FLAG_ENABLE] = true;
    enableFlag_ = enableFlag;
  }
  return *this;
}

TextNode &TextNode::SetPadding(float top, float right, float bottom, float left) {
  if (!initFlag_[FLAG_PADDING] || !(top == top_ && right == right_ && bottom == bottom_ && left == left_)) {
//    FOOTSTONE_DLOG(INFO) << "TextNode SetPadding flag = " << initFlag_[FLAG_PADDING];
    ArkUI_NumberValue value[] = {{.f32 = top}, {.f32 = right}, {.f32 = bottom}, {.f32 = left}};
    ArkUI_AttributeItem item = {.value = value, .size = 4};
    MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_PADDING, &item));
    initFlag_[FLAG_PADDING] = true;
    top_ = top;
    right_ = right;
    bottom_ = bottom;
    left_ = left;
  }
  return *this;
}

} // namespace native
} // namespace render
} // namespace hippy
