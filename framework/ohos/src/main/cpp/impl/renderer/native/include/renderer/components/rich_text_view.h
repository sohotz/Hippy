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

#include "renderer/components/base_view.h"
#include "renderer/arkui/text_node.h"
#include <optional>

namespace hippy {
inline namespace render {
inline namespace native {

class RichTextView : public BaseView {
public:
  RichTextView(std::shared_ptr<NativeRenderContext> &ctx);
  ~RichTextView();

  TextNode &GetLocalRootArkUINode() override;
  bool SetProp(const std::string &propKey, const HippyValue &propValue) override;
  void OnSetPropsEnd() override;
  void UpdateRenderViewFrame(const HRRect &frame, const HRPadding &padding) override;
  
  void OnChildInserted(std::shared_ptr<BaseView> const &childView, int32_t index) override;
  void OnChildRemoved(std::shared_ptr<BaseView> const &childView, int32_t index) override;
  
  void SendTextEllipsizedEvent();
  
private:
  TextNode textNode_;
  
  std::optional<std::string> text_;
  std::optional<uint32_t> color_;
  std::optional<std::string> fontFamily_;
  std::optional<float> fontSize_;
  std::optional<int32_t> fontStyle_;
  std::optional<int32_t> fontWeight_;
  std::optional<float> letterSpacing_;
  std::optional<float> lineHeight_;
  std::optional<int32_t> numberOfLines_;
  std::optional<int32_t> textAlign_;
  std::optional<std::string> ellipsizeModeValue_;
  
  ArkUI_TextDecorationType decorationType_ = ARKUI_TEXT_DECORATION_TYPE_NONE;
  ArkUI_TextDecorationStyle decorationStyle_ = ARKUI_TEXT_DECORATION_STYLE_SOLID;
  uint32_t decorationColor_ = 0xff000000;
  float textShadowRadius_ = 0.f;
  float textShadowOffsetX_ = 0.f;
  float textShadowOffsetY_ = 0.f;
  uint32_t textShadowColor_ = 0xff000000;
  
  bool toSetTextDecoration_ = false;
  bool toSetTextShadow = false;
  
  bool isListenEllipsized_ = false;
  bool toSendEllipsizedEvent_ = false;
};

} // namespace native
} // namespace render
} // namespace hippy
