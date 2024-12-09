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

#include "renderer/text_measure/text_measurer.h"
#include "footstone/logging.h"

namespace hippy {
inline namespace render {
inline namespace native {

#ifdef MEASURE_TEXT_CHECK_PROP
void TextMeasurer::StartCollectProp() { usedProp_.clear(); }

void TextMeasurer::CheckUnusedProp(const char *tag, std::map<std::string, std::string> &propMap) {
  for (auto it = propMap.begin(); it != propMap.end(); ++it) {
    if (std::find(usedProp_.begin(), usedProp_.end(), it->first) == usedProp_.end()) {
      FOOTSTONE_DLOG(WARNING) << "hippy text - measure " << tag << " unused prop: " << it->first << " : " << it->second;
    }
  }
}
#endif

OH_Drawing_FontWeight TextMeasurer::FontWeightToDrawing(std::string &str) {
  if (str.length() == 0 || str == "normal") {
    return FONT_WEIGHT_400;
  } else if (str == "bold") {
    return FONT_WEIGHT_700;
  } else {
    auto w = std::atoi(str.c_str());
    if (std::isnan(w) || w == 0) {
      return FONT_WEIGHT_400;
    }
    if (w < 200) {
      return FONT_WEIGHT_100;
    } else if (w < 300) {
      return FONT_WEIGHT_200;
    } else if (w < 400) {
      return FONT_WEIGHT_300;
    } else if (w < 500) {
      return FONT_WEIGHT_400;
    } else if (w < 600) {
      return FONT_WEIGHT_500;
    } else if (w < 700) {
      return FONT_WEIGHT_600;
    } else if (w < 800) {
      return FONT_WEIGHT_700;
    } else if (w < 900) {
      return FONT_WEIGHT_800;
    } else {
      return FONT_WEIGHT_900;
    }
  }
}

bool TextMeasurer::GetPropValue(std::map<std::string, std::string> &propMap, const char *prop, std::string &propValue) {
  auto it = propMap.find(prop);
  if (it == propMap.end()) {
    propValue.clear();
    return false;
  }
#ifdef MEASURE_TEXT_CHECK_PROP
  usedProp_.push_back(prop);
#endif
  propValue = it->second;
  return true;
}

void TextMeasurer::StartMeasure(std::map<std::string, std::string> &propMap, const std::set<std::string> &fontFamilyNames) {
  if (typographyStyle_) {
    return;
  }
  
#ifdef MEASURE_TEXT_CHECK_PROP
  StartCollectProp();
#endif
  typographyStyle_ = OH_Drawing_CreateTypographyStyle();
  OH_Drawing_SetTypographyTextDirection(typographyStyle_, TEXT_DIRECTION_LTR); // 从左向右排版
  
  std::string propValue;
  
  int textAlign = TEXT_ALIGN_START;
  if (GetPropValue(propMap, "textAlign", propValue)) {
    if (propValue == "center") {
      textAlign = TEXT_ALIGN_CENTER;
    } else if (propValue == "end") {
      textAlign = TEXT_ALIGN_END;
    } else if (propValue == "justify") {
      textAlign = TEXT_ALIGN_JUSTIFY;
    }
  }
  OH_Drawing_SetTypographyTextAlign(typographyStyle_, textAlign);

  int maxLines = 100000;
  if (GetPropValue(propMap, "numberOfLines", propValue) && propValue.size() > 0) {
    maxLines = std::stoi(propValue);
  }
  OH_Drawing_SetTypographyTextMaxLines(typographyStyle_, maxLines);

  if (GetPropValue(propMap, "breakStrategy", propValue)) {
    OH_Drawing_BreakStrategy bs = BREAK_STRATEGY_GREEDY;
    if (propValue == "high_quality") {
      bs = BREAK_STRATEGY_HIGH_QUALITY;
    } else if (propValue == "balanced") {
      bs = BREAK_STRATEGY_BALANCED;
    }
    OH_Drawing_SetTypographyTextBreakStrategy(typographyStyle_, bs);
  }

  if (GetPropValue(propMap, "ellipsizeMode", propValue)) {
    OH_Drawing_EllipsisModal em = ELLIPSIS_MODAL_TAIL;
    if (propValue == "head") {
      em = ELLIPSIS_MODAL_HEAD;
    } else if (propValue == "middle") {
      em = ELLIPSIS_MODAL_MIDDLE;
    }
    OH_Drawing_SetTypographyTextEllipsisModal(typographyStyle_, em);
  }

  fontCollection_ = OH_Drawing_CreateFontCollection();
  for (auto itName = fontFamilyNames.begin(); itName != fontFamilyNames.end(); itName++) {
    auto &fontFamilyName = *itName;
    auto itFont = fontFamilyList_.find(fontFamilyName);
    if (itFont != fontFamilyList_.end()) {
      auto font = itFont->second;
      uint32_t ret = OH_Drawing_RegisterFont(fontCollection_, fontFamilyName.c_str(), font.c_str());
      if (ret != 0) {
        FOOTSTONE_LOG(ERROR) << "Measure Text OH_Drawing_RegisterFont(" << fontFamilyName << "," << font << ") fail";
      }
    } else {
      FOOTSTONE_LOG(ERROR) << "Measure Text OH_Drawing_RegisterFont not found font:" << fontFamilyName;
    }
  }

  styled_string_ = OH_ArkUI_StyledString_Create(typographyStyle_, fontCollection_);
  
  if (GetPropValue(propMap, "lineHeight", propValue) && propValue.size() > 0) {
    lineHeight_ = std::stod(propValue);
  }
  if (GetPropValue(propMap, "paddingVertical", propValue) && propValue.size() > 0) {
    auto paddingValue = std::stod(propValue);
    paddingTop_ = paddingValue;
    paddingBottom_ = paddingValue;
  }
  if (GetPropValue(propMap, "paddingHorizontal", propValue) && propValue.size() > 0) {
    auto paddingValue = std::stod(propValue);
    paddingLeft_ = paddingValue;
    paddingRight_ = paddingValue;
  }

#ifdef MEASURE_TEXT_CHECK_PROP
  const static std::vector<std::string> dropProp = {
    "text",        "backgroundColor", "color",           "marginLeft",       "marginRight",
    "marginTop",   "marginBottom",    "textShadowColor", "textShadowOffset", "borderColor",
    "borderWidth", "breakStrategy",   "textShadowRadius"};
  for (uint32_t i = 0; i < dropProp.size(); i++) {
    usedProp_.push_back(dropProp[i]);
  }
  CheckUnusedProp("Start", propMap);
#endif
}

void TextMeasurer::AddText(std::map<std::string, std::string> &propMap, float density) {
#ifdef MEASURE_TEXT_CHECK_PROP
  StartCollectProp();
#endif

  OH_Drawing_TextStyle *txtStyle = OH_Drawing_CreateTextStyle();
  
  std::string propValue;
  
  if (GetPropValue(propMap, "lineHeight", propValue) && propValue.size() > 0) {
    lineHeight_ = std::stod(propValue);
  }
  
  if (GetPropValue(propMap, "color", propValue) && propValue.size() > 0) {
    unsigned long color = std::stoul(propValue);
    OH_Drawing_SetTextStyleColor(txtStyle, (uint32_t)color);
  }
  double fontSize = 14; // 默认的fontSize是14
  if (GetPropValue(propMap, "fontSize", propValue) && propValue.size() > 0) {
    fontSize = std::stod(propValue);
  }
  OH_Drawing_SetTextStyleFontSize(txtStyle, fontSize * density);
  if (GetPropValue(propMap, "fontWeight", propValue)) {
    int fontWeight = FontWeightToDrawing(propValue);
    OH_Drawing_SetTextStyleFontWeight(txtStyle, fontWeight);
  }

  OH_Drawing_SetTextStyleBaseLine(txtStyle, TEXT_BASELINE_ALPHABETIC);

  if (GetPropValue(propMap, "textDecorationLine", propValue)) {
    OH_Drawing_TextDecoration td = TEXT_DECORATION_NONE;
    if (propValue == "underline") {
      td = TEXT_DECORATION_UNDERLINE;
    } else if (propValue == "line-through") {
      td = TEXT_DECORATION_LINE_THROUGH;
    } else if (propValue == "overline") {
      td = TEXT_DECORATION_OVERLINE;
    }
    OH_Drawing_SetTextStyleDecoration(txtStyle, td);
  }
  if (GetPropValue(propMap, "textDecorationColor", propValue) && propValue.size() > 0) {
    unsigned long color = std::stoul(propValue);
    OH_Drawing_SetTextStyleDecorationColor(txtStyle, (uint32_t)color);
  }
  if (GetPropValue(propMap, "textDecorationStyle", propValue)) {
    OH_Drawing_TextDecorationStyle ds = TEXT_DECORATION_STYLE_SOLID;
    if (propValue == "dotted") {
      ds = TEXT_DECORATION_STYLE_DOTTED;
    } else if (propValue == "double") {
      ds = TEXT_DECORATION_STYLE_DOUBLE;
    } else if (propValue == "dashed") {
      ds = TEXT_DECORATION_STYLE_DASHED;
    } else if (propValue == "wavy") {
      ds = TEXT_DECORATION_STYLE_WAVY;
    }
    OH_Drawing_SetTextStyleDecorationStyle(txtStyle, ds);
  }

  // If font height is set, measure results for some special char will be wrong.
  // For example, ε (utf8 code: e0bdbdceb5). Measured height is less than drawn height.
  // OH_Drawing_SetTextStyleFontHeight(txtStyle, 1.25);

  if (GetPropValue(propMap, "fontFamily", propValue)) {
    const char *fontFamilies[] = {propValue.c_str()};
    OH_Drawing_SetTextStyleFontFamilies(txtStyle, 1, fontFamilies);
  }
  int fontStyle = FONT_STYLE_NORMAL;
  if (GetPropValue(propMap, "fontStyle", propValue) && propValue == "italic") {
    fontStyle = FONT_STYLE_ITALIC;
  }
  OH_Drawing_SetTextStyleFontStyle(txtStyle, fontStyle);
    
  // use default locale,
  // If en is set, measure results for Chinese characters will be inaccurate.
  // OH_Drawing_SetTextStyleLocale(txtStyle, "zh");
  
  if (GetPropValue(propMap, "letterSpacing", propValue) && propValue.size() > 0) {
    double letterSpacing = std::stod(propValue);
    OH_Drawing_SetTextStyleLetterSpacing(txtStyle, letterSpacing * density);
  }
  
  OH_ArkUI_StyledString_PushTextStyle(styled_string_, txtStyle);
  if (GetPropValue(propMap, "text", propValue)) {
    OH_ArkUI_StyledString_AddText(styled_string_, propValue.c_str());
#ifdef MEASURE_TEXT_LOG_RESULT
    logTextContent_ += "[span]";
    logTextContent_ += propValue;
#endif
  }

  OH_ArkUI_StyledString_PopTextStyle(styled_string_);
  OH_Drawing_DestroyTextStyle(txtStyle);

#ifdef MEASURE_TEXT_CHECK_PROP
  const static std::vector<std::string> dropProp = {
    "backgroundColor",  "lineHeight",        "margin",          "marginLeft",      "marginRight",
    "marginTop",        "marginBottom",      "textAlign",       "textShadowColor", "textShadowOffset",
    "ellipsizeMode",    "numberOfLines",     "borderColor",     "borderWidth",     "breakStrategy",
    "textShadowRadius", "paddingHorizontal", "paddingVertical", "verticalAlign"};
  for (uint32_t i = 0; i < dropProp.size(); i++) {
    usedProp_.push_back(dropProp[i]);
  }
  CheckUnusedProp("Text", propMap);
#endif
}

void TextMeasurer::AddImage(std::map<std::string, std::string> &propMap, float density) {
#ifdef MEASURE_TEXT_CHECK_PROP
  StartCollectProp();
#endif

  std::string propValue;
  
  OH_Drawing_PlaceholderSpan span;
  if (GetPropValue(propMap, "width", propValue) && propValue.size() > 0) {
      span.width = std::stod(propValue) * density;
  }
  if (GetPropValue(propMap, "height", propValue) && propValue.size() > 0) {
      span.height = std::stod(propValue) * density;
  }
  span.alignment = OH_Drawing_PlaceholderVerticalAlignment::ALIGNMENT_CENTER_OF_ROW_BOX;

  OH_ArkUI_StyledString_AddPlaceholder(styled_string_, &span);
  
  if (minLineHeight_ < span.height) {
    minLineHeight_ = span.height;
  }

  OhImageSpanHolder spanH;
  spanH.width = span.width;
  spanH.height = span.height;
  spanH.alignment = OH_Drawing_PlaceholderVerticalAlignment::ALIGNMENT_CENTER_OF_ROW_BOX;

  if (GetPropValue(propMap, "verticalAlign", propValue)) {
    if (propValue == "top") {
      spanH.alignment = OH_Drawing_PlaceholderVerticalAlignment::ALIGNMENT_TOP_OF_ROW_BOX;
    } else if (propValue == "middle") {
      spanH.alignment = OH_Drawing_PlaceholderVerticalAlignment::ALIGNMENT_CENTER_OF_ROW_BOX;
    } else if (propValue == "baseline") {
      spanH.alignment = OH_Drawing_PlaceholderVerticalAlignment::ALIGNMENT_OFFSET_AT_BASELINE;
    } else if (propValue == "bottom") {
      spanH.alignment = OH_Drawing_PlaceholderVerticalAlignment::ALIGNMENT_BOTTOM_OF_ROW_BOX;
    }
  }
  if (GetPropValue(propMap, "verticalAlignment", propValue)) {
    // TODO: check
    spanH.alignment = OH_Drawing_PlaceholderVerticalAlignment::ALIGNMENT_ABOVE_BASELINE;
  }
  if (GetPropValue(propMap, "margin", propValue) && propValue.size() > 0) {
    spanH.marginTop = std::stod(propValue);
    spanH.marginBottom = std::stod(propValue);
  }
  if (GetPropValue(propMap, "top", propValue) && propValue.size() > 0) {
    spanH.top = std::stod(propValue);
  }

  imageSpans_.push_back(spanH);
  
#ifdef MEASURE_TEXT_CHECK_PROP
  const static std::vector<std::string> dropProp = {"backgroundColor", "src", "tintColor"};
  for (uint32_t i = 0; i < dropProp.size(); i++) {
    usedProp_.push_back(dropProp[i]);
  }
  CheckUnusedProp("Image", propMap);
#endif
}

double TextMeasurer::CalcSpanPostion(OH_Drawing_Typography *typography, OhMeasureResult &ret) {
  double baseLine = 0;
  size_t lineCount = 0;
  std::vector<double> lineHeights;    // 真实每行高度
  std::vector<double> measureHeights; // 测得每行高度

  lineCount = OH_Drawing_TypographyGetLineCount(typography); // 总行数
  for (uint32_t i = 0; i < lineCount; i++) {                 // 获取每行行高
    // 当前行没有文本时，或者指定了lineHeight，baseLine获取的就不对
    // baseLine = OH_Drawing_TypographyGetAlphabeticBaseline(typography); //=h*11/16
    // baseLine = OH_Drawing_TypographyGetIdeographicBaseline(typography); //=h
    double h = OH_Drawing_TypographyGetLineHeight(typography, (int)i);
    measureHeights.push_back(h);
    if (lineHeight_ != 0) {
      lineHeights.push_back(lineHeight_);
    } else {
      lineHeights.push_back(h);
    }
  }

  OH_Drawing_TextBox *tb = OH_Drawing_TypographyGetRectsForPlaceholders(typography); // 获取每个imageSpan的区域
  size_t textBoxCount = OH_Drawing_GetSizeOfTextBox(tb); // 获取到的数量，应该和 imageSpans_ 一样多
  double bottom = lineHeights[0];
  for (uint32_t i = 0; i < textBoxCount; i++) { // i 对应到 imageSpans_ 下标
    float boxTop = OH_Drawing_GetTopFromTextBox(tb, (int)i);
    float boxBottom = OH_Drawing_GetBottomFromTextBox(tb, (int)i);
    float boxLeft = OH_Drawing_GetLeftFromTextBox(tb, (int)i);
    // float boxRight = OH_Drawing_GetRightFromTextBox(tb, (int)i);
    double top = 0;
    double measureTop = 0;
    OhImageSpanPos pos;
    pos.x = boxLeft;
    pos.y = boxTop;
    for (uint32_t j = 0; j < lineCount; j++) {
      bottom = top + lineHeights[j];
      double measureBottom = measureTop + measureHeights[j];
      if (measureTop <= boxTop && boxBottom <= measureBottom) { // 根据测得的top和bottom定位到span所在行
        baseLine = lineHeights[j] * 0.6;                      // todo 猜的比例
        switch (imageSpans_[i].alignment) {
        case OH_Drawing_PlaceholderVerticalAlignment::ALIGNMENT_TOP_OF_ROW_BOX:
          pos.y = top + imageSpans_[i].marginTop;
          break;
        case OH_Drawing_PlaceholderVerticalAlignment::ALIGNMENT_CENTER_OF_ROW_BOX:
          pos.y = top + lineHeights[j] / 2 - imageSpans_[i].height / 2;
          break;
        case OH_Drawing_PlaceholderVerticalAlignment::ALIGNMENT_BOTTOM_OF_ROW_BOX:
          pos.y = bottom - imageSpans_[i].height - imageSpans_[i].marginBottom;
          break;
        case OH_Drawing_PlaceholderVerticalAlignment::ALIGNMENT_OFFSET_AT_BASELINE:
          // todo         这里和安卓不同，安卓没有 / 2
          pos.y = top + baseLine - imageSpans_[i].height / 2 - imageSpans_[i].marginBottom;
          break;
        case OH_Drawing_PlaceholderVerticalAlignment::ALIGNMENT_ABOVE_BASELINE:
          // todo 有verticalAlignment属性时，不知如何处理
          pos.y = top + lineHeights[j] * 0.7 - imageSpans_[i].height;
          break;
        case OH_Drawing_PlaceholderVerticalAlignment::ALIGNMENT_BELOW_BASELINE:
          pos.y = top + baseLine;
          break;
        }
        pos.y += imageSpans_[i].top;
        if (pos.y < top) {
          pos.y = top;
        }
        if (pos.y + imageSpans_[i].height > bottom) {
          pos.y = bottom - imageSpans_[i].height;
        }
        break;
      }
      top = bottom;
      measureTop = measureBottom;
    }
    ret.spanPos.push_back(pos);
  }
  return bottom;
}

OhMeasureResult TextMeasurer::EndMeasure(int width, int widthMode, int height, int heightMode, float density) {
  OhMeasureResult ret;
  size_t lineCount = 0;
  
  typography_ = OH_ArkUI_StyledString_CreateTypography(styled_string_);
  double maxWidth = double(width);
  if (maxWidth == 0 || std::isnan(maxWidth)) {
    // fix text measure width wrong when maxWidth is nan or 0
    maxWidth = std::numeric_limits<double>::max();
  }
  
  OH_Drawing_TypographyLayout(typography_, maxWidth);
    
  // MATE 60, beta5, "新品" "商店" text cannot be fully displayed. So add 0.5.
  ret.width = ceil(OH_Drawing_TypographyGetLongestLine(typography_) + 0.5 * density);
  ret.height = OH_Drawing_TypographyGetHeight(typography_);
  ret.isEllipsized = OH_Drawing_TypographyDidExceedMaxLines(typography_);
  lineCount = OH_Drawing_TypographyGetLineCount(typography_);
  
  double realHeight = CalcSpanPostion(typography_, ret);
  ret.height = fmax(ret.height, realHeight);
  
  if (ret.height < minLineHeight_) {
    ret.height = minLineHeight_;
  }
  
#ifdef MEASURE_TEXT_LOG_RESULT
  FOOTSTONE_DLOG(INFO) << "hippy text - measure result, maxWidth: " << maxWidth
    << ", result: (" << ret.width << ", " << ret.height << "), "
    << logTextContent_.c_str() << ", lineCount: " << lineCount;
#endif

  if (lineHeight_ != 0) {
    ret.height = lineHeight_ * density * (double)lineCount;
#ifdef MEASURE_TEXT_LOG_RESULT
    FOOTSTONE_DLOG(INFO) << "hippy text - lineHeight fix result, result height: " << ret.height;
#endif
  }
  
  return ret;
}

void TextMeasurer::Destroy() {
  if (typography_) {
    OH_Drawing_DestroyTypography(typography_);
    typography_ = nullptr;
  }
  if (styled_string_) {
    OH_ArkUI_StyledString_Destroy(styled_string_);
    styled_string_ = nullptr;
  }
  if (fontCollection_) {
    OH_Drawing_DestroyFontCollection(fontCollection_);
    fontCollection_ = nullptr;
  }
  if (typographyStyle_) {
    OH_Drawing_DestroyTypographyStyle(typographyStyle_);
    typographyStyle_ = nullptr;
  }
}

} // namespace native
} // namespace render
} // namespace hippy
