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

#include <queue>
#include "renderer/components/div_view.h"
#include "renderer/components/rich_text_view.h"
#include "renderer/components/image_view.h"
#include "renderer/native_render_context.h"

namespace hippy {
inline namespace render {
inline namespace native {

class HRViewPool {
public:
  static std::shared_ptr<DivView> GetDivView(std::shared_ptr<NativeRenderContext> &ctx);
  static void SetDivViewToPool(std::shared_ptr<DivView> &view);
  
  static std::shared_ptr<ImageView> GetImageView(std::shared_ptr<NativeRenderContext> &ctx);
  static void SetImageToPool(std::shared_ptr<ImageView> &imageView);
  
  static std::shared_ptr<RichTextView> GetRichTextView(std::shared_ptr<NativeRenderContext> &ctx);
  static void SetRichTextViewToPool(std::shared_ptr<RichTextView> &richTextView);
  
  static void SetPoolSize(size_t pool_size);
  
  static bool IsPoolAllFull();
  
private:
  static size_t pool_size_;
  static std::queue<std::shared_ptr<DivView>> view_queue_;
  static std::queue<std::shared_ptr<RichTextView>> text_queue_;
  static std::queue<std::shared_ptr<ImageView>> image_queue_;
};

} // namespace native
} // namespace render
} // namespace hippy
