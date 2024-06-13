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

#include "renderer/components/hippy_render_view_creator.h"
#include "renderer/components/div_view.h"
#include "renderer/components/image_view.h"
#include "renderer/components/list_item_view.h"
#include "renderer/components/list_view.h"
#include "renderer/components/modal_view.h"
#include "renderer/components/pager_item_view.h"
#include "renderer/components/pager_view.h"
#include "renderer/components/pull_footer_view.h"
#include "renderer/components/pull_header_view.h"
#include "renderer/components/refresh_wrapper_item_view.h"
#include "renderer/components/refresh_wrapper_view.h"
#include "renderer/components/rich_text_view.h"
#include "renderer/components/rich_text_span_view.h"
#include "renderer/components/rich_text_image_span_view.h"
#include "renderer/components/scroll_view.h"
#include "renderer/components/text_input_view.h"
#include "renderer/components/waterfall_item_view.h"
#include "renderer/components/waterfall_view.h"
#include "renderer/components/web_view.h"

namespace hippy {
inline namespace render {
inline namespace native {

std::shared_ptr<BaseView> HippyCreateRenderView(std::string &view_name, bool is_parent_text, std::shared_ptr<NativeRenderContext> &ctx) {
  if (view_name == "View") {
    return std::make_shared<DivView>(ctx);
  } else if (view_name == "Image") {
    if (is_parent_text) {
      return std::make_shared<RichTextImageSpanView>(ctx);
    } else {
      return std::make_shared<ImageView>(ctx);
    }
  } else if (view_name == "Text") {
    if (is_parent_text) {
      return std::make_shared<RichTextSpanView>(ctx);
    } else {
      return std::make_shared<RichTextView>(ctx);
    }
  } else if (view_name == "Modal") {
    return std::make_shared<ModalView>(ctx);
  } else if (view_name == "ListView") {
    auto view = std::make_shared<ListView>(ctx);
    view->Init();
    return view;
  } else if (view_name == "ListViewItem") {
    return std::make_shared<ListItemView>(ctx);
  } else if (view_name == "ScrollView") {
    return std::make_shared<ScrollView>(ctx);
  } else if (view_name == "TextInput") {
    return std::make_shared<TextInputView>(ctx);
  } else if (view_name == "WebView") {
    return std::make_shared<WebView>(ctx);
  } else if (view_name == "ViewPager") {
    return std::make_shared<PagerView>(ctx);
  } else if (view_name == "ViewPagerItem") {
    return std::make_shared<PagerItemView>(ctx);
  } else if (view_name == "WaterfallView") {
    return std::make_shared<WaterfallView>(ctx);
  } else if (view_name == "WaterfallItem") {
    return std::make_shared<WaterfallItemView>(ctx);
  } else if (view_name == "RefreshWrapper") {
    return std::make_shared<RefreshWrapperView>(ctx);
  } else if (view_name == "RefreshWrapperItemView") {
    return std::make_shared<RefreshWrapperItemView>(ctx);
  } else if (view_name == "PullHeaderView") {
    return std::make_shared<PullHeaderView>(ctx);
  } else if (view_name == "PullFooterView") {
    return std::make_shared<PullFooterView>(ctx);
  }
  
  return nullptr;
}

} // namespace native
} // namespace render
} // namespace hippy
