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

#include "renderer/components/rich_text_image_span_view.h"
#include "renderer/dom_node/hr_node_props.h"
#include "renderer/utils/hr_text_convert_utils.h"
#include "renderer/utils/hr_url_utils.h"
#include "renderer/utils/hr_value_utils.h"

namespace hippy {
inline namespace render {
inline namespace native {

// TODO(hot):
static const std::string BASE64_IMAGE_PREFIX = "data:image";
static const std::string RAW_IMAGE_PREFIX = "hpfile://";
static const std::string ASSET_PREFIX = "asset:/";
static const std::string INTERNET_IMAGE_PREFIX = "http";

RichTextImageSpanView::RichTextImageSpanView(std::shared_ptr<NativeRenderContext> &ctx) : BaseView(ctx) {
  imageSpanNode_.SetImageObjectFit(ARKUI_OBJECT_FIT_FILL);
}

RichTextImageSpanView::~RichTextImageSpanView() {}

ImageSpanNode &RichTextImageSpanView::GetLocalRootArkUINode() {
  return imageSpanNode_;
}

bool RichTextImageSpanView::SetProp(const std::string &propKey, const HippyValue &propValue) {
  if (propKey == HRNodeProps::WIDTH) {
    auto value = HRValueUtils::GetFloat(propValue);
    GetLocalRootArkUINode().SetWidth(value);
    return true;
  } else if (propKey == HRNodeProps::HEIGHT) {
    auto value = HRValueUtils::GetFloat(propValue);
    GetLocalRootArkUINode().SetHeight(value);
    return true;
  } else if (propKey == "verticalAlign") {
    auto t = HRValueUtils::GetString(propValue);
    if (t == "top") {
      // TODO(hot):
    } else if (t == "middle") {
      
    } else if (t == "bottom") {
      
    } else if (t == "baseline") {
      
    }
    return true;
  } else if (propKey == "src") {
    auto value = HRValueUtils::GetString(propValue);
    if (value != src_) {
      src_ = value;
      fetchImage(value);
    }
    return true;
  } else if (propKey == "defaultSource") {
    auto value = HRValueUtils::GetString(propValue);
    if (!value.empty()) {
      auto sourceUrl = HRUrlUtils::convertAssetImageUrl(value);
      GetLocalRootArkUINode().SetAlt(sourceUrl);
      return true;
    }
    return false;
  }
  
  // Not to set some attributes for text span.
  return false;
}

void RichTextImageSpanView::UpdateRenderViewFrame(const HRRect &frame, const HRPadding &padding) {
  if (frame.x != 0 || frame.y != 0) { // c 测得span的位置
    GetLocalRootArkUINode().SetPosition(HRPosition(frame.x, frame.y));
    return;
  }
}

void RichTextImageSpanView::fetchImage(const std::string &imageUrl) {
  if (imageUrl.size() > 0) {
    if (imageUrl.find(BASE64_IMAGE_PREFIX) == 0) {
      GetLocalRootArkUINode().SetSources(imageUrl);
      return;
		} else if (imageUrl.find(RAW_IMAGE_PREFIX) == 0) {
			std::string convertUrl = ConvertToLocalPathIfNeeded(imageUrl);
      GetLocalRootArkUINode().SetSources(convertUrl);
      return;
		} else if (HRUrlUtils::isWebUrl(imageUrl)) {
			GetLocalRootArkUINode().SetSources(imageUrl);
      return;
		} else if (imageUrl.find(ASSET_PREFIX) == 0) {
      std::string resourceStr = HRUrlUtils::convertAssetImageUrl(imageUrl);
      GetLocalRootArkUINode().SetSources(resourceStr);
		}
    
    // TODO(hot):
	}
}

} // namespace native
} // namespace render
} // namespace hippy
