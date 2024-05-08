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

#include "renderer/components/base_view.h"

namespace hippy {
inline namespace render {
inline namespace native {

BaseView::BaseView(std::shared_ptr<NativeRenderContext> &ctx) : ctx_(ctx), tag_(0) {
  
}

bool BaseView::SetProp(const std::string &propKey, HippyValue &propValue) {

  return false;
}

void BaseView::AddSubRenderView(std::shared_ptr<BaseView> &subView, int32_t index) {
  if (index < 0 || index > (int32_t)children_.size()) {
    index = (int32_t)children_.size();
  }
  OnChildInserted(subView, index);
  auto it = children_.begin() + index;
  subView->SetParent(shared_from_this());
  children_.insert(it, std::move(subView));
}

void BaseView::RemoveSubView(std::shared_ptr<BaseView> &subView) {
  auto it = std::find(children_.begin(), children_.end(), subView);
  if (it != children_.end()) {
    auto view = std::move(*it);
    children_.erase(it);
    OnChildRemoved(view);
  }
}

void BaseView::RemoveFromParentView() {
  auto parentView = parent_.lock();
  if (parentView) {
    auto thisView = shared_from_this();
    parentView->RemoveSubView(thisView);
    SetParent(nullptr);
  }
}

bool BaseView::IsImageSpan() {
  auto parentView = parent_.lock();
  if (parentView && parentView->GetViewType() == "Text" && view_type_ == "Image") {
    return true;
  }
  return false;
}

void BaseView::SetRenderViewFrame(const HRRect &frame) {
  UpdateRenderViewFrame(frame);
}

void BaseView::UpdateRenderViewFrame(const HRRect &frame) {
  if (IsImageSpan()) {
    if (frame.x != 0 || frame.y != 0) { // c 测得span的位置
      GetLocalRootArkUINode().SetPosition(HRPosition(frame.x, frame.y));
      return;
    }
  }

  GetLocalRootArkUINode().SetPosition(HRPosition(frame.x, frame.y));
  GetLocalRootArkUINode().SetSize(HRSize(frame.width, frame.height));
}

void BaseView::UpdateEventListener(HippyValueObjectType &newEvents) {
//     this.events = newEvents
}

bool BaseView::CheckRegisteredEvent(std::string &eventName) {
//     if (this.events && this.events.has(eventName)) {
//       let value = this.events.get(eventName)
//       if (typeof value == 'boolean') {
//         return value
//       }
//     }
  return false;
}

} // namespace native
} // namespace render
} // namespace hippy
