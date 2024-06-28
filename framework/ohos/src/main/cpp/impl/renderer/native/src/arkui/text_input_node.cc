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

#include "renderer/arkui/text_input_node.h"
#include "renderer/arkui/native_node_api.h"

namespace hippy {
inline namespace render {
inline namespace native {

static constexpr std::array<ArkUI_NodeEventType, 6> TEXT_INPUT_NODE_EVENT_TYPES = {
  NODE_TEXT_INPUT_ON_PASTE,
  NODE_TEXT_INPUT_ON_CHANGE,
  NODE_TEXT_INPUT_ON_SUBMIT,
  NODE_ON_FOCUS,
  NODE_ON_BLUR,
  NODE_TEXT_INPUT_ON_TEXT_SELECTION_CHANGE};

TextInputNode::TextInputNode()
    : TextInputBaseNode(ArkUI_NodeType::ARKUI_NODE_TEXT_INPUT), textInputNodeDelegate_(nullptr) {
  for (auto eventType : TEXT_INPUT_NODE_EVENT_TYPES) {
    MaybeThrow(NativeNodeApi::GetInstance()->registerNodeEvent(nodeHandle_, eventType, 0, nullptr));
  }

  ArkUI_NumberValue value = {.i32 = 1};
  ArkUI_AttributeItem item = {&value, 1, nullptr, nullptr};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_TEXT_INPUT_ENABLE_KEYBOARD_ON_FOCUS, &item));
}

TextInputNode::~TextInputNode() {
  for (auto eventType : TEXT_INPUT_NODE_EVENT_TYPES) {
    NativeNodeApi::GetInstance()->unregisterNodeEvent(nodeHandle_, eventType);
  }
}

void TextInputNode::OnNodeEvent(ArkUI_NodeEvent *event) {
  ArkUINode::OnNodeEvent(event);
  if (textInputNodeDelegate_ == nullptr) {
    return;
  }

  auto eventType = OH_ArkUI_NodeEvent_GetEventType(event);
  auto nodeComponentEvent = OH_ArkUI_NodeEvent_GetNodeComponentEvent(event);
  if (eventType == ArkUI_NodeEventType::NODE_TEXT_INPUT_ON_PASTE) {
    textInputNodeDelegate_->OnPaste();
  } else if (eventType == ArkUI_NodeEventType::NODE_TEXT_INPUT_ON_CHANGE) {
    auto stringEvent = OH_ArkUI_NodeEvent_GetStringAsyncEvent(event);
    std::string text = stringEvent->pStr;
    textInputNodeDelegate_->OnChange(std::move(text));
  } else if (eventType == ArkUI_NodeEventType::NODE_TEXT_INPUT_ON_SUBMIT) {
    textInputNodeDelegate_->OnSubmit();
  } else if (eventType == ArkUI_NodeEventType::NODE_ON_FOCUS) {
    textInputNodeDelegate_->OnFocus();
  } else if (eventType == ArkUI_NodeEventType::NODE_ON_BLUR) {
    textInputNodeDelegate_->OnBlur();
  } else if (eventType == ArkUI_NodeEventType::NODE_TEXT_INPUT_ON_TEXT_SELECTION_CHANGE) {
    int32_t selectionLocation = nodeComponentEvent->data[0].i32;
    int32_t selectionLength = nodeComponentEvent->data[1].i32 - nodeComponentEvent->data[0].i32;
    textInputNodeDelegate_->OnTextSelectionChange(selectionLocation, selectionLength);
  }
}

void TextInputNode::SetTextInputNodeDelegate(TextInputNodeDelegate *textInputNodeDelegate) {
  textInputNodeDelegate_ = textInputNodeDelegate;
}

void TextInputNode::SetTextContent(std::string const &textContent) {
  ArkUI_AttributeItem item = {.string = textContent.c_str()};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_TEXT_INPUT_TEXT, &item));
}

void TextInputNode::SetTextSelection(int32_t start, int32_t end) {
  std::array<ArkUI_NumberValue, 2> value = {{{.i32 = start}, {.i32 = end}}};
  ArkUI_AttributeItem item = {value.data(), value.size(), nullptr, nullptr};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_TEXT_INPUT_TEXT_SELECTION, &item));
}

void TextInputNode::SetCaretColor(uint32_t const &color) {
  uint32_t colorValue = color;
  ArkUI_NumberValue value = {.u32 = colorValue};
  ArkUI_AttributeItem item = {&value, sizeof(ArkUI_NumberValue), nullptr, nullptr};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_TEXT_INPUT_CARET_COLOR, &item));
}

void TextInputNode::SetMaxLength(int32_t const &maxLength) {
  ArkUI_NumberValue value = {.i32 = maxLength};
  ArkUI_AttributeItem item = {&value, sizeof(ArkUI_NumberValue), nullptr, nullptr};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_TEXT_INPUT_MAX_LENGTH, &item));
}

