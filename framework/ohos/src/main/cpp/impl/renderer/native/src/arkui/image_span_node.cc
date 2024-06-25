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

#include "renderer/arkui/image_span_node.h"
#include "renderer/arkui/native_node_api.h"

namespace hippy {
inline namespace render {
inline namespace native {

ImageSpanNode::ImageSpanNode() : ArkUINode(NativeNodeApi::GetInstance()->createNode(ArkUI_NodeType::ARKUI_NODE_IMAGE_SPAN)) {}

ImageSpanNode::~ImageSpanNode() {}

void ImageSpanNode::OnNodeEvent(ArkUI_NodeEvent *event) {
  if (imageSpanNodeDelegate_ == nullptr) {
    return;
  }
  
  if (OH_ArkUI_NodeEvent_GetEventType(event) == ArkUI_NodeEventType::NODE_ON_CLICK) {
    imageSpanNodeDelegate_->OnClick();
  }
}

void ImageSpanNode::SetSpanNodeDelegate(ImageSpanNodeDelegate *imageSpanNodeDelegate) {
  imageSpanNodeDelegate_ = imageSpanNodeDelegate; 
}

ImageSpanNode &ImageSpanNode::SetSources(std::string const &src) {
  ArkUI_AttributeItem item;
  uri_ = src;
  item = {.string = uri_.c_str()};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_IMAGE_SPAN_SRC, &item));
  return *this;
}

ImageSpanNode &ImageSpanNode::SetAlt(std::string const &src) {
  if (!src.empty()) {
    ArkUI_AttributeItem item = {.string = src.c_str()};
    MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_IMAGE_SPAN_ALT, &item));
  }
  return *this;
}

ImageSpanNode &ImageSpanNode::SetImageObjectFit(ArkUI_ObjectFit fit) {
  ArkUI_NumberValue value[] = {{.i32 = fit}};
  ArkUI_AttributeItem item = {.value = value, .size = sizeof(value) / sizeof(ArkUI_NumberValue)};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_IMAGE_OBJECT_FIT, &item));
  return *this;
}

} // namespace native
} // namespace render
} // namespace hippy
