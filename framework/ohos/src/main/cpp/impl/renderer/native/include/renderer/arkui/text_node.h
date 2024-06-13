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

class TextNodeDelegate {
public:
  virtual ~TextNodeDelegate() = default;
  virtual void OnClick() {}
};

class TextNode : public ArkUINode {
private:
  enum { FLAG_PADDING = 0, FLAG_MINFONTSIZE, FLAG_MAXFONTSIZE, FLAG_COPYOPTION, FLAG_ENABLE, FLAG_MAX };
  bool initFlag_[FLAG_MAX] = {0};

  float minFontSize_ = 0.0;
  float maxFontSize_ = 0.0;
  int32_t textCopyOption_ = 0;
  bool enableFlag_ = false;
  float top_ = 0.0;
  float right_ = 0.0;
  float bottom_ = 0.0;
  float left_ = 0.0;

protected:
  TextNodeDelegate *textNodeDelegate_;
  
public:
  TextNode();
  ~TextNode() override;

  void InsertChild(ArkUINode &child, int32_t index);
  void RemoveChild(ArkUINode &child);
  void OnNodeEvent(ArkUI_NodeEvent *event) override;
  void SetTextNodeDelegate(TextNodeDelegate *textNodeDelegate);

  TextNode &SetTextContent(const std::string &text);
  TextNode &SetFontColor(uint32_t fontColor);
  TextNode &ResetFontColor();
  TextNode &SetFontSize(float fontSize);
  TextNode &SetFontStyle(int32_t fontStyle);
  TextNode &SetFontWeight(ArkUI_FontWeight fontWeight);
  TextNode &SetTextLineHeight(float textLineHeight);
  TextNode &SetTextDecoration(ArkUI_TextDecorationType decorationType, uint32_t decorationColor, ArkUI_TextDecorationStyle decorationStyle);
  TextNode &SetTextCase(int32_t textCase);
  TextNode &SetTextLetterSpacing(float textLetterSpacing);
  TextNode &SetTextMaxLines(int32_t textMaxLines);
  TextNode &ResetTextMaxLines();
  TextNode &SetTextAlign(ArkUI_TextAlignment align);
  TextNode &SetTextEllipsisMode(ArkUI_EllipsisMode ellipsisMode);
  TextNode &SetTextOverflow(ArkUI_TextOverflow textOverflow);
  TextNode &SetWordBreak(ArkUI_WordBreak workBreak);
  TextNode &SetFontFamily(const std::string &fontFamily);
  TextNode &SetTextCopyOption(int32_t testCopyOption);
  TextNode &SetTextBaselineOffset(float textBaselineOffset);
  TextNode &SetTextShadow(float textShadowRadius, ArkUI_ShadowType textShadowType, uint32_t textShadowColor,
                          float textShadowOffsetX, float textShadowOffsetY);
  TextNode &SetMinFontSize(float minFontSize);
  TextNode &SetMaxFontSize(float maxFontSize);
  TextNode &SetTextFont(float fontSize, int32_t fontWeight = ARKUI_FONT_WEIGHT_NORMAL,
                        int32_t fontStyle = ARKUI_FONT_STYLE_NORMAL, const std::string &fontFamily = std::string());
  TextNode &SetTextHeightAdaptivePolicy(int32_t policyType);
  TextNode &SetTextIndent(float textIndent);
  TextNode &SetTextEnable(bool enableFlag);
  TextNode &SetPadding(float top, float right, float bottom, float left);
};

} // namespace native
} // namespace render
} // namespace hippy
