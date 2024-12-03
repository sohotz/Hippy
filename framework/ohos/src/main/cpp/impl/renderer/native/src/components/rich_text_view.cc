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

#include "renderer/components/rich_text_view.h"
#include "renderer/dom_node/hr_node_props.h"
#include "renderer/utils/hr_pixel_utils.h"
#include "renderer/utils/hr_text_convert_utils.h"
#include "renderer/utils/hr_event_utils.h"
#include "renderer/utils/hr_value_utils.h"

namespace hippy {
inline namespace render {
inline namespace native {

RichTextView::RichTextView(std::shared_ptr<NativeRenderContext> &ctx) : BaseView(ctx) {

}

RichTextView::~RichTextView() {
  if (!children_.empty()) {
    if (textNode_) {
      for (const auto &child : children_) {
        textNode_->RemoveChild(child->GetLocalRootArkUINode());
      }
    }
    children_.clear();
  }
}

TextNode *RichTextView::GetLocalRootArkUINode() {
  return textNode_.get();
}

void RichTextView::CreateArkUINodeImpl() {
  textNode_ = std::make_shared<TextNode>();
}

void RichTextView::DestroyArkUINodeImpl() {
  textNode_ = nullptr;
  ClearProps();
}

bool RichTextView::RecycleArkUINodeImpl(std::shared_ptr<RecycleView> &recycleView) {
  textNode_->ResetAllAttributes();
  recycleView->cachedNodes_.resize(1);
  recycleView->cachedNodes_[0] = textNode_;
  textNode_ = nullptr;
  ClearProps();
  return true;
}

bool RichTextView::ReuseArkUINodeImpl(std::shared_ptr<RecycleView> &recycleView) {
  if (recycleView->cachedNodes_.size() < 1) {
    return false;
  }
  textNode_ = std::static_pointer_cast<TextNode>(recycleView->cachedNodes_[0]);
  return true;
}

bool RichTextView::SetPropImpl(const std::string &propKey, const HippyValue &propValue) {
  if (propKey == "text") {
    std::string value = HRValueUtils::GetString(propValue);
    if (!text_.has_value() || value != text_) {
      GetLocalRootArkUINode()->SetTextContent(value);
      text_ = value;
    }
    return true;
  } else if (propKey == HRNodeProps::COLOR) {
    uint32_t value = HRValueUtils::GetUint32(propValue);
    if (!color_.has_value() || value != color_) {
      GetLocalRootArkUINode()->SetFontColor(value);
      color_ = value;
    }
    return true;
  } else if (propKey == "enableScale") {
    return true;
  } else if (propKey == HRNodeProps::FONT_FAMILY) {
    std::string value = HRValueUtils::GetString(propValue);
    if (!fontFamily_.has_value() || value != fontFamily_) {
      GetLocalRootArkUINode()->SetFontFamily(value);
      fontFamily_ = value;
    }
    return true;
  } else if (propKey == HRNodeProps::FONT_SIZE) {
    float value = HRValueUtils::GetFloat(propValue);
    if (!fontSize_.has_value() || value != fontSize_) {
      GetLocalRootArkUINode()->SetFontSize(value);
      fontSize_ = value;
    }
    return true;
  } else if (propKey == HRNodeProps::FONT_STYLE) {
    std::string value = HRValueUtils::GetString(propValue);
    int32_t style = HRTextConvertUtils::FontStyleToArk(value);
    if (!fontStyle_.has_value() || style != fontStyle_) {
      GetLocalRootArkUINode()->SetFontStyle(style);
      fontStyle_ = style;
    }
    return true;
  } else if (propKey == HRNodeProps::FONT_WEIGHT) {
    std::string value = HRValueUtils::GetString(propValue);
    ArkUI_FontWeight weight = HRTextConvertUtils::FontWeightToArk(value);
    if (!fontWeight_.has_value() || weight != fontWeight_) {
      GetLocalRootArkUINode()->SetFontWeight(weight);
      fontWeight_ = weight;
    }
    return true;
  } else if (propKey == HRNodeProps::LETTER_SPACING) {
    float value = HRValueUtils::GetFloat(propValue);
    if (!letterSpacing_.has_value() || value != letterSpacing_) {
      GetLocalRootArkUINode()->SetTextLetterSpacing(value);
      letterSpacing_ = value;
    }
    return true;
  } else if (propKey == HRNodeProps::LINE_HEIGHT) {
    float value = HRValueUtils::GetFloat(propValue);
    if (!lineHeight_.has_value() || value != lineHeight_) {
      GetLocalRootArkUINode()->SetTextLineHeight(value);
      GetLocalRootArkUINode()->SetTextHalfLeading(true);
      lineHeight_ = value;
    }
    return true;
  } else if (propKey == HRNodeProps::LINE_SPACING_EXTRA) { // Android有，iOS/ohos无
    return true;
  } else if (propKey == HRNodeProps::LINE_SPACING_MULTIPLIER) { // Android有，iOS/ohos无
    return true;
  } else if (propKey == HRNodeProps::NUMBER_OF_LINES) {
    int32_t value = HRValueUtils::GetInt32(propValue, 1);
    if (value <= 0) {
      value = 10000000;
    }
    if (!numberOfLines_.has_value() || value != numberOfLines_) {
      GetLocalRootArkUINode()->SetTextMaxLines(value);
      numberOfLines_ = value;
    }
    return true;
  } else if (propKey == HRNodeProps::TEXT_ALIGN) {
    std::string value = HRValueUtils::GetString(propValue);
    ArkUI_TextAlignment align = HRTextConvertUtils::TextAlignToArk(value);
    if (!textAlign_.has_value() || align != textAlign_) {
      GetLocalRootArkUINode()->SetTextAlign(align);
      textAlign_ = align;
    }
    return true;
  } else if (propKey == HRNodeProps::TEXT_DECORATION_LINE) {
    std::string value = HRValueUtils::GetString(propValue);
    decorationType_ = HRTextConvertUtils::TextDecorationTypeToArk(value);
    toSetTextDecoration_ = true;
    return true;
  } else if (propKey == HRNodeProps::TEXT_DECORATION_COLOR) {
    decorationColor_ = HRValueUtils::GetUint32(propValue);
    toSetTextDecoration_ = true;
    return true;
  } else if (propKey == HRNodeProps::TEXT_DECORATION_STYLE) {
    std::string value = HRValueUtils::GetString(propValue);
    decorationStyle_ = HRTextConvertUtils::TextDecorationStyleToArk(value);
    toSetTextDecoration_ = true;
    return true;
  } else if (propKey == HRNodeProps::TEXT_SHADOW_COLOR) {
    textShadowColor_ = HRValueUtils::GetUint32(propValue);
    toSetTextShadow = true;
    return true;
  } else if (propKey == HRNodeProps::TEXT_SHADOW_OFFSET) {
    HippyValueObjectType m;
    if (propValue.ToObject(m)) {
      textShadowOffsetX_ = HRValueUtils::GetFloat(m["width"]);
      textShadowOffsetY_ = HRValueUtils::GetFloat(m["height"]);
    }
    toSetTextShadow = true;
    return true;
  } else if (propKey == HRNodeProps::TEXT_SHADOW_RADIUS) {
    textShadowRadius_ = HRValueUtils::GetFloat(propValue);
    toSetTextShadow = true;
    return true;
  } else if (propKey == HRNodeProps::ELLIPSIZE_MODE) {
    std::string value = HRValueUtils::GetString(propValue);
    if (!ellipsizeModeValue_.has_value() || value != ellipsizeModeValue_) {
      ArkUI_EllipsisMode ellipsisMode = ARKUI_ELLIPSIS_MODE_END;
      ArkUI_TextOverflow textOverflow = ARKUI_TEXT_OVERFLOW_ELLIPSIS;
      HRTextConvertUtils::EllipsisModeToArk(value, ellipsisMode, textOverflow);
      GetLocalRootArkUINode()->SetTextOverflow(textOverflow);
      GetLocalRootArkUINode()->SetTextEllipsisMode(ellipsisMode);
      ellipsizeModeValue_ = value;
    }
    return true;
  } else if (propKey == HRNodeProps::BREAK_STRATEGY) {
    std::string value = HRValueUtils::GetString(propValue);
    ArkUI_WordBreak wordBreak = HRTextConvertUtils::WordBreakToArk(value);
    GetLocalRootArkUINode()->SetWordBreak(wordBreak);
    return true;
  } else if (propKey == "ellipsized") {
    isListenEllipsized_ = HRValueUtils::GetBool(propValue, false);
    if (isListenEllipsized_ && toSendEllipsizedEvent_) {
      HREventUtils::SendComponentEvent(ctx_, tag_, "ellipsized", nullptr);
      toSendEllipsizedEvent_ = false;
    }
    return true;
  }
  
  return BaseView::SetPropImpl(propKey, propValue);
}

void RichTextView::OnSetPropsEndImpl() {
  if (!fontSize_.has_value()) {
    float defaultValue = HRNodeProps::FONT_SIZE_SP;
    GetLocalRootArkUINode()->SetFontSize(defaultValue);
    fontSize_ = defaultValue;
  }
  if (!ellipsizeModeValue_.has_value()) {
    std::string defaultValue = "tail";
    ellipsizeModeValue_ = defaultValue;
    GetLocalRootArkUINode()->SetTextOverflow(ARKUI_TEXT_OVERFLOW_ELLIPSIS);
    GetLocalRootArkUINode()->SetTextEllipsisMode(ARKUI_ELLIPSIS_MODE_END);
  }
  if (toSetTextDecoration_) {
    toSetTextDecoration_ = false;
    GetLocalRootArkUINode()->SetTextDecoration(decorationType_, decorationColor_, decorationStyle_);
  }
  if (toSetTextShadow) {
    toSetTextShadow = false;
    GetLocalRootArkUINode()->SetTextShadow(HRPixelUtils::DpToVp(textShadowRadius_), ARKUI_SHADOW_TYPE_COLOR, textShadowColor_, HRPixelUtils::DpToVp(textShadowOffsetX_), HRPixelUtils::DpToVp(textShadowOffsetY_));
  }
  BaseView::OnSetPropsEndImpl();
}

void RichTextView::UpdateRenderViewFrameImpl(const HRRect &frame, const HRPadding &padding) {
  BaseView::UpdateRenderViewFrameImpl(frame, padding);
  textNode_->SetPadding(padding.paddingTop, padding.paddingRight, padding.paddingBottom, padding.paddingLeft);
}

void RichTextView::OnChildInsertedImpl(std::shared_ptr<BaseView> const &childView, int32_t index) {
  BaseView::OnChildInsertedImpl(childView, index);
  textNode_->InsertChild(childView->GetLocalRootArkUINode(), index);
}

void RichTextView::OnChildRemovedImpl(std::shared_ptr<BaseView> const &childView, int32_t index) {
  BaseView::OnChildRemovedImpl(childView, index);
  textNode_->RemoveChild(childView->GetLocalRootArkUINode());
}

void RichTextView::SendTextEllipsizedEvent() {
  if (!isListenEllipsized_) {
    toSendEllipsizedEvent_ = true;
    return;
  }
  HREventUtils::SendComponentEvent(ctx_, tag_, "ellipsized", nullptr);
  toSendEllipsizedEvent_ = false;
}

void RichTextView::ClearProps() {
  text_.reset();
  color_.reset();
  fontFamily_.reset();
  fontSize_.reset();
  fontStyle_.reset();
  fontWeight_.reset();
  letterSpacing_.reset();
  lineHeight_.reset();
  numberOfLines_.reset();
  textAlign_.reset();
  ellipsizeModeValue_.reset();
}

} // namespace native
} // namespace render
} // namespace hippy
