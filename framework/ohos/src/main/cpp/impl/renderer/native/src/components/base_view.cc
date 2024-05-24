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

#include "renderer/components/base_view.h"
#include "renderer/dom_node/hr_node_props.h"
#include "renderer/utils/hr_value_utils.h"
#include "renderer/utils/hr_convert_utils.h"
#include "renderer/uimanager/hr_gesture_dispatcher.h"

#define HIPPY_COMPONENT_KEY_PREFIX "HippyKey"

namespace hippy {
inline namespace render {
inline namespace native {

BaseView::BaseView(std::shared_ptr<NativeRenderContext> &ctx) : ctx_(ctx), tag_(0) {
  
}

bool BaseView::SetProp(const std::string &propKey, const HippyValue &propValue) {
  if (propKey == HRNodeProps::WIDTH) {
    return true;
  } else if (propKey == HRNodeProps::HEIGHT) {
    return true;
  } else if (propKey == HRNodeProps::VISIBILITY) {
    auto value = HRValueUtils::GetString(propValue);
    GetLocalRootArkUINode().SetVisibility(value != HRNodeProps::HIDDEN ? true : false);
    return true;
  } else if (propKey == HRNodeProps::BACKGROUND_COLOR) {
    uint32_t value = HRValueUtils::GetUint32(propValue);
    GetLocalRootArkUINode().SetBackgroundColor(value);
    return true;
  } else if (propKey == HRNodeProps::OPACITY) {
    auto value = HRValueUtils::GetFloat(propValue, 1.f);
    GetLocalRootArkUINode().SetOpacity(value);
    return true;
  } else if (propKey == HRNodeProps::TRANSFORM) {
    HippyValueArrayType valueArray;
    if (propValue.ToArray(valueArray)) {
      HRTransform transform;
      HRConvertUtils::TransformToArk(valueArray, transform);
      GetLocalRootArkUINode().SetTransform(transform, 1.0f); // TODO(hot):
    }
    return true;
  } else if (propKey == HRNodeProps::OVERFLOW) {
    return true;
  } else if (propKey == HRNodeProps::Z_INDEX) {
    return true;
  } else if (propKey == HRNodeProps::PROP_ACCESSIBILITY_LABEL) {
    return true;
  } else if (propKey == HRNodeProps::FOCUSABLE) {
    return true;
  } else if (propKey == HRNodeProps::REQUEST_FOCUS) {
    return true;
  } else if (propKey == HRNodeProps::LINEAR_GRADIENT) {
    return true;
  } else {
    bool handled = SetBackgroundImageProp(propKey, propValue);
    if (!handled) {
      handled = SetBorderProp(propKey, propValue);
    }
    if (!handled) {
      handled = SetShadowProp(propKey, propValue);
    }
    if (!handled) {
      handled = SetEventProp(propKey, propValue);
    }
    return handled;
  }
}

bool BaseView::SetBackgroundImageProp(const std::string &propKey, const HippyValue &propValue) {
  if (propKey == HRNodeProps::BACKGROUND_IMAGE) {
    return true;
  } else if (propKey == HRNodeProps::BACKGROUND_POSITION_X) {
    return true;
  } else if (propKey == HRNodeProps::BACKGROUND_POSITION_Y) {
    return true;
  } else if (propKey == HRNodeProps::BACKGROUND_SIZE) {
    return true;
  }
  return false;
}

bool BaseView::SetBorderProp(const std::string &propKey, const HippyValue &propValue) {
  if (propKey == HRNodeProps::BORDER_RADIUS) {
    float value = HRValueUtils::GetFloat(propValue);
    borderTopLeftRadius_ = value;
    borderTopRightRadius_ = value;
    borderBottomRightRadius_ = value;
    borderBottomLeftRadius_ = value;
    toSetBorderRadius_ = true;
    return true;
  } else if (propKey == HRNodeProps::BORDER_TOP_LEFT_RADIUS) {
    float value = HRValueUtils::GetFloat(propValue);
    borderTopLeftRadius_ = value;
    toSetBorderRadius_ = true;
    return true;
  } else if (propKey == HRNodeProps::BORDER_TOP_RIGHT_RADIUS) {
    float value = HRValueUtils::GetFloat(propValue);
    borderTopRightRadius_ = value;
    toSetBorderRadius_ = true;
    return true;
  } else if (propKey == HRNodeProps::BORDER_BOTTOM_RIGHT_RADIUS) {
    float value = HRValueUtils::GetFloat(propValue);
    borderBottomRightRadius_ = value;
    toSetBorderRadius_ = true;
    return true;
  } else if (propKey == HRNodeProps::BORDER_BOTTOM_LEFT_RADIUS) {
    float value = HRValueUtils::GetFloat(propValue);
    borderBottomLeftRadius_ = value;
    toSetBorderRadius_ = true;
    return true;
  } else if (propKey == HRNodeProps::BORDER_WIDTH) {
    float value = HRValueUtils::GetFloat(propValue);
    borderTopWidth_ = value;
    borderRightWidth_ = value;
    borderBottomWidth_ = value;
    borderLeftWidth_ = value;
    toSetBorderWidth_ = true;
    return true;
  } else if (propKey == HRNodeProps::BORDER_TOP_WIDTH) {
    float value = HRValueUtils::GetFloat(propValue);
    borderTopWidth_ = value;
    toSetBorderWidth_ = true;
    return true;
  } else if (propKey == HRNodeProps::BORDER_RIGHT_WIDTH) {
    float value = HRValueUtils::GetFloat(propValue);
    borderRightWidth_ = value;
    toSetBorderWidth_ = true;
    return true;
  } else if (propKey == HRNodeProps::BORDER_BOTTOM_WIDTH) {
    float value = HRValueUtils::GetFloat(propValue);
    borderBottomWidth_ = value;
    toSetBorderWidth_ = true;
    return true;
  } else if (propKey == HRNodeProps::BORDER_LEFT_WIDTH) {
    float value = HRValueUtils::GetFloat(propValue);
    borderLeftWidth_ = value;
    toSetBorderWidth_ = true;
    return true;
  } else if (propKey == HRNodeProps::BORDER_STYLE) {
    std::string value = HRValueUtils::GetString(propValue);
    borderTopStyle_ = value;
    borderRightStyle_ = value;
    borderBottomStyle_ = value;
    borderLeftStyle_ = value;
    toSetBorderStyle_ = true;
    return true;
  } else if (propKey == HRNodeProps::BORDER_TOP_STYLE) {
    std::string value = HRValueUtils::GetString(propValue);
    borderTopStyle_ = value;
    toSetBorderStyle_ = true;
    return true;
  } else if (propKey == HRNodeProps::BORDER_RIGHT_STYLE) {
    std::string value = HRValueUtils::GetString(propValue);
    borderRightStyle_ = value;
    toSetBorderStyle_ = true;
    return true;
  } else if (propKey == HRNodeProps::BORDER_BOTTOM_STYLE) {
    std::string value = HRValueUtils::GetString(propValue);
    borderBottomStyle_ = value;
    toSetBorderStyle_ = true;
    return true;
  } else if (propKey == HRNodeProps::BORDER_LEFT_STYLE) {
    std::string value = HRValueUtils::GetString(propValue);
    borderLeftStyle_ = value;
    toSetBorderStyle_ = true;
    return true;
  } else if (propKey == HRNodeProps::BORDER_COLOR) {
    uint32_t value = HRValueUtils::GetUint32(propValue);
    borderTopColor_ = value;
    borderRightColor_ = value;
    borderBottomColor_ = value;
    borderLeftColor_ = value;
    toSetBorderColor_ = true;
    return true;
  } else if (propKey == HRNodeProps::BORDER_TOP_COLOR) {
    uint32_t value = HRValueUtils::GetUint32(propValue);
    borderTopColor_ = value;
    toSetBorderColor_ = true;
    return true;
  } else if (propKey == HRNodeProps::BORDER_RIGHT_COLOR) {
    uint32_t value = HRValueUtils::GetUint32(propValue);
    borderRightColor_ = value;
    toSetBorderColor_ = true;
    return true;
  } else if (propKey == HRNodeProps::BORDER_BOTTOM_COLOR) {
    uint32_t value = HRValueUtils::GetUint32(propValue);
    borderBottomColor_ = value;
    toSetBorderColor_ = true;
    return true;
  } else if (propKey == HRNodeProps::BORDER_LEFT_COLOR) {
    uint32_t value = HRValueUtils::GetUint32(propValue);
    borderLeftColor_ = value;
    toSetBorderColor_ = true;
    return true;
  }
  return false;
}

bool BaseView::SetShadowProp(const std::string &propKey, const HippyValue &propValue) {
  if (propKey == HRNodeProps::SHADOW_OFFSET) {
    return true;
  } else if (propKey == HRNodeProps::SHADOW_OFFSET_X) {
    return true;
  } else if (propKey == HRNodeProps::SHADOW_OFFSET_Y) {
    return true;
  } else if (propKey == HRNodeProps::SHADOW_OPACITY) {
    return true;
  } else if (propKey == HRNodeProps::SHADOW_RADIUS) {
    return true;
  } else if (propKey == HRNodeProps::SHADOW_COLOR) {
    return true;
  }
  return false;
}

#define SET_EVENT_PROP_CASE(keyName, method) \
  if (propKey == keyName) { \
    bool value = false; \
    bool isBool = propValue.ToBoolean(value); \
    if (isBool) { \
      method(value); \
    } \
    return true; \
  }

bool BaseView::SetEventProp(const std::string &propKey, const HippyValue &propValue) {
  SET_EVENT_PROP_CASE("click", SetClickable)
  SET_EVENT_PROP_CASE("longclick", SetLongClickable)
  SET_EVENT_PROP_CASE("pressin", SetPressIn)
  SET_EVENT_PROP_CASE("pressout", SetPressOut)
  SET_EVENT_PROP_CASE("touchstart", SetTouchDownHandle)
  SET_EVENT_PROP_CASE("touchmove", SetTouchMoveHandle)
  SET_EVENT_PROP_CASE("touchend", SetTouchEndHandle)
  SET_EVENT_PROP_CASE("touchcancel", SetTouchCancelHandle)
  SET_EVENT_PROP_CASE("onInterceptTouchEvent", SetInterceptTouch)
  SET_EVENT_PROP_CASE("onInterceptPullUpEvent", SetInterceptPullUp)
  SET_EVENT_PROP_CASE("attachedtowindow", SetAttachedToWindowHandle)
  SET_EVENT_PROP_CASE("detachedfromwindow", SetDetachedFromWindowHandle)
  return false;
}

#undef SET_EVENT_PROP_CASE

void BaseView::SetClickable(bool flag) {
  if (HandleGestureBySelf()) {
    return;
  }
  if (flag) {
    GetLocalRootArkUINode().RegisterClickEvent();
    auto weak_view = weak_from_this();
    eventClick_ = [weak_view]() {
      auto view = weak_view.lock();
      if (view) {
        HRGestureDispatcher::HandleClickEvent(view->ctx_, view->tag_, HRNodeProps::ON_CLICK);
      }
    };
  } else {
    GetLocalRootArkUINode().UnregisterClickEvent();
    eventClick_ = nullptr;
  }
}

void BaseView::SetLongClickable(bool flag) {
  
}

void BaseView::SetPressIn(bool flag) {
  
}

void BaseView::SetPressOut(bool flag) {
  
}

void BaseView::SetTouchDownHandle(bool flag) {
  
}

void BaseView::SetTouchMoveHandle(bool flag) {
  
}

void BaseView::SetTouchEndHandle(bool flag) {
  
}

void BaseView::SetTouchCancelHandle(bool flag) {
  
}

void BaseView::SetInterceptTouch(bool flag) {
  
}

void BaseView::SetInterceptPullUp(bool flag) {
  
}

void BaseView::SetAttachedToWindowHandle(bool flag) {
  
}

void BaseView::SetDetachedFromWindowHandle(bool flag) {
  
}

void BaseView::OnSetPropsEnd() {
  if (toSetBorderRadius_) {
    toSetBorderRadius_ = false;
    GetLocalRootArkUINode().SetBorderRadius(borderTopLeftRadius_, borderTopRightRadius_, borderBottomLeftRadius_, borderBottomRightRadius_);
  }
  if (toSetBorderWidth_) {
    toSetBorderWidth_ = false;
    GetLocalRootArkUINode().SetBorderWidth(borderTopWidth_, borderRightWidth_, borderBottomWidth_, borderLeftWidth_);
  }
  if (toSetBorderStyle_) {
    toSetBorderStyle_ = false;
    ArkUI_BorderStyle topStyle = HRConvertUtils::BorderStyleToArk(borderTopStyle_);
    ArkUI_BorderStyle rightStyle = HRConvertUtils::BorderStyleToArk(borderRightStyle_);
    ArkUI_BorderStyle bottomStyle = HRConvertUtils::BorderStyleToArk(borderBottomStyle_);
    ArkUI_BorderStyle leftStyle = HRConvertUtils::BorderStyleToArk(borderLeftStyle_);
    GetLocalRootArkUINode().SetBorderStyle(topStyle, rightStyle, bottomStyle, leftStyle);
  }
  if (toSetBorderColor_) {
    toSetBorderColor_ = false;
    GetLocalRootArkUINode().SetBorderColor(borderTopColor_, borderRightColor_, borderBottomColor_, borderLeftColor_);
  }
}

void BaseView::AddSubRenderView(std::shared_ptr<BaseView> &subView, int32_t index) {
  if (index < 0 || index > (int32_t)children_.size()) {
    index = (int32_t)children_.size();
  }
  OnChildInserted(subView, index);
  auto it = children_.begin() + index;
  subView->SetParent(shared_from_this());
  children_.insert(it, subView);
}

void BaseView::RemoveSubView(std::shared_ptr<BaseView> &subView) {
  auto it = std::find(children_.begin(), children_.end(), subView);
  if (it != children_.end()) {
    auto view = std::move(*it);
    children_.erase(it);
    OnChildRemoved(view);
  }
}

void BaseView::RemoveFromParentView() {
  auto parentView = parent_.lock();
  if (parentView) {
    auto thisView = shared_from_this();
    parentView->RemoveSubView(thisView);
    SetParent(nullptr);
  }
}

bool BaseView::IsImageSpan() {
  auto parentView = parent_.lock();
  if (parentView && parentView->GetViewType() == "Text" && view_type_ == "Image") {
    return true;
  }
  return false;
}

void BaseView::SetRenderViewFrame(const HRRect &frame, const HRPadding &padding) {
  UpdateRenderViewFrame(frame, padding);
}

void BaseView::UpdateRenderViewFrame(const HRRect &frame, const HRPadding &padding) {
  if (IsImageSpan()) {
    if (frame.x != 0 || frame.y != 0) { // c 测得span的位置
      GetLocalRootArkUINode().SetPosition(HRPosition(frame.x, frame.y));
      return;
    }
  }

  GetLocalRootArkUINode().SetPosition(HRPosition(frame.x, frame.y));
  GetLocalRootArkUINode().SetSize(HRSize(frame.width, frame.height));
}

void BaseView::UpdateEventListener(HippyValueObjectType &newEvents) {
  events_ = newEvents;
}

bool BaseView::CheckRegisteredEvent(std::string &eventName) {
  if (events_.size() > 0 && events_.find(eventName) != events_.end()) {
    auto value = events_[eventName];
    bool boolValue = false;
    bool isBool = value.ToBoolean(boolValue);
    if (isBool) {
      return boolValue;
    }
  }
  return false;
}

} // namespace native
} // namespace render
} // namespace hippy
