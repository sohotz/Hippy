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

#include "renderer/arkui/text_area_node.h"
#include "renderer/arkui/native_node_api.h"

namespace hippy {
inline namespace render {
inline namespace native {

static constexpr std::array<ArkUI_NodeEventType, 5> TEXT_AREA_NODE_EVENT_TYPES = {
  NODE_TEXT_AREA_ON_PASTE, NODE_TEXT_AREA_ON_CHANGE, NODE_ON_FOCUS, NODE_ON_BLUR,
  NODE_TEXT_AREA_ON_TEXT_SELECTION_CHANGE};

TextAreaNode::TextAreaNode()
    : TextInputBaseNode(ArkUI_NodeType::ARKUI_NODE_TEXT_AREA), textAreaNodeDelegate_(nullptr) {
  for (auto eventType : TEXT_AREA_NODE_EVENT_TYPES) {
    MaybeThrow(NativeNodeApi::GetInstance()->registerNodeEvent(nodeHandle_, eventType, 0, nullptr));
  }
}

TextAreaNode::~TextAreaNode() {
  for (auto eventType : TEXT_AREA_NODE_EVENT_TYPES) {
    NativeNodeApi::GetInstance()->unregisterNodeEvent(nodeHandle_, eventType);
  }
}

void TextAreaNode::OnNodeEvent(ArkUI_NodeEvent *event) {
  if (textAreaNodeDelegate_ == nullptr) {
    return;
  }

  auto eventType = OH_ArkUI_NodeEvent_GetEventType(event);
  auto nodeComponentEvent = OH_ArkUI_NodeEvent_GetNodeComponentEvent(event);
  if (eventType == ArkUI_NodeEventType::NODE_TEXT_AREA_ON_PASTE) {
    textAreaNodeDelegate_->OnPaste();
  } else if (eventType == ArkUI_NodeEventType::NODE_TEXT_AREA_ON_CHANGE) {
    auto stringEvent = OH_ArkUI_NodeEvent_GetStringAsyncEvent(event);
    std::string text = stringEvent->pStr;
    textAreaNodeDelegate_->OnChange(std::move(text));
  } else if (eventType == ArkUI_NodeEventType::NODE_ON_FOCUS) {
    textAreaNodeDelegate_->OnFocus();
  } else if (eventType == ArkUI_NodeEventType::NODE_ON_BLUR) {
    textAreaNodeDelegate_->OnBlur();
  } else if (eventType == ArkUI_NodeEventType::NODE_TEXT_AREA_ON_TEXT_SELECTION_CHANGE) {
    int32_t selectionLocation = nodeComponentEvent->data[0].i32;
    int32_t selectionLength = nodeComponentEvent->data[1].i32 - nodeComponentEvent->data[0].i32;
    textAreaNodeDelegate_->OnTextSelectionChange(selectionLocation, selectionLength);
  }
}

void TextAreaNode::SetTextInputNodeDelegate(TextAreaNodeDelegate *textAreaNodeDelegate) {
  textAreaNodeDelegate_ = textAreaNodeDelegate;
}

void TextAreaNode::SetTextContent(std::string const &textContent) {
  ArkUI_AttributeItem item = {.string = textContent.c_str()};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_TEXT_AREA_TEXT, &item));
}

void TextAreaNode::SetTextSelection(int32_t start, int32_t end) {
  std::array<ArkUI_NumberValue, 2> value = {{{.i32 = start}, {.i32 = end}}};
  ArkUI_AttributeItem item = {value.data(), value.size(), nullptr, nullptr};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_TEXT_AREA_TEXT_SELECTION, &item));
}

void TextAreaNode::SetCaretColor(uint32_t const &color) {
  uint32_t colorValue = color;
  ArkUI_NumberValue value = {.u32 = colorValue};
  ArkUI_AttributeItem item = {&value, sizeof(ArkUI_NumberValue), nullptr, nullptr};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_TEXT_AREA_CARET_COLOR, &item));
}

void TextAreaNode::SetMaxLength(int32_t const &maxLength) {
  ArkUI_NumberValue value = {.i32 = maxLength};
  ArkUI_AttributeItem item = {&value, sizeof(ArkUI_NumberValue), nullptr, nullptr};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_TEXT_AREA_MAX_LENGTH, &item));
}

void TextAreaNode::SetPlaceholder(std::string const &placeholder) {
  ArkUI_AttributeItem item = {.string = placeholder.c_str()};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_TEXT_AREA_PLACEHOLDER, &item));
}

void TextAreaNode::SetPlaceholderColor(uint32_t const &color) {
  uint32_t colorValue = color;
  ArkUI_NumberValue value = {.u32 = colorValue};
  ArkUI_AttributeItem item = {&value, sizeof(ArkUI_NumberValue), nullptr, nullptr};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_TEXT_AREA_PLACEHOLDER_COLOR, &item));
}

std::string TextAreaNode::GetTextContent() {
  auto item = NativeNodeApi::GetInstance()->getAttribute(nodeHandle_, NODE_TEXT_AREA_TEXT);
  return item->string;
}

HRPoint TextAreaNode::GetTextAreaOffset() const {
  auto value = NativeNodeApi::GetInstance()->getAttribute(nodeHandle_, NODE_OFFSET)->value;
  float x = static_cast<float>(value[0].i32);
  float y = static_cast<float>(value[1].i32);
  return HRPoint{x, y};
}

void TextAreaNode::SetInputType(int32_t const &keyboardType) {
  ArkUI_NumberValue value = {.i32 = keyboardType};
  ArkUI_AttributeItem item = {&value, sizeof(ArkUI_NumberValue), nullptr, nullptr};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_TEXT_AREA_TYPE, &item));
}

void TextAreaNode::DefaultSetPadding() {
  ArkUI_NumberValue value = {.f32 = 0.f};
  ArkUI_AttributeItem item = {&value, sizeof(ArkUI_NumberValue), nullptr, nullptr};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_PADDING, &item));
}

void TextAreaNode::SetTextEditing(bool const enable) {
  ArkUI_NumberValue value = {.i32 = enable ? 1 : 0};
  ArkUI_AttributeItem item = {&value, sizeof(ArkUI_NumberValue), nullptr, nullptr};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_TEXT_AREA_EDITING, &item));
}

void TextAreaNode::SetEnterKeyType(ArkUI_EnterKeyType const &returnKeyType) {
  ArkUI_NumberValue value = {.i32 = returnKeyType};
  ArkUI_AttributeItem item = {&value, sizeof(ArkUI_NumberValue), nullptr, nullptr};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_TEXT_AREA_ENTER_KEY_TYPE, &item));
}

HRRect TextAreaNode::GetTextContentRect(){
  auto item = NativeNodeApi::GetInstance()->getAttribute(nodeHandle_, NODE_TEXT_AREA_CONTENT_RECT);
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
