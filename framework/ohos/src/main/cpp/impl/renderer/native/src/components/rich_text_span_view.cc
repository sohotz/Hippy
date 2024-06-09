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

#include "renderer/components/rich_text_span_view.h"
#include "renderer/utils/hr_text_convert_utils.h"
#include "renderer/utils/hr_value_utils.h"

namespace hippy {
inline namespace render {
inline namespace native {

RichTextSpanView::RichTextSpanView(std::shared_ptr<NativeRenderContext> &ctx) : BaseView(ctx) {
  spanNode_.SetSpanNodeDelegate(this);
}

RichTextSpanView::~RichTextSpanView() {}

SpanNode &RichTextSpanView::GetLocalRootArkUINode() {
  return spanNode_;
}

bool RichTextSpanView::SetProp(const std::string &propKey, const HippyValue &propValue) {
  if (propKey == "text") {
    std::string value = HRValueUtils::GetString(propValue);
    if (value != text_) {
//       GetLocalRootArkUINode().SetTextContent(value);
      text_ = value;
    }
    return true;
  } else if (propKey == "color") {
    uint32_t value = HRValueUtils::GetUint32(propValue);
    if (firstSetColor_ || value != color_) {
      GetLocalRootArkUINode().SetFontColor(value);
      color_ = value;
      firstSetColor_ = false;
    }
    return true;
  } else if (propKey == "enableScale") {
    return true;
  } else if (propKey == "fontFamily") {
    
    return true;
  } else if (propKey == "fontSize") {
    float value = HRValueUtils::GetFloat(propValue);
    if (value != fontSize_) {
      GetLocalRootArkUINode().SetFontSize(value);
      fontSize_ = value;
    }
    return true;
  } else if (propKey == "fontStyle") {
    std::string value = HRValueUtils::GetString(propValue);
    int32_t style = HRTextConvertUtils::FontStyleToArk(value);
    if (style != fontStyle_) {
      GetLocalRootArkUINode().SetFontStyle(style);
      fontStyle_ = style;
    }
    return true;
  } else if (propKey == "fontWeight") {
    std::string value = HRValueUtils::GetString(propValue);
    ArkUI_FontWeight weight = HRTextConvertUtils::FontWeightToArk(value);
    if (weight != fontWeight_) {
      GetLocalRootArkUINode().SetFontWeight(weight);
      fontWeight_ = weight;
    }
    return true;
  } else if (propKey == "letterSpacing") {
    float value = HRValueUtils::GetFloat(propValue);
    if (firstSetLetterSpacing_ || value != letterSpacing_) {
      GetLocalRootArkUINode().SetTextLetterSpacing(value);
      letterSpacing_ = value;
      firstSetLetterSpacing_ = false;
    }
    return true;
  } else if (propKey == "lineHeight") {
    float value = HRValueUtils::GetFloat(propValue);
    if (value != lineHeight_) {
      GetLocalRootArkUINode().SetTextLineHeight(value);
      lineHeight_ = value;
    }
    return true;
  } else if (propKey == "lineSpacingExtra") {
    return true;
  } else if (propKey == "lineSpacingMultiplier") {
    return true;
  } else if (propKey == "numberOfLines") {
    int32_t value = HRValueUtils::GetInt32(propValue, 1);
    if (value != numberOfLines_) {
//       GetLocalRootArkUINode().SetTextMaxLines(value);
      numberOfLines_ = value;
    }
    return true;
  } else if (propKey == "textAlign") {
    std::string value = HRValueUtils::GetString(propValue);
    ArkUI_TextAlignment align = HRTextConvertUtils::TextAlignToArk(value);
    if (firstSetTextAlign_ || align != textAlign_) {
//       GetLocalRootArkUINode().SetTextAlign(align);
      textAlign_ = align;
      firstSetTextAlign_ = false;
    }
    return true;
  } else if (propKey == "textDecorationLine") {
    
    return true;
  } else if (propKey == "textDecorationColor") {
    
    return true;
  } else if (propKey == "textDecorationStyle") {
    
    return true;
  } else if (propKey == "textShadowColor") {
    
    return true;
  } else if (propKey == "textShadowOffset") {
    
    return true;
  } else if (propKey == "textShadowRadius") {
    
    return true;
  } else if (propKey == "ellipsizeMode") {
    
    return true;
  } else if (propKey == "breakStrategy") {
    
    return true;
  }
  
  // Not to set some attributes for text span.
  // For example: NODE_BACKGROUND_COLOR will return ARKUI_ERROR_CODE_ATTRIBUTE_OR_EVENT_NOT_SUPPORTED (106102)
  return false;
}

void RichTextSpanView::UpdateRenderViewFrame(const HRRect &frame, const HRPadding &padding) {
  // Nothing to set for text span.
  // NODE_POSITION / NODE_WIDTH will return ARKUI_ERROR_CODE_ATTRIBUTE_OR_EVENT_NOT_SUPPORTED (106102)
}

void RichTextSpanView::OnClick() {
  if (eventClick_) {
    eventClick_();
  }
}

} // namespace native
} // namespace render
} // namespace hippy
