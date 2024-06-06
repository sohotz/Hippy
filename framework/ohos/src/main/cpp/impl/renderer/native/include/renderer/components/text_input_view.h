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

#include "renderer/components/base_view.h"
#include "renderer/arkui/stack_node.h"
#include "renderer/utils/hr_types.h"
#include "renderer/arkui/text_input_node.h"
#include "renderer/arkui/text_area_node.h"
#include <bits/alltypes.h>

namespace hippy {
inline namespace render {
inline namespace native {
using HippyValue = footstone::HippyValue;

class TextInputView : public BaseView, public TextInputNodeDelegate,public TextAreaNodeDelegate {
public:
  TextInputView(std::shared_ptr<NativeRenderContext> &ctx);
  ~TextInputView();

  StackNode &GetLocalRootArkUINode() override;
  TextInputBaseNode &GetTextNode();
  bool SetProp(const std::string &propKey, const HippyValue &propValue) override;
  void OnSetPropsEnd() override;
  void Call(const std::string &method, const std::vector<HippyValue> params,
                   std::function<void(const HippyValue &result)> callback) override;
  void UpdateRenderViewFrame(const HRRect &frame, const HRPadding &padding) override;
    
  void OnChange(std::string text) override;
  void OnBlur() override;
  void OnFocus() override;
  void OnSubmit() override;
  void OnPaste() override;
  void OnTextSelectionChange(int32_t location, int32_t length) override;

public:    
  void InitNode();
  void SetFontWeight(const HippyValue &propValue);
  void SetTextAlign(const HippyValue &propValue);
  void SetTextAlignVertical(const HippyValue &propValue);
  void SetKeyBoardType(const HippyValue &propValue);
  void SetEntryKeyType(const HippyValue &propValue);

  void SetText(const HippyValueArrayType &param);
  void FocusTextInput(const HippyValueArrayType &param);
  void BlurTextInput(const HippyValueArrayType &param);
  void HideInputMethod(const HippyValueArrayType &param);
  void OnEventEndEditing(ArkUI_EnterKeyType enterKeyType);
    
public:    
  uint32_t caretColor = 0x00000000;
  uint32_t color = 0x00000000;
  std::string fontFamily  = "HarmonyOS Sans";
  float_t fontSize = 18;
  uint32_t fontStyle = ArkUI_FontStyle::ARKUI_FONT_STYLE_NORMAL;
  uint32_t fontWeight = ArkUI_FontWeight::ARKUI_FONT_WEIGHT_NORMAL;
  uint32_t maxLength = 0x7FFFFFFF;
  bool multiline = false;
  uint32_t textAlign = ArkUI_TextAlignment::ARKUI_TEXT_ALIGNMENT_START;
  uint32_t textAlignVertical = ArkUI_Alignment::ARKUI_ALIGNMENT_CENTER;
  std::string value  = "";
  uint32_t keyboardType = ArkUI_TextInputType::ARKUI_TEXTINPUT_TYPE_NORMAL;
  uint32_t returnKeyType = ArkUI_EnterKeyType::ARKUI_ENTER_KEY_TYPE_DONE;
  std::string placeholder = "";
  uint32_t placeholderTextColor = 0x00000000;
  int32_t maxLines = 10000000;
    
private:
  StackNode stackNode_;
  TextInputNode textInputNode_;
  TextAreaNode textAreaNode_;
  HRPadding cssPadding = {0, 0, 0, 0};

  bool isListenChangeText = false;
  bool isListenSelectionChange = false;
  bool isListenEndEditing = false;
  bool isListenFocus = false;
  bool isListenBlur = false;
  bool isListenKeyboardWillShow = false;
  bool isListenKeyboardWillHide = false;
  bool isListenContentSizeChange = false;

  bool focus = false;
  float_t previousContentWidth = 0;
  float_t previousContentHeight = 0;
};

} // namespace native
} // namespace render
} // namespace hippy
