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

#include "oh_napi/oh_measure_text.h"
#include <map>
#include <arkui/native_node.h>
#include <arkui/native_type.h>
#include <arkui/styled_string.h>
#include <native_drawing/drawing_color.h>
#include <native_drawing/drawing_font_collection.h>
#include <native_drawing/drawing_text_declaration.h>
#include <native_drawing/drawing_types.h>
#include <native_drawing/drawing_text_typography.h>
#include <native_drawing/drawing_register_font.h>

namespace hippy {
inline namespace render {
inline namespace native {

class TextMeasurer {
public:
  TextMeasurer() {}
  ~TextMeasurer() {}
  
  void StartMeasure(std::map<std::string, std::string> &propMap, const std::set<std::string> &fontFamilyNames);
  void AddText(std::map<std::string, std::string> &propMap);
  void AddImage(std::map<std::string, std::string> &propMap);
  OhMeasureResult EndMeasure(int width, int widthMode, int height, int heightMode, float density);
  
private:
  std::shared_ptr<ArkUI_StyledString> styled_string_;
//   size_t m_attachmentCount = 0;
//   std::vector<size_t> m_fragmentLengths{};
//   facebook::react::Float m_maximumWidth =
//       std::numeric_limits<facebook::react::Float>::max();
//   SharedFontCollection m_fontCollection;
};

} // namespace native
} // namespace render
} // namespace hippy
