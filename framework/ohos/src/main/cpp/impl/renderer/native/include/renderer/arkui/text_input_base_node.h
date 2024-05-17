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

enum class TextAlignment {
  Natural,
  Left,
  Center,
  Right,
  Justified
};

class TextInputBaseNode : public ArkUINode {
protected:
  TextInputBaseNode(ArkUI_NodeType nodeType);
  virtual ~TextInputBaseNode();
  
  // void SetCommonFontAttributes(TextAttributes const &textAttributes);

public:
  void SetPadding(float left, float top, float right, float bottom);
  void SetFocusable(bool const &focusable);
  void SetAutoFocus(bool autoFocus);
  void SetResponseRegion(HRPosition const &position, HRSize const &size);
  void SetFocusStatus(int32_t focus);
  void SetFontColor(uint32_t const &color);
  void SetTextAlign(std::optional<TextAlignment> const &textAlign);
  // virtual void SetFont(TextAttributes const &textAttributes) = 0;
  virtual void SetTextContent(std::string const &textContent) = 0;
  virtual void SetTextSelection(int32_t start, int32_t end) = 0;
  virtual void SetCaretColor(uint32_t const &color) = 0;
  virtual void SetMaxLength(int32_t maxLength) = 0;
  virtual void SetPlaceholder(std::string const &placeholder) = 0;
  virtual void SetPlaceholderColor(uint32_t const &color) = 0;
  virtual std::string GetTextContent() = 0;
};

} // namespace native
} // namespace render
} // namespace hippy
