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

#include <string>
#include <map>
#include <set>
#include <unordered_map>
#include <arkui/native_node.h>
#include <arkui/native_type.h>
#include <arkui/styled_string.h>
#include <native_drawing/drawing_color.h>
#include <native_drawing/drawing_font_collection.h>
#include <native_drawing/drawing_text_declaration.h>
#include <native_drawing/drawing_types.h>
#include <native_drawing/drawing_text_typography.h>
#include <native_drawing/drawing_register_font.h>

// Note: Do not open normally, it impacts performance.
// #define MEASURE_TEXT_CHECK_PROP
// #define MEASURE_TEXT_LOG_RESULT

namespace hippy {
inline namespace render {
inline namespace native {

struct OhImageSpanHolder {
  double width;
  double height;
  OH_Drawing_PlaceholderVerticalAlignment alignment;
  double top = 0;

  double marginTop = 0;
  double marginBottom = 0;
};

struct OhImageSpanPos {
  double x;
  double y;
};

struct OhMeasureResult {
  double width;
  double height;
  std::vector<OhImageSpanPos> spanPos; // 指定imageSpan的位置
  bool isEllipsized;
};

class TextMeasurer {
public:
  TextMeasurer() {}
  TextMeasurer(const std::unordered_map<std::string, std::string>& fontFamilyList) : fontFamilyList_(fontFamilyList) {}
  ~TextMeasurer() {
    Destroy();
  }
  
  void StartMeasure(std::map<std::string, std::string> &propMap, const std::set<std::string> &fontFamilyNames);
  void AddText(std::map<std::string, std::string> &propMap, float density);
  void AddImage(std::map<std::string, std::string> &propMap, float density);
  OhMeasureResult EndMeasure(int width, int widthMode, int height, int heightMode, float density);
  
  void Destroy();
  
  ArkUI_StyledString *GetStyledString() {
    return styled_string_;
  }
  
private:
#ifdef MEASURE_TEXT_CHECK_PROP
  void StartCollectProp();
  void CheckUnusedProp(const char *tag, std::map<std::string, std::string> &propMap);
  std::vector<std::string> usedProp_;
#endif
#ifdef MEASURE_TEXT_LOG_RESULT
  std::string logTextContent_;
#endif
  
private:
  OH_Drawing_FontWeight FontWeightToDrawing(std::string &str);
  bool GetPropValue(std::map<std::string, std::string> &propMap, const char *prop, std::string &propValue);
  double CalcSpanPostion(OH_Drawing_Typography *typography, OhMeasureResult &ret);
  
  std::unordered_map<std::string, std::string> fontFamilyList_;
  OH_Drawing_FontCollection *fontCollection_;
  
  OH_Drawing_TypographyStyle *typographyStyle_;
  OH_Drawing_Typography *typography_ = nullptr;
  ArkUI_StyledString *styled_string_ = nullptr;
  
  std::vector<OhImageSpanHolder> imageSpans_;
  double lineHeight_ = 0; // 外部指定的行高，最高优先级
  double minLineHeight_ = 0; // 子组件中只有ImageSpan，没有TextSpan时，Placeholder不能撑大高度，使用ImageSpan的高度
  double paddingTop_ = 0;
  double paddingBottom_ = 0;
  double paddingLeft_ = 0;
  double paddingRight_ = 0;
};

} // namespace native
} // namespace render
} // namespace hippy
