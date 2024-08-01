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

#include <arkui/native_node_napi.h>
#include "renderer/components/base_view.h"
#include "oh_napi/ark_ts.h"
#include "oh_napi/oh_napi_object.h"
#include "oh_napi/oh_napi_object_builder.h"
#include "oh_napi/oh_napi_utils.h"
#include "renderer/dom_node/hr_node_props.h"
#include "renderer/native_render_params.h"
#include "renderer/utils/hr_pixel_utils.h"
#include "renderer/utils/hr_url_utils.h"
#include "renderer/utils/hr_value_utils.h"
#include "renderer/utils/hr_convert_utils.h"
#include "renderer/uimanager/hr_gesture_dispatcher.h"

#define HIPPY_COMPONENT_KEY_PREFIX "HippyKey"

namespace hippy {
inline namespace render {
inline namespace native {

std::shared_ptr<footstone::value::Serializer> BaseView::serializer_ = nullptr;

BaseView::BaseView(std::shared_ptr<NativeRenderContext> &ctx) : ctx_(ctx), tag_(0) {
#if HIPPY_OHOS_MEM_CHECK
  static int sCount = 0;
  ++sCount;
  FOOTSTONE_DLOG(INFO) << "Hippy ohos mem check, view, new: " << this << ", type: " << view_type_ << ", count: " << sCount;
#endif
}

BaseView::~BaseView() {
#if HIPPY_OHOS_MEM_CHECK
  static int sCount = 0;
  ++sCount;
  FOOTSTONE_DLOG(INFO) << "Hippy ohos mem check, view, del: " << this << ", type: " << view_type_ << ", count: " << sCount;
#endif
}

void BaseView::Init() {
  GetLocalRootArkUINode().SetArkUINodeDelegate(this);
}

void BaseView::SetTag(uint32_t tag) {
  tag_ = tag;
  std::string id_str = "HippyId" + std::to_string(tag);
  GetLocalRootArkUINode().SetId(id_str);
}

bool BaseView::SetProp(const std::string &propKey, const HippyValue &propValue) {
  if (propKey == HRNodeProps::VISIBILITY) {
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
    if (propValue.IsArray() && propValue.ToArray(valueArray)) {
      HRTransform transform;
      HRConvertUtils::TransformToArk(valueArray, transform);
      GetLocalRootArkUINode().SetTransform(transform, 1.0f); // TODO(hot):
    }
    return true;
  } else if (propKey == HRNodeProps::OVERFLOW) {
    auto value = HRValueUtils::GetString(propValue);
    bool clip = value != HRNodeProps::HIDDEN ? false : true;
    GetLocalRootArkUINode().SetClip(clip);
    return true;
  } else if (propKey == HRNodeProps::Z_INDEX) {
    auto value = HRValueUtils::GetInt32(propValue);
    GetLocalRootArkUINode().SetZIndex(value);
    return true;
  } else if (propKey == HRNodeProps::PROP_ACCESSIBILITY_LABEL) {
    auto value = HRValueUtils::GetString(propValue);
    GetLocalRootArkUINode().SetAccessibilityText(value);
    return true;
  } else if (propKey == HRNodeProps::FOCUSABLE) {
    auto value = HRValueUtils::GetBool(propValue, false);
    GetLocalRootArkUINode().SetFocusable(value);
    return true;
  } else if (propKey == HRNodeProps::REQUEST_FOCUS) {
    auto value = HRValueUtils::GetBool(propValue, false);
    if (value) {
      GetLocalRootArkUINode().SetFocusStatus(1);
    }
    return true;
  } else if (propKey == HRNodeProps::LINEAR_GRADIENT) {
    SetLinearGradientProp(propKey, propValue);
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

bool BaseView::SetLinearGradientProp(const std::string &propKey, const HippyValue &propValue) {
  HippyValueObjectType m;
  if (!propValue.ToObject(m)) {
    return false;
  }
  
  auto angleIt = m.find("angle");
  if (angleIt == m.end()) {
    return false;
  }
  std::string angle;
  if (!angleIt->second.ToString(angle)) {
    return false;
  }
  if (angle.length() == 0) {
    return false;
  }

  auto colorStopListIt = m.find("colorStopList");
  if (colorStopListIt == m.end()) {
    return false;
  }
  HippyValueArrayType colorStopList;
  if (!colorStopListIt->second.IsArray() || !colorStopListIt->second.ToArray(colorStopList)) {
    return false;
  }
  if (colorStopList.size() == 0) {
    return false;
  }
  
  HRLinearGradient linearGradient;

  auto size = colorStopList.size();
  for (uint32_t i = 0; i < size; i++) {
    HippyValueObjectType colorStop;
    if (!colorStopList[i].ToObject(colorStop)) {
      continue;
    }
    auto color = HRValueUtils::GetUint32(colorStop["color"]);
    float ratio = 0.f;
    if (colorStop.find("ratio") != colorStop.end()) {
      ratio = HRValueUtils::GetFloat(colorStop["ratio"]);
    } else if (i == size - 1) {
      ratio = 1.f;
    }
    linearGradient.colors.push_back(color);
    linearGradient.stops.push_back(ratio);
  }
  
  if (angle == "totopright") {
    linearGradient.direction = ARKUI_LINEAR_GRADIENT_DIRECTION_RIGHT_TOP;
  } else if (angle == "tobottomright") {
    linearGradient.direction = ARKUI_LINEAR_GRADIENT_DIRECTION_RIGHT_BOTTOM;
  } else if (angle == "tobottomleft") {
    linearGradient.direction = ARKUI_LINEAR_GRADIENT_DIRECTION_LEFT_BOTTOM;
  } else if (angle == "totopleft") {
    linearGradient.direction = ARKUI_LINEAR_GRADIENT_DIRECTION_LEFT_TOP;
  } else {
    uint32_t value = static_cast<uint32_t>(std::stof(angle)) % 360;
    linearGradient.angle = value;
  }
  
  GetLocalRootArkUINode().SetLinearGradient(linearGradient);

  return true;
}

bool BaseView::SetBackgroundImageProp(const std::string &propKey, const HippyValue &propValue) {
  if (propKey == HRNodeProps::BACKGROUND_IMAGE) {
    std::string value;
    if (propValue.ToString(value)) {
      GetLocalRootArkUINode().SetBackgroundImage(ConvertToLocalPathIfNeeded(value));
    }
    return true;
  } else if (propKey == HRNodeProps::BACKGROUND_POSITION_X) {
    backgroundImagePosition_.x = HRValueUtils::GetFloat(propValue);
    toSetBackgroundImagePosition_ = true;
    return true;
  } else if (propKey == HRNodeProps::BACKGROUND_POSITION_Y) {
    backgroundImagePosition_.y = HRValueUtils::GetFloat(propValue);
    toSetBackgroundImagePosition_ = true;
    return true;
  } else if (propKey == HRNodeProps::BACKGROUND_SIZE) {
    auto value = HRValueUtils::GetString(propValue);
    auto imageSize = HRConvertUtils::BackgroundImageSizeToArk(value);
    GetLocalRootArkUINode().SetBackgroundImageSize(imageSize);
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
    HippyValueObjectType m;
    if (propValue.ToObject(m)) {
      auto x = HRPixelUtils::DpToPx(HRValueUtils::GetFloat(m["x"]));
      auto y = HRPixelUtils::DpToPx(HRValueUtils::GetFloat(m["y"]));
      shadow_.shadowOffset.width = x;
      shadow_.shadowOffset.height = y;
    }
    toSetShadow = true;
    return true;
  } else if (propKey == HRNodeProps::SHADOW_OFFSET_X) {
    shadow_.shadowOffset.width = HRPixelUtils::DpToPx(HRValueUtils::GetFloat(propValue));
    toSetShadow = true;
    return true;
  } else if (propKey == HRNodeProps::SHADOW_OFFSET_Y) {
    shadow_.shadowOffset.height = HRPixelUtils::DpToPx(HRValueUtils::GetFloat(propValue));
    toSetShadow = true;
    return true;
  } else if (propKey == HRNodeProps::SHADOW_OPACITY) {
    shadow_.shadowOpacity = HRValueUtils::GetFloat(propValue);
    toSetShadow = true;
    return true;
  } else if (propKey == HRNodeProps::SHADOW_RADIUS) {
    shadow_.shadowRadius = HRPixelUtils::DpToPx(HRValueUtils::GetFloat(propValue));
    toSetShadow = true;
    return true;
  } else if (propKey == HRNodeProps::SHADOW_COLOR) {
    shadow_.shadowColor = HRValueUtils::GetUint32(propValue);
    toSetShadow = true;
    return true;
  } else if (propKey == HRNodeProps::SHADOW_SPREAD) {
    // ohos not support
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
  if (HandleGestureBySelf()) {
    return;
  }
  if (flag) {
    auto weak_view = weak_from_this();
    eventLongPress_ = [weak_view]() {
      auto view = weak_view.lock();
      if (view) {
        HRGestureDispatcher::HandleClickEvent(view->ctx_, view->tag_, HRNodeProps::ON_LONG_CLICK);
      }
    };
  } else {
    eventLongPress_ = nullptr;
  }
}

void BaseView::SetPressIn(bool flag) {
  if (HandleGestureBySelf()) {
    return;
  }
  if (flag) {
    auto weak_view = weak_from_this();
    eventPressIn_ = [weak_view]() {
      auto view = weak_view.lock();
      if (view) {
        HRGestureDispatcher::HandleClickEvent(view->ctx_, view->tag_, HRNodeProps::ON_PRESS_IN);
      }
    };
  } else {
    eventPressIn_ = nullptr;
  }
}

void BaseView::SetPressOut(bool flag) {
  if (HandleGestureBySelf()) {
    return;
  }
  if (flag) {
    auto weak_view = weak_from_this();
    eventPressOut_ = [weak_view]() {
      auto view = weak_view.lock();
      if (view) {
        HRGestureDispatcher::HandleClickEvent(view->ctx_, view->tag_, HRNodeProps::ON_PRESS_OUT);
      }
    };
  } else {
    eventPressOut_ = nullptr;
  }
}

void BaseView::SetTouchDownHandle(bool flag) {
  if (HandleGestureBySelf()) {
    return;
  }
  if (flag) {
    auto weak_view = weak_from_this();
    eventTouchDown_ = [weak_view]() {
      auto view = weak_view.lock();
      if (view) {
        float touchX = 0; // TODO(hot):
        float touchY = 0;
        HRGestureDispatcher::HandleTouchEvent(view->ctx_, view->tag_, touchX, touchY, HRNodeProps::ON_TOUCH_DOWN);
      }
    };
  } else {
    eventTouchDown_ = nullptr;
  }
}

void BaseView::SetTouchMoveHandle(bool flag) {
  if (HandleGestureBySelf()) {
    return;
  }
  if (flag) {
    auto weak_view = weak_from_this();
    eventTouchMove_ = [weak_view]() {
      auto view = weak_view.lock();
      if (view) {
        float touchX = 0; // TODO(hot):
        float touchY = 0;
        HRGestureDispatcher::HandleTouchEvent(view->ctx_, view->tag_, touchX, touchY, HRNodeProps::ON_TOUCH_MOVE);
      }
    };
  } else {
    eventTouchMove_ = nullptr;
  }
}

void BaseView::SetTouchEndHandle(bool flag) {
  if (HandleGestureBySelf()) {
    return;
  }
  if (flag) {
    auto weak_view = weak_from_this();
    eventTouchUp_ = [weak_view]() {
      auto view = weak_view.lock();
      if (view) {
        float touchX = 0; // TODO(hot):
        float touchY = 0;
        HRGestureDispatcher::HandleTouchEvent(view->ctx_, view->tag_, touchX, touchY, HRNodeProps::ON_TOUCH_END);
      }
    };
  } else {
    eventTouchUp_ = nullptr;
  }
}

void BaseView::SetTouchCancelHandle(bool flag) {
  if (HandleGestureBySelf()) {
    return;
  }
  if (flag) {
    auto weak_view = weak_from_this();
    eventTouchCancel_ = [weak_view]() {
      auto view = weak_view.lock();
      if (view) {
        float touchX = 0; // TODO(hot):
        float touchY = 0;
        HRGestureDispatcher::HandleTouchEvent(view->ctx_, view->tag_, touchX, touchY, HRNodeProps::ON_TOUCH_CANCEL);
      }
    };
  } else {
    eventTouchCancel_ = nullptr;
  }
}

void BaseView::SetInterceptTouch(bool flag) {
  if (HandleGestureBySelf()) {
    return;
  }
  GetLocalRootArkUINode().SetHitTestMode(flag ? ARKUI_HIT_TEST_MODE_BLOCK : ARKUI_HIT_TEST_MODE_DEFAULT);
}

void BaseView::SetInterceptPullUp(bool flag) {
  if (HandleGestureBySelf()) {
  return;
  }
  flagInterceptPullUp_ = flag;
}

void BaseView::HandleInterceptPullUp() {
  // TODO(hot);
}

void BaseView::SetAttachedToWindowHandle(bool flag) {
  if (flag) {
    auto weak_view = weak_from_this();
    eventAttachedToWindow_ = [weak_view]() {
      auto view = weak_view.lock();
      if (view) {
        HRGestureDispatcher::HandleAttachedToWindow(view->ctx_, view->tag_);
      }
    };
  } else {
    eventAttachedToWindow_ = nullptr;
  }
}

void BaseView::SetDetachedFromWindowHandle(bool flag) {
  if (flag) {
    auto weak_view = weak_from_this();
    eventDetachedFromWindow_ = [weak_view]() {
      auto view = weak_view.lock();
      if (view) {
        HRGestureDispatcher::HandleDetachedFromWindow(view->ctx_, view->tag_);
      }
    };
  } else {
    eventDetachedFromWindow_ = nullptr;
  }
}

void BaseView::OnSetPropsEnd() {
  if (toSetBackgroundImagePosition_) {
    toSetBackgroundImagePosition_ = false;
    GetLocalRootArkUINode().SetBackgroundImagePosition(backgroundImagePosition_);
  }
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
  if (toSetShadow) {
    toSetShadow = false;
    GetLocalRootArkUINode().SetShadow(shadow_);
  }
}

void BaseView::Call(const std::string &method, const std::vector<HippyValue> params,
                    std::function<void(const HippyValue &result)> callback) {
  FOOTSTONE_DLOG(INFO) << "BaseView call: method " << method << ", params: " << params.size();
  if (method == "measureInWindow") {
    if (!callback) {
      return;
    }
    
    float statusBarHeight = NativeRenderParams::StatusBarHeight();
    HRPosition viewPosition = GetLocalRootArkUINode().GetLayoutPositionInScreen();
    HRSize viewSize = GetLocalRootArkUINode().GetSize();
    
    HippyValueObjectType result;
    result["x"] = HippyValue(viewPosition.x);
    result["y"] = HippyValue(viewPosition.y - statusBarHeight);
    result["width"] = HippyValue(viewSize.width);
    result["height"] = HippyValue(viewSize.height);
    result["statusBarHeight"] = HippyValue(statusBarHeight);
    callback(HippyValue(result));
  } else if (method == "getBoundingClientRect") {
    if (!callback) {
      return;
    }
    
    bool relToContainer = false;
    if (!params.empty()) {
      HippyValueObjectType param;
      if (params[0].IsObject() && params[0].ToObject(param)) {
        relToContainer = HRValueUtils::GetBool(param["relToContainer"], false);
      }
    }
    float x = 0;
    float y = 0;
    HRSize viewSize = GetLocalRootArkUINode().GetSize();
    if (relToContainer) {
      HRPosition viewPosition = GetLocalRootArkUINode().GetLayoutPositionInWindow();
      x = viewPosition.x;
      y = viewPosition.y;
      auto render = ctx_->GetNativeRender().lock();
      if (render) {
        HRPosition rootViewPosition = render->GetRootViewtPositionInWindow(ctx_->GetRootId());
        x -= rootViewPosition.x;
        y -= rootViewPosition.y;
      }
    } else {
      HRPosition viewPosition = GetLocalRootArkUINode().GetLayoutPositionInScreen();
      x = viewPosition.x;
      y = viewPosition.y;
    }
        
    HippyValueObjectType result;
    result["x"] = HippyValue(x);
    result["y"] = HippyValue(y);
    result["width"] = HippyValue(viewSize.width);
    result["height"] = HippyValue(viewSize.height);
    callback(HippyValue(result));
  }
}

void BaseView::AddSubRenderView(std::shared_ptr<BaseView> &subView, int32_t index) {
  if (index < 0 || index > (int32_t)children_.size()) {
    index = (int32_t)children_.size();
  }
  auto it = children_.begin() + index;
  subView->SetParent(shared_from_this());
  children_.insert(it, subView);
  OnChildInserted(subView, index);
}

void BaseView::RemoveSubView(std::shared_ptr<BaseView> &subView) {
  auto it = std::find(children_.begin(), children_.end(), subView);
  if (it != children_.end()) {
    auto view = std::move(*it);
    int32_t index = static_cast<int32_t>(it - children_.begin());
    children_.erase(it);
    OnChildRemoved(view, index);
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

void BaseView::SetRenderViewFrame(const HRRect &frame, const HRPadding &padding) {
  UpdateRenderViewFrame(frame, padding);
}

void BaseView::UpdateRenderViewFrame(const HRRect &frame, const HRPadding &padding) {
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

void BaseView::SetTsRenderProvider(napi_env ts_env, napi_ref ts_render_provider_ref) {
  ts_env_ = ts_env;
  ts_render_provider_ref_ = ts_render_provider_ref;
}

void BaseView::SetTsEventCallback(napi_ref ts_event_callback_ref) {
  ts_event_callback_ref_ = ts_event_callback_ref;
}

void BaseView::OnClick() {
  if (eventClick_) {
    eventClick_();
  }
}

void BaseView::OnAppear() {
  if (eventAttachedToWindow_) {
    eventAttachedToWindow_();
  }
}

void BaseView::OnDisappear() {
  if (eventDetachedFromWindow_) {
    eventDetachedFromWindow_();
  }
}

void BaseView::OnAreaChange(ArkUI_NumberValue* data) {
  
}

// TODO(hot):
std::string BaseView::ConvertToLocalPathIfNeeded(const std::string &uri) {
  // hpfile://./assets/defaultSource.jpg
  if (uri.find("hpfile://") == 0) {
    std::string prefix = "hpfile://./";
    auto pos = uri.find(prefix);
    if (pos == 0) {
      auto relativePath = uri.substr(prefix.length());
      auto bundlePath = ctx_->GetNativeRender().lock()->GetBundlePath();
      auto lastPos = bundlePath.rfind("/");
      if (lastPos != std::string::npos) {
        bundlePath = bundlePath.substr(0, lastPos + 1);
      }
      auto fullPath = bundlePath + relativePath;
      auto localPath = HRUrlUtils::convertAssetImageUrl(fullPath);
      return localPath;
    }
  } else if (uri.find("asset:/") == 0) {
    auto localPath = HRUrlUtils::convertAssetImageUrl(uri);
    return localPath;
  }
  return uri;
}

int64_t BaseView::GetTimeMilliSeconds() {
  auto now = std::chrono::system_clock::now();
  auto duration = now.time_since_epoch();
  auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
  return millis;
}

std::shared_ptr<footstone::value::Serializer> &BaseView::GetSerializer() {
  if (!serializer_) {
    serializer_ = std::make_shared<footstone::value::Serializer>();
  }
  return serializer_;
}

void BaseView::OnViewComponentEvent(const std::string &event_name, const HippyValueObjectType &hippy_object) {
  if (!ts_event_callback_ref_) {
    return;
  }
  
  ArkTS arkTs(ts_env_);
  auto ts_params = OhNapiUtils::HippyValue2NapiValue(ts_env_, HippyValue(hippy_object));
  
  std::vector<napi_value> args = {
    arkTs.CreateString(event_name),
    ts_params
  };
  
  auto callback = arkTs.GetReferenceValue(ts_event_callback_ref_);
  arkTs.Call(callback, args);
}

} // namespace native
} // namespace render
} // namespace hippy
