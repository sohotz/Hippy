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

#include "renderer/utils/hr_text_convert_utils.h"

namespace hippy {
inline namespace render {
inline namespace native {

int32_t HRTextConvertUtils::FontWeightToArk(std::string &str) {
  if (str == "normal") {
    return ArkUI_FontWeight::ARKUI_FONT_WEIGHT_NORMAL;
  } else if (str == "bold") {
    return ArkUI_FontWeight::ARKUI_FONT_WEIGHT_BOLD;
  } else {
    auto w = std::atoi(str.c_str());
    if (std::isnan(w) || w == 0) {
      return ArkUI_FontWeight::ARKUI_FONT_WEIGHT_NORMAL;
    }
    if (w < 200) {
      return ArkUI_FontWeight::ARKUI_FONT_WEIGHT_W100;
    } else if (w < 300) {
      return ArkUI_FontWeight::ARKUI_FONT_WEIGHT_W200;
    } else if (w < 400) {
      return ArkUI_FontWeight::ARKUI_FONT_WEIGHT_W300;
    } else if (w < 500) {
      return ArkUI_FontWeight::ARKUI_FONT_WEIGHT_W400;
    } else if (w < 600) {
      return ArkUI_FontWeight::ARKUI_FONT_WEIGHT_W500;
    } else if (w < 700) {
      return ArkUI_FontWeight::ARKUI_FONT_WEIGHT_W600;
    } else if (w < 800) {
      return ArkUI_FontWeight::ARKUI_FONT_WEIGHT_W700;
    } else if (w < 900) {
      return ArkUI_FontWeight::ARKUI_FONT_WEIGHT_W800;
    } else {
      return ArkUI_FontWeight::ARKUI_FONT_WEIGHT_W900;
    }
  }
}

int32_t HRTextConvertUtils::FontStyleToArk(std::string &str) {
  if (str == "italic") {
    return 1;
  } else {
    return 0;
  }
}

int32_t HRTextConvertUtils::TextAlignToArk(std::string &str) {
  if (str == "left") {
    return ArkUI_TextAlignment::ARKUI_TEXT_ALIGNMENT_START;
  } else if (str == "right") {
    return ArkUI_TextAlignment::ARKUI_TEXT_ALIGNMENT_END;
  } else if (str == "center") {
    return ArkUI_TextAlignment::ARKUI_TEXT_ALIGNMENT_CENTER;
  } else {
    return ArkUI_TextAlignment::ARKUI_TEXT_ALIGNMENT_START;
  }
}

} // namespace native
} // namespace render
} // namespace hippy
