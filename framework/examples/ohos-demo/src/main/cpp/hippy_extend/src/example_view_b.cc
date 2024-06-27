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

#include "../include/example_view_b.h"
#include "renderer/utils/hr_value_utils.h"

namespace hippy {
inline namespace render {
inline namespace native {

ExampleViewB::ExampleViewB(std::shared_ptr<NativeRenderContext> &ctx) : CustomView(ctx) {
  stackNode_.SetStackNodeDelegate(this);
  stackNode_.AddChild(textNode_);
  textNode_.SetTextContent("This is a custom component B.");
}

ExampleViewB::~ExampleViewB() {
  if (!children_.empty()) {
    for (const auto &child : children_) {
      stackNode_.RemoveChild(child->GetLocalRootArkUINode());
    }
    children_.clear();
  }
}

StackNode &ExampleViewB::GetLocalRootArkUINode() {
  return stackNode_;
}

bool ExampleViewB::SetProp(const std::string &propKey, const HippyValue &propValue) {
  return BaseView::SetProp(propKey, propValue);
}

void ExampleViewB::OnChildInserted(std::shared_ptr<BaseView> const &childView, int32_t index) {
  BaseView::OnChildInserted(childView, index);
  stackNode_.InsertChild(childView->GetLocalRootArkUINode(), index);
}

void ExampleViewB::OnChildRemoved(std::shared_ptr<BaseView> const &childView) {
  BaseView::OnChildRemoved(childView);
  stackNode_.RemoveChild(childView->GetLocalRootArkUINode());
}

void ExampleViewB::OnClick() {
  if (eventClick_) {
    eventClick_();
  }
}

} // namespace native
} // namespace render
} // namespace hippy
