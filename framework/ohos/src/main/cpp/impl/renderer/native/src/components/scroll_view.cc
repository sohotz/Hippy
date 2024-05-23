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

#include "renderer/components/scroll_view.h"
#include "renderer/utils/hr_value_utils.h"

namespace hippy {
inline namespace render {
inline namespace native {

ScrollView::ScrollView(std::shared_ptr<NativeRenderContext> &ctx) : BaseView(ctx) {
  scrollNode_.AddChild(stackNode_);
}

ScrollView::~ScrollView() {}

ScrollNode &ScrollView::GetLocalRootArkUINode() { return scrollNode_; }

bool ScrollView::SetProp(const std::string &propKey, const HippyValue &propValue) {
  if (propKey == "showScrollIndicator") {
    return true;
  } else if (propKey == "pagingEnabled") {
    return true;
  } else if (propKey == "flingEnabled") {
    return true;
  } else if (propKey == "contentOffset4Reuse") {
    return true;
  } else if (propKey == "scrollEnabled") {
    return true;
  } else if (propKey == "horizontal") {
    return true;
  } else if (propKey == "initialContentOffset") {
    return true;
  } else if (propKey == "scrollEventThrottle") {
    return true;
  } else if (propKey == "scrollMinOffset") {
    return true;
  }
  return BaseView::SetProp(propKey, propValue);
}

void ScrollView::OnChildInserted(std::shared_ptr<BaseView> const &childView, int32_t index) {
  BaseView::OnChildInserted(childView, index);
  stackNode_.InsertChild(childView->GetLocalRootArkUINode(), index);
}

void ScrollView::OnChildRemoved(std::shared_ptr<BaseView> const &childView) {
  BaseView::OnChildRemoved(childView);
  stackNode_.RemoveChild(childView->GetLocalRootArkUINode());
}

} // namespace native
} // namespace render
} // namespace hippy
