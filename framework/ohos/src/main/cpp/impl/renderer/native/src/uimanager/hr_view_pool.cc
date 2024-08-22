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

#include "renderer/uimanager/hr_view_pool.h"

namespace hippy {
inline namespace render {
inline namespace native {

void HRViewPool::SetPoolSize(size_t pool_size) {
  if (pool_size != HRViewPool::pool_size_) {
    HRViewPool::pool_size_ = pool_size;
  }
}

std::shared_ptr<DivView> HRViewPool::GetDivView(std::shared_ptr<NativeRenderContext> &ctx) {
  if (view_queue_.empty()) {
    return nullptr;
	} else {
    std::shared_ptr<DivView> view = view_queue_.front();
    view->SetNativeRenderContext(ctx);
    view->ReSetViewProps();
    view_queue_.pop();
    return view;
	}
}

void HRViewPool::SetDivViewToPool(std::shared_ptr<DivView> &view) {
  if (view_queue_.size() > pool_size_) {
    return;
  } else {
    view->CleanAllChildren();
    view_queue_.push(view);
  }
}

std::shared_ptr<ImageView> HRViewPool::GetImageView(std::shared_ptr<NativeRenderContext> &ctx) {
  if (image_queue_.empty()) {
    return nullptr;
	} else {
    std::shared_ptr<ImageView> imageView = image_queue_.front();
    imageView->SetNativeRenderContext(ctx);
    imageView->ReSetViewProps();
    image_queue_.pop();
    return imageView;
	}
}

void HRViewPool::SetImageToPool(std::shared_ptr<ImageView> &imageView) {
  if (image_queue_.size() > pool_size_) {
    return;
  } else {
    imageView->ReSetViewProps();
    image_queue_.push(imageView);
  }
}

std::shared_ptr<RichTextView> HRViewPool::GetRichTextView(std::shared_ptr<NativeRenderContext> &ctx) {
  if (text_queue_.empty()) {
    return nullptr;
  } else {
    std::shared_ptr<RichTextView> textView = text_queue_.front();
    textView->SetNativeRenderContext(ctx);
    textView->ReSetViewProps();
    text_queue_.pop();
    return textView;
  }
}

void HRViewPool::SetRichTextViewToPool(std::shared_ptr<RichTextView> &richTextView) {
  if (text_queue_.size() > pool_size_) {
    return;
  } else {
    richTextView->ReSetViewProps();
    richTextView->CleanAllChildren();
    text_queue_.push(richTextView);
  }
}

bool HRViewPool::IsPoolAllFull() {
  return (text_queue_.size() == pool_size_ && view_queue_.size() == pool_size_ && image_queue_.size() == pool_size_);
}

size_t HRViewPool::pool_size_ = 200;
std::queue<std::shared_ptr<DivView>> HRViewPool::view_queue_;
std::queue<std::shared_ptr<RichTextView>> HRViewPool::text_queue_;
std::queue<std::shared_ptr<ImageView>> HRViewPool::image_queue_;

} // namespace native
} // namespace render
} // namespace hippy