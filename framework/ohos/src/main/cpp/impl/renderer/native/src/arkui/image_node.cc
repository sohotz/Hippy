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

#include "renderer/arkui/image_node.h"
#include "renderer/arkui/native_node_api.h"

namespace hippy {
inline namespace render {
inline namespace native {

static constexpr ArkUI_NodeEventType IMAGE_NODE_EVENT_TYPES[] = {NODE_ON_CLICK, NODE_IMAGE_ON_COMPLETE, NODE_IMAGE_ON_ERROR};

using namespace std::literals;
constexpr std::string_view ASSET_PREFIX = "asset://";

ImageNode::ImageNode()
    : ArkUINode(NativeNodeApi::GetInstance()->createNode(ArkUI_NodeType::ARKUI_NODE_IMAGE)),
      imageNodeDelegate_(nullptr) {
  for (auto eventType : IMAGE_NODE_EVENT_TYPES) {
    MaybeThrow(NativeNodeApi::GetInstance()->registerNodeEvent(nodeHandle_, eventType, eventType));
  }
}

ImageNode::~ImageNode() {
  for (auto eventType : IMAGE_NODE_EVENT_TYPES) {
    NativeNodeApi::GetInstance()->unregisterNodeEvent(nodeHandle_, eventType);
  }
}

void ImageNode::SetNodeDelegate(ImageNodeDelegate *imageNodeDelegate) { imageNodeDelegate_ = imageNodeDelegate; }

void ImageNode::OnNodeEvent(ArkUI_NodeEvent *event) {
  if (imageNodeDelegate_ == nullptr) {
    return;
  }
  
  if (event->kind == ArkUI_NodeEventType::NODE_ON_CLICK) {
    imageNodeDelegate_->OnClick();
  } else if (event->kind == ArkUI_NodeEventType::NODE_IMAGE_ON_COMPLETE) {
    if (event->componentEvent.data[0].i32 == 1) {
      imageNodeDelegate_->OnComplete(event->componentEvent.data[1].f32, event->componentEvent.data[2].f32);
    }
  } else if (event->kind == ArkUI_NodeEventType::NODE_IMAGE_ON_ERROR) {
    imageNodeDelegate_->OnError(event->componentEvent.data[0].i32);
  }
}

ImageNode &ImageNode::SetSources(std::string const &src) {
  ArkUI_AttributeItem item;
  uri_ = src;
  if (uri_.rfind(ASSET_PREFIX, 0) == 0) {
    std::string resourceStr = std::string("resource://RAWFILE/") + "assets/";
    resourceStr += uri_.substr(ASSET_PREFIX.size());
    item = {.string = resourceStr.c_str()};
  } else {
    item = {.string = uri_.c_str()};
  }
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_IMAGE_SRC, &item));
  return *this;
}

ImageNode &ImageNode::SetResizeMode(HRImageResizeMode const &mode) {
  int32_t val = ARKUI_OBJECT_FIT_FILL;
  if (mode == HRImageResizeMode::Cover) {
    val = ARKUI_OBJECT_FIT_COVER;
  } else if (mode == HRImageResizeMode::Contain) {
    val = ARKUI_OBJECT_FIT_CONTAIN;
  } else if (mode == HRImageResizeMode::Center) {
    val = ARKUI_OBJECT_FIT_SCALE_DOWN;
  } else if (mode == HRImageResizeMode::Origin) {
    val = ARKUI_OBJECT_FIT_NONE;
  } else if (mode == HRImageResizeMode::FitXY) {
    val = ARKUI_OBJECT_FIT_FILL;
  } else if (mode == HRImageResizeMode::Repeat) {
    val = ARKUI_OBJECT_FIT_NONE;
    // TODO(hot):
  }

  ArkUI_NumberValue value[] = {{.i32 = val}};
  ArkUI_AttributeItem item = {value, sizeof(value) / sizeof(ArkUI_NumberValue), nullptr, nullptr};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_IMAGE_OBJECT_FIT, &item));
  return *this;
}

