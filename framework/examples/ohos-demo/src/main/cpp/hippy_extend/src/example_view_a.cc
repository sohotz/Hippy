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

#include "../include/example_view_a.h"
#include "renderer/utils/hr_value_utils.h"

namespace hippy {
inline namespace render {
inline namespace native {

ExampleViewA::ExampleViewA(std::shared_ptr<NativeRenderContext> &ctx) : CustomView(ctx) {
  stackNode_.AddChild(textNode_);
  textNode_.SetTextContent("This is a custom component A.");
}

ExampleViewA::~ExampleViewA() {
  if (!children_.empty()) {
    for (const auto &child : children_) {
      stackNode_.RemoveChild(child->GetLocalRootArkUINode());
    }
    children_.clear();
  }
}

StackNode &ExampleViewA::GetLocalRootArkUINode() {
  return stackNode_;
}

bool ExampleViewA::SetProp(const std::string &propKey, const HippyValue &propValue) {
  return BaseView::SetProp(propKey, propValue);
}

void ExampleViewA::OnChildInserted(std::shared_ptr<BaseView> const &childView, int32_t index) {
  BaseView::OnChildInserted(childView, index);
  stackNode_.InsertChild(childView->GetLocalRootArkUINode(), index);
}

void ExampleViewA::OnChildRemoved(std::shared_ptr<BaseView> const &childView) {
  BaseView::OnChildRemoved(childView);
  stackNode_.RemoveChild(childView->GetLocalRootArkUINode());
}

} // namespace native
} // namespace render
} // namespace hippy
