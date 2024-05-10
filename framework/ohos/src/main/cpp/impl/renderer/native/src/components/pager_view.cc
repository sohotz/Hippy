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

#include "renderer/components/pager_view.h"
#include "renderer/utils/hr_value_utils.h"

namespace hippy {
inline namespace render {
inline namespace native {

PagerView::PagerView(std::shared_ptr<NativeRenderContext> &ctx) : BaseView(ctx) {}

PagerView::~PagerView() {}

SwiperNode &PagerView::GetLocalRootArkUINode() { return swiperNode_; }

bool PagerView::SetProp(const std::string &propKey, HippyValue &propValue) {
  if (propKey == "initialPage") {
    return true;
  } else if (propKey == "scrollEnabled") {
    return true;
  } else if (propKey == "direction") {
    return true;
  } else if (propKey == "vertical") {
    return true;
  } else if (propKey == "pageMargin") {
    return true;
  }
  return BaseView::SetProp(propKey, propValue);
}

void PagerView::OnChildInserted(std::shared_ptr<BaseView> const &childView, int32_t index) {
  BaseView::OnChildInserted(childView, index);
  swiperNode_.InsertChild(childView->GetLocalRootArkUINode(), index);
}

void PagerView::OnChildRemoved(std::shared_ptr<BaseView> const &childView) {
  BaseView::OnChildRemoved(childView);
  swiperNode_.RemoveChild(childView->GetLocalRootArkUINode());
}

} // namespace native
} // namespace render
} // namespace hippy
