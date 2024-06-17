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

#include "renderer/components/refresh_wrapper_view.h"
#include "renderer/utils/hr_value_utils.h"

namespace hippy {
inline namespace render {
inline namespace native {

RefreshWrapperView::RefreshWrapperView(std::shared_ptr<NativeRenderContext> &ctx) : BaseView(ctx) {}

RefreshWrapperView::~RefreshWrapperView() {
  if (!children_.empty()) {
    for (const auto &child : children_) {
      listNode_.RemoveChild(child->GetLocalRootArkUINode());
    }
    children_.clear();
  }
}

ListNode &RefreshWrapperView::GetLocalRootArkUINode() { return listNode_; }

bool RefreshWrapperView::SetProp(const std::string &propKey, const HippyValue &propValue) {
  if (propKey == "bounceTime") {
    return true;
  } else if (propKey == "onScrollEnable") {
    return true;
  } else if (propKey == "scrollEventThrottle") {
    return true;
  }
  return BaseView::SetProp(propKey, propValue);
}

void RefreshWrapperView::OnChildInserted(std::shared_ptr<BaseView> const &childView, int32_t index) {
  BaseView::OnChildInserted(childView, index);
  listNode_.InsertChild(childView->GetLocalRootArkUINode(), index);
}

void RefreshWrapperView::OnChildRemoved(std::shared_ptr<BaseView> const &childView) {
  BaseView::OnChildRemoved(childView);
  listNode_.RemoveChild(childView->GetLocalRootArkUINode());
}

} // namespace native
} // namespace render
} // namespace hippy
