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

#include "renderer/components/list_view.h"

namespace hippy {
inline namespace render {
inline namespace native {

ListView::ListView(std::shared_ptr<NativeRenderContext> &ctx) : BaseView(ctx) {
  stackNode_.AddChild(listNode_);
  
  auto weak_view = weak_from_this();
  end_batch_callback_id_ = ctx_->GetNativeRender().lock()->AddEndBatchCallback(ctx_->GetRootId(), [weak_view]() {
    auto view = weak_view.lock();
    if (view) {
      auto listView = std::static_pointer_cast<ListView>(view);
      listView->HandleOnChildrenUpdated();
    }
  });
}

ListView::~ListView() {
  ctx_->GetNativeRender().lock()->RemoveEndBatchCallback(ctx_->GetRootId(), end_batch_callback_id_);
}

StackNode &ListView::GetLocalRootArkUINode() { return stackNode_; }

bool ListView::SetProp(const std::string &propKey, HippyValue &propValue) {

  return BaseView::SetProp(propKey, propValue);
}

void ListView::HandleOnChildrenUpdated() {
  for (uint32_t i = 0; i < children_.size(); i++) {
//     ListItemNode itemNode;
//     listItemNodes_.emplace_back(itemNode);
  }
}

} // namespace native
} // namespace render
} // namespace hippy
