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
#include "renderer/arkui/span_node.h"

namespace hippy {
inline namespace render {
inline namespace native {

class RichTextSpanView : public BaseView, public SpanNodeDelegate {
public:
  RichTextSpanView(std::shared_ptr<NativeRenderContext> &ctx);
  ~RichTextSpanView();

  SpanNode &GetLocalRootArkUINode() override;
  bool SetProp(const std::string &propKey, const HippyValue &propValue) override;
  void UpdateRenderViewFrame(const HRRect &frame, const HRPadding &padding) override;

  void OnClick() override;
  
private:
  SpanNode spanNode_;
  
  std::string text_;
  uint32_t color_ = 0;
  std::string fontFamily_;
  float fontSize_ = 0;
  int32_t fontStyle_ = 0;
  int32_t fontWeight_ = 0;
  float letterSpacing_ = 0;
  float lineHeight_ = 0;
  int32_t numberOfLines_ = 1;
  int32_t textAlign_ = 0;

  bool firstSetColor_ = true;
  bool firstSetLetterSpacing_ = true;
  bool firstSetTextAlign_ = true;
};

} // namespace native
} // namespace render
} // namespace hippy