void TextInputNode::SetPlaceholder(std::string const &placeholder) {
  ArkUI_AttributeItem item = {.string = placeholder.c_str()};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_TEXT_INPUT_PLACEHOLDER, &item));
}

void TextInputNode::SetPlaceholderColor(uint32_t const &color) {
  uint32_t colorValue = color;
  ArkUI_NumberValue value = {.u32 = colorValue};
  ArkUI_AttributeItem item = {&value, sizeof(ArkUI_NumberValue), nullptr, nullptr};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_TEXT_INPUT_PLACEHOLDER_COLOR, &item));
}

std::string TextInputNode::GetTextContent() {
  auto item = NativeNodeApi::GetInstance()->getAttribute(nodeHandle_, NODE_TEXT_INPUT_TEXT);
  return item->string;
}

HRPoint TextInputNode::GetTextInputOffset() const {
  auto value = NativeNodeApi::GetInstance()->getAttribute(nodeHandle_, NODE_OFFSET)->value;
  float x = static_cast<float >(value->i32);
  value++;
  float y = static_cast<float >(value->i32);
  return HRPoint{x, y};
}

void TextInputNode::SetCaretHidden(bool hidden) {
  if (hidden) {
    ArkUI_NumberValue value = {.f32 = 0};
    ArkUI_AttributeItem item = {&value, sizeof(ArkUI_NumberValue), nullptr, nullptr};
    MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_TEXT_INPUT_CARET_STYLE, &item));
  } else {
    MaybeThrow(NativeNodeApi::GetInstance()->resetAttribute(nodeHandle_, NODE_TEXT_INPUT_CARET_STYLE));
  }
}

void TextInputNode::SetInputType(int32_t const &keyboardType) {
  ArkUI_NumberValue value = {.i32 = keyboardType};
  ArkUI_AttributeItem item = {&value, sizeof(ArkUI_NumberValue), nullptr, nullptr};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_TEXT_INPUT_TYPE, &item));
}

void TextInputNode::SetSelectedBackgroundColor(uint32_t const &color) {
  ArkUI_NumberValue selectedBackgroundColor = {.u32 = color};
  ArkUI_AttributeItem colorItem = {&selectedBackgroundColor, sizeof(ArkUI_NumberValue), nullptr, nullptr};
  MaybeThrow(
    NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_TEXT_INPUT_SELECTED_BACKGROUND_COLOR, &colorItem));
}

void TextInputNode::SetPasswordIconVisibility(bool isVisible) {
  ArkUI_NumberValue value = {.i32 = isVisible ? 1 : 0};
  ArkUI_AttributeItem item = {&value, sizeof(ArkUI_NumberValue), nullptr, nullptr};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_TEXT_INPUT_SHOW_PASSWORD_ICON, &item));
}

void TextInputNode::SetEnterKeyType(ArkUI_EnterKeyType const &returnKeyType) {
  ArkUI_NumberValue value = {.i32 = returnKeyType};
  ArkUI_AttributeItem item = {&value, sizeof(ArkUI_NumberValue), nullptr, nullptr};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_TEXT_INPUT_ENTER_KEY_TYPE, &item));
}

void TextInputNode::SetCancelButtonMode(uint32_t mode) {
  int32_t cancelButtonStyle = static_cast<int32_t>(ARKUI_CANCELBUTTON_STYLE_INVISIBLE); // TODO(hot):
  ArkUI_NumberValue value[] = {{.i32 = cancelButtonStyle}};
  ArkUI_AttributeItem item = {value, sizeof(value) / sizeof(ArkUI_NumberValue), nullptr, nullptr};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_TEXT_INPUT_CANCEL_BUTTON, &item));
}

void TextInputNode::ResetSelectedBackgroundColor() {
  MaybeThrow(NativeNodeApi::GetInstance()->resetAttribute(nodeHandle_, NODE_TEXT_INPUT_SELECTED_BACKGROUND_COLOR));
}

void TextInputNode::SetTextEditing(bool const enable){
  ArkUI_NumberValue value = {.i32 = enable ? 1 : 0};
  ArkUI_AttributeItem item = {&value, sizeof(ArkUI_NumberValue), nullptr, nullptr};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_TEXT_INPUT_EDITING, &item));
}

HRRect TextInputNode::GetTextContentRect() {
  auto item = NativeNodeApi::GetInstance()->getAttribute(nodeHandle_, NODE_TEXT_INPUT_CONTENT_RECT);
  float x = static_cast<float>(item->value[0].f32);
  float y = static_cast<float>(item->value[1].f32);
  float w = static_cast<float>(item->value[2].f32);
  float h = static_cast<float>(item->value[3].f32);
  HRRect rect(x, y, w, h);
  return rect;
}

} // namespace native
} // namespace render
} // namespace hippy
