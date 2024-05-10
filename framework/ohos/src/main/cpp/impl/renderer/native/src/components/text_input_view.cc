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

#include "renderer/components/text_input_view.h"
#include "renderer/utils/hr_value_utils.h"

namespace hippy {
inline namespace render {
inline namespace native {

TextInputView::TextInputView(std::shared_ptr<NativeRenderContext> &ctx) : BaseView(ctx) {}

TextInputView::~TextInputView() {}

StackNode &TextInputView::GetLocalRootArkUINode() { return stackNode_; }

bool TextInputView::SetProp(const std::string &propKey, HippyValue &propValue) {
  if (propKey == "caret-color") {
    return true;
  } else if (propKey == "color") {
    return true;
  } else if (propKey == "defaultValue") {
    return true;
  } else if (propKey == "fontFamily") {
    return true;
  } else if (propKey == "fontSize") {
    return true;
  } else if (propKey == "fontStyle") {
    return true;
  } else if (propKey == "fontWeight") {
    return true;
  } else if (propKey == "maxLength") {
    return true;
  } else if (propKey == "multiline") {
    return true;
  } else if (propKey == "textAlign") {
    return true;
  } else if (propKey == "textAlignVertical") {
    return true;
  } else if (propKey == "placeholder") {
    return true;
  } else if (propKey == "placeholderTextColor") {
    return true;
  } else if (propKey == "numberOfLines") {
    return true;
  } else if (propKey == "keyboardType") {
    return true;
  } else if (propKey == "returnKeyType") {
    return true;
  } else if (propKey == "value") {
    return true;
  } else if (propKey == "changetext") {
    return true;
  } else if (propKey == "selectionchange") {
    return true;
  } else if (propKey == "endediting") {
    return true;
  } else if (propKey == "focus") {
    return true;
  } else if (propKey == "blur") {
    return true;
  } else if (propKey == "keyboardwillshow") {
    return true;
  } else if (propKey == "keyboardwillhide") {
    return true;
  } else if (propKey == "contentSizeChange") {
    return true;
  }
  return BaseView::SetProp(propKey, propValue);
}

} // namespace native
} // namespace render
} // namespace hippy