ImageNode &ImageNode::SetTintColor(uint32_t sharedColor) {
  if (!sharedColor) { // restore default value
    MaybeThrow(NativeNodeApi::GetInstance()->resetAttribute(nodeHandle_, NODE_IMAGE_COLOR_FILTER));
    return *this;
  }
  
  float ratio = 255;
  float red = (float)((sharedColor >> 16) & 0xff) / ratio;
  float green = (float)((sharedColor >> 8) & 0xff) / ratio;
  float blue = (float)((sharedColor >> 0) & 0xff) / ratio;

  ArkUI_NumberValue value[] = {{.f32 = 0}, {.f32 = 0}, {.f32 = 0}, {.f32 = red},   {.f32 = 0},
                               {.f32 = 0}, {.f32 = 0}, {.f32 = 0}, {.f32 = green}, {.f32 = 0},
                               {.f32 = 0}, {.f32 = 0}, {.f32 = 0}, {.f32 = blue},  {.f32 = 0},
                               {.f32 = 0}, {.f32 = 0}, {.f32 = 0}, {.f32 = 1},     {.f32 = 0}};

  ArkUI_AttributeItem item = {value, sizeof(value) / sizeof(ArkUI_NumberValue), nullptr, nullptr};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_IMAGE_COLOR_FILTER, &item));
  return *this;
}

ImageNode &ImageNode::SetBlur(float blur) {
  ArkUI_NumberValue value[] = {{.f32 = static_cast<float>(blur)}};
  ArkUI_AttributeItem item = {value, sizeof(value) / sizeof(ArkUI_NumberValue), nullptr, nullptr};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_BLUR, &item));
  return *this;
}

ImageNode &ImageNode::SetObjectRepeat(HRImageResizeMode const &resizeMode) {
  int32_t val = ARKUI_IMAGE_REPEAT_NONE;
  if (resizeMode == HRImageResizeMode::Repeat) {
    val = ARKUI_IMAGE_REPEAT_XY;
  }

  ArkUI_NumberValue value[] = {{.i32 = val}};
  ArkUI_AttributeItem item = {value, sizeof(value) / sizeof(ArkUI_NumberValue), nullptr, nullptr};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_IMAGE_OBJECT_REPEAT, &item));
  return *this;
}

ImageNode &ImageNode::SetInterpolation(int32_t interpolation) {
  ArkUI_NumberValue value[] = {{.i32 = interpolation}};
  ArkUI_AttributeItem item = {value, sizeof(value) / sizeof(ArkUI_NumberValue), nullptr, nullptr};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_IMAGE_INTERPOLATION, &item));
  return *this;
}

ImageNode &ImageNode::SetDraggable(bool draggable) {
  ArkUI_NumberValue value[] = {{.i32 = static_cast<int32_t>(draggable)}};
  ArkUI_AttributeItem item = {value, sizeof(value) / sizeof(ArkUI_NumberValue), nullptr, nullptr};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_IMAGE_DRAGGABLE, &item));
  return *this;
}

ImageNode &ImageNode::SetFocusable(bool focusable) {
  ArkUI_NumberValue value[] = {{.i32 = static_cast<int32_t>(focusable)}};
  ArkUI_AttributeItem item = {value, sizeof(value) / sizeof(ArkUI_NumberValue), nullptr, nullptr};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_FOCUSABLE, &item));
  return *this;
}

ImageNode &ImageNode::SetResizeMethod(std::string const &resizeMethod) {
  auto autoResize = (resizeMethod != "scale") ? 1 : 0;
  ArkUI_NumberValue value[] = {{.i32 = autoResize}};
  ArkUI_AttributeItem item = {value, sizeof(value) / sizeof(ArkUI_NumberValue), nullptr, nullptr};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_IMAGE_AUTO_RESIZE, &item));
  return *this;
}

ImageNode &ImageNode::SetAlt(std::string const &src) {
  if (!src.empty()) {
    auto uri = src;
    std::string resourceStr = std::string("resource://RAWFILE/") + "assets/";
    resourceStr += uri.substr(ASSET_PREFIX.size());
    ArkUI_AttributeItem item = {.string = resourceStr.c_str()};
    MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_IMAGE_ALT, &item));
  }
  return *this;
}

ImageNode &ImageNode::ResetFocusable() {
  MaybeThrow(NativeNodeApi::GetInstance()->resetAttribute(nodeHandle_, NODE_FOCUSABLE));
  return *this;
}
ImageNode &ImageNode::ResetResizeMethod() {
  MaybeThrow(NativeNodeApi::GetInstance()->resetAttribute(nodeHandle_, NODE_IMAGE_AUTO_RESIZE));
  return *this;
}

std::string ImageNode::GetUri() { return uri_; }

} // namespace native
} // namespace render
} // namespace hippy
