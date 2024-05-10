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
#include <arkui/native_type.h>
#include <sys/stat.h>
#include "renderer/utils/hr_types.h"

namespace hippy {
inline namespace render {
inline namespace native {

class HRTextConvertUtil {
public:
  inline static int32_t FontWeightToArk(std::string &str) {
    if (str == "normal") {
      return ArkUI_FontWeight::ARKUI_FONT_WEIGHT_NORMAL;
    } else if (str == "bold") {
      return ArkUI_FontWeight::ARKUI_FONT_WEIGHT_BOLD;
    } else {
      auto w = std::atoi(str.c_str());
      if (std::isnan(w)) {
        return ArkUI_FontWeight::ARKUI_FONT_WEIGHT_NORMAL;
      }
      w = std::min(std::max(1, w), 1000);
      return w;
    }
  }
  
  inline static int32_t FontStyleToArk(std::string &str) {
    if (str == "italic") {
      return 1;
    } else {
      return 0;
    }
  }
  
  inline static int32_t TextAlignToArk(std::string &str) {
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
  
};

} // namespace native
} // namespace render
} // namespace hippy
