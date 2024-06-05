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

#include "renderer/components/pull_header_view.h"
#include "renderer/components/list_view.h"
#include "renderer/components/waterfall_view.h"
#include "renderer/utils/hr_value_utils.h"

namespace hippy {
inline namespace render {
inline namespace native {

PullHeaderView::PullHeaderView(std::shared_ptr<NativeRenderContext> &ctx) : ListItemView(ctx) {}

PullHeaderView::~PullHeaderView() {}

bool PullHeaderView::SetProp(const std::string &propKey, const HippyValue &propValue) {
  return ListItemView::SetProp(propKey, propValue);
}

void PullHeaderView::Call(const std::string &method, const std::vector<HippyValue> params,
                    std::function<void(const HippyValue &result)> callback) {
  if (method == "collapsePullHeader") {
    OnHeadRefreshFinish();
  } else if (method == "collapsePullHeaderWithOptions") {
    HippyValueObjectType map;
    bool r = params[0].ToObject(map);
    if (r && map.size() > 0) {
      auto collapseTime = HRValueUtils::GetInt32(map["time"]);
      if (collapseTime > 0) {
        // TODO(hot):
        FOOTSTONE_DLOG(INFO) << "PullHeaderView collapse delay: " << collapseTime;
        OnHeadRefreshFinish();
      } else {
        OnHeadRefreshFinish();
      }
    }
  } else if (method == "expandPullHeader") {
    OnHeaderRefresh();
  }
}

void PullHeaderView::OnHeadRefreshFinish() {
  auto parentView = parent_.lock();
  if (parentView) {
    if (parentView->GetViewType() == "ListView") {
      auto listView = std::static_pointer_cast<ListView>(parentView);
      listView->ScrollToIndex(1, true);
    } else if (parentView->GetViewType() == "WaterfallView") {
      auto waterView = std::static_pointer_cast<WaterfallView>(parentView);
      // TODO(hot):
      //waterView->OnHeadRefreshFinish();
    }
  }
}

void PullHeaderView::OnHeaderRefresh() {
  auto parentView = parent_.lock();
  if (parentView) {
    if (parentView->GetViewType() == "ListView") {
      auto listView = std::static_pointer_cast<ListView>(parentView);
      listView->ScrollToIndex(0, true);
    }
  }
}

} // namespace native
} // namespace render
} // namespace hippy
