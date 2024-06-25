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

#include "renderer/arkui/arkui_node.h"
#include <algorithm>
#include "renderer/arkui/arkui_node_registry.h"
#include "renderer/arkui/native_node_api.h"
#include "renderer/utils/hr_convert_utils.h"

namespace hippy {
inline namespace render {
inline namespace native {

ArkUINode::ArkUINode(ArkUI_NodeHandle nodeHandle) : nodeHandle_(nodeHandle) {
#if HIPPY_OHOS_MEM_CHECK
  static int sCount = 0;
  ++sCount;
  FOOTSTONE_DLOG(INFO) << "Hippy ohos mem check, ArkUINode handle, new: " << nodeHandle_ << ", count: " << sCount;
#endif
  
  ArkUINodeRegistry::GetInstance().RegisterNode(this);
}

ArkUINode::~ArkUINode() {
#if HIPPY_OHOS_MEM_CHECK
  static int sCount = 0;
  ++sCount;
  FOOTSTONE_DLOG(INFO) << "Hippy ohos mem check, ArkUINode handle, del: " << nodeHandle_ << ", count: " << sCount;
#endif
  
  if (nodeHandle_ != nullptr) {
    UnregisterClickEvent();
    ArkUINodeRegistry::GetInstance().UnregisterNode(this);
    NativeNodeApi::GetInstance()->disposeNode(nodeHandle_);
  }
}

ArkUINode::ArkUINode(ArkUINode &&other) noexcept : nodeHandle_(std::move(other.nodeHandle_)) {
  other.nodeHandle_ = nullptr;
}

ArkUINode &ArkUINode::operator=(ArkUINode &&other) noexcept {
  std::swap(nodeHandle_, other.nodeHandle_);
  return *this;
}

ArkUI_NodeHandle ArkUINode::GetArkUINodeHandle() { return nodeHandle_; }

void ArkUINode::MarkDirty() {
  NativeNodeApi::GetInstance()->markDirty(GetArkUINodeHandle(), ArkUI_NodeDirtyFlag::NODE_NEED_RENDER);
  NativeNodeApi::GetInstance()->markDirty(GetArkUINodeHandle(), ArkUI_NodeDirtyFlag::NODE_NEED_LAYOUT);
  NativeNodeApi::GetInstance()->markDirty(GetArkUINodeHandle(), ArkUI_NodeDirtyFlag::NODE_NEED_MEASURE);
}

void ArkUINode::AddChild(ArkUINode &child) {
  MaybeThrow(NativeNodeApi::GetInstance()->addChild(nodeHandle_, child.GetArkUINodeHandle()));
}

void ArkUINode::InsertChild(ArkUINode &child, int32_t index) {
  MaybeThrow(
    NativeNodeApi::GetInstance()->insertChildAt(nodeHandle_, child.GetArkUINodeHandle(), static_cast<int32_t>(index)));
}

void ArkUINode::RemoveChild(ArkUINode &child) {
  MaybeThrow(NativeNodeApi::GetInstance()->removeChild(nodeHandle_, child.GetArkUINodeHandle()));
}

ArkUINode &ArkUINode::SetPosition(const HRPosition &position) {
  ArkUI_NumberValue value[] = {{position.x}, {position.y}};
  ArkUI_AttributeItem item = {value, sizeof(value) / sizeof(ArkUI_NumberValue), nullptr, nullptr};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_POSITION, &item));
  return *this;
}

ArkUINode &ArkUINode::SetSize(const HRSize &size) {
  ArkUI_NumberValue widthValue[] = {{size.width}};
  ArkUI_AttributeItem widthItem = {widthValue, sizeof(widthValue) / sizeof(ArkUI_NumberValue), nullptr, nullptr};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_WIDTH, &widthItem));

  ArkUI_NumberValue heightValue[] = {{size.height}};
  ArkUI_AttributeItem heightItem = {heightValue, sizeof(heightValue) / sizeof(ArkUI_NumberValue), nullptr, nullptr};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_HEIGHT, &heightItem));
  return *this;
}
 
ArkUINode &ArkUINode::SetWidth(float width) {
  ArkUI_NumberValue widthValue[] = {{width}};
  ArkUI_AttributeItem widthItem = {widthValue, sizeof(widthValue) / sizeof(ArkUI_NumberValue), nullptr, nullptr};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_WIDTH, &widthItem));
  return *this;
}

ArkUINode &ArkUINode::SetHeight(float height) {
  ArkUI_NumberValue heightValue[] = {{height}};
  ArkUI_AttributeItem heightItem = {heightValue, sizeof(heightValue) / sizeof(ArkUI_NumberValue), nullptr, nullptr};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_HEIGHT, &heightItem));
  return *this;
}

HRSize ArkUINode::GetSize() const {
  float width = 0.0;
  float height = 0.0;
  auto widthValue = NativeNodeApi::GetInstance()->getAttribute(nodeHandle_, NODE_WIDTH);
  if (widthValue) {
    width = widthValue->value->f32;
  }
  auto heightValue = NativeNodeApi::GetInstance()->getAttribute(nodeHandle_, NODE_HEIGHT);
  if (heightValue) {
    height = heightValue->value->f32;
  }
  return HRSize{width, height};
}

uint32_t ArkUINode::GetTotalChildCount() const {
  return NativeNodeApi::GetInstance()->getTotalChildCount(nodeHandle_);
}

ArkUINode &ArkUINode::SetSizePercent(const HRSize &size) {
  ArkUI_NumberValue widthValue[] = {{size.width}};
  ArkUI_AttributeItem widthItem = {widthValue, sizeof(widthValue) / sizeof(ArkUI_NumberValue), nullptr, nullptr};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_WIDTH_PERCENT, &widthItem));

  ArkUI_NumberValue heightValue[] = {{size.height}};
  ArkUI_AttributeItem heightItem = {heightValue, sizeof(heightValue) / sizeof(ArkUI_NumberValue), nullptr, nullptr};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_HEIGHT_PERCENT, &heightItem));
  return *this;
}

ArkUINode &ArkUINode::SetWidthPercent(float percent) {
  ArkUI_NumberValue value[] = {{.f32 = percent}};
  ArkUI_AttributeItem item = {value, 1, nullptr, nullptr};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_WIDTH_PERCENT, &item));
  return *this;
}

ArkUINode &ArkUINode::SetHeightPercent(float percent) {
  ArkUI_NumberValue value[] = {{.f32 = percent}};
  ArkUI_AttributeItem item = {value, 1, nullptr, nullptr};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_HEIGHT_PERCENT, &item));
  return *this;
}

ArkUINode &ArkUINode::SetVisibility(bool visibility) {
  ArkUI_NumberValue value[] = {{.i32 = visibility ? ARKUI_VISIBILITY_VISIBLE : ARKUI_VISIBILITY_HIDDEN}};
  ArkUI_AttributeItem item = {value, sizeof(value) / sizeof(value), nullptr, nullptr};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_VISIBILITY, &item));
  return *this;
}

ArkUINode &ArkUINode::SetBackgroundColor(uint32_t color) {
  ArkUI_NumberValue preparedColorValue[] = {{.u32 = color}};
  ArkUI_AttributeItem colorItem = {preparedColorValue, sizeof(preparedColorValue) / sizeof(ArkUI_NumberValue), nullptr,
                                   nullptr};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_BACKGROUND_COLOR, &colorItem));
  return *this;
}

ArkUINode &ArkUINode::SetTransform(const HRTransform &transform, float pointScaleFactor) {
  if (transform.rotate.has_value()) {
    SetRotate(transform.rotate.value());
  }
  if (transform.scale.has_value()) {
    SetScale(transform.scale.value());
  }
  if (transform.translate.has_value()) {
    SetTranslate(transform.translate.value(), pointScaleFactor);
  }
  if (transform.matrix.has_value()) {
    SetMatrix(transform.matrix.value(), pointScaleFactor);
  }
  return *this;
}

ArkUINode &ArkUINode::SetOpacity(float opacity) {
  ArkUI_NumberValue opacityValue[] = {{.f32 = (float)opacity}};
  ArkUI_AttributeItem opacityItem = {opacityValue, sizeof(opacityValue) / sizeof(ArkUI_NumberValue), nullptr, nullptr};

  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_OPACITY, &opacityItem));
  return *this;
}

ArkUINode &ArkUINode::SetMatrix(const HRMatrix &transformMatrix, float pointScaleFactor) {
  ArkUI_NumberValue transformCenterValue[] = {{0}, {0}, {0}, {0.5f}, {0.5f}};
  ArkUI_AttributeItem transformCenterItem = {transformCenterValue,
                                             sizeof(transformCenterValue) / sizeof(ArkUI_NumberValue), nullptr, nullptr};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_TRANSFORM_CENTER, &transformCenterItem));

  // NOTE: ArkUI translation is in `px` units
  auto matrix = transformMatrix.m;
  matrix[12] *= pointScaleFactor;
  matrix[13] *= pointScaleFactor;
  matrix[14] *= pointScaleFactor;

  std::array<ArkUI_NumberValue, 16> transformValue;
  for (uint32_t i = 0; i < 16; i++) {
    transformValue[i] = {.f32 = static_cast<float>(matrix[i])};
  }

  ArkUI_AttributeItem transformItem = {transformValue.data(), transformValue.size(), nullptr, nullptr};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_TRANSFORM, &transformItem));
  return *this;
}

ArkUINode &ArkUINode::SetRotate(const HRRotate &rotate) {
  ArkUI_NumberValue value[] = {{.f32 = rotate.x}, {.f32 = rotate.y}, {.f32 = rotate.z}, {.f32 = rotate.angle}, {.f32 = rotate.perspective}};
  ArkUI_AttributeItem item = {value, sizeof(value) / sizeof(ArkUI_NumberValue), nullptr, nullptr};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_ROTATE, &item));
  return *this;
}

ArkUINode &ArkUINode::SetScale(const HRScale &scale) {
  ArkUI_NumberValue value[] = {{.f32 = scale.x}, {.f32 = scale.y}};
  ArkUI_AttributeItem item = {value, sizeof(value) / sizeof(ArkUI_NumberValue), nullptr, nullptr};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_SCALE, &item));
  return *this;
}

ArkUINode &ArkUINode::SetTranslate(const HRTranslate &translate, float pointScaleFactor) {
  ArkUI_NumberValue value[] = {{.f32 = translate.x * pointScaleFactor},
                               {.f32 = translate.y * pointScaleFactor},
                               {.f32 = translate.z * pointScaleFactor}};
  ArkUI_AttributeItem item = {value, sizeof(value) / sizeof(ArkUI_NumberValue), nullptr, nullptr};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_TRANSLATE, &item));
  return *this;
}

ArkUINode &ArkUINode::SetClip(bool clip) {
  uint32_t isClip = static_cast<uint32_t>(clip);
  ArkUI_NumberValue clipValue[] = {{.u32 = isClip}};
  ArkUI_AttributeItem clipItem = {clipValue, sizeof(clipValue) / sizeof(ArkUI_NumberValue), nullptr, nullptr};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_CLIP, &clipItem));
  return *this;
}

ArkUINode &ArkUINode::SetZIndex(int32_t zIndex) {
  ArkUI_NumberValue value[] = {{.f32 = (float)zIndex}};
  ArkUI_AttributeItem item = {value, sizeof(value) / sizeof(ArkUI_NumberValue), nullptr, nullptr};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_Z_INDEX, &item));
  return *this;
}

ArkUINode &ArkUINode::SetAccessibilityText(const std::string &accessibilityLabel) {
  ArkUI_AttributeItem textItem = {.string = accessibilityLabel.c_str()};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_ACCESSIBILITY_TEXT, &textItem));
  return *this;
}

ArkUINode &ArkUINode::SetFocusable(bool focusable) {
  ArkUI_NumberValue value[] = {{.i32 = focusable ? 1 : 0}};
  ArkUI_AttributeItem item = {value, sizeof(value) / sizeof(ArkUI_NumberValue), nullptr, nullptr};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_FOCUSABLE, &item));
  return *this;
}

ArkUINode &ArkUINode::SetFocusStatus(int32_t focus) {
  ArkUI_NumberValue value[] = {{.i32 = focus}};
  ArkUI_AttributeItem item = {value, sizeof(value) / sizeof(ArkUI_NumberValue), nullptr, nullptr};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_FOCUS_STATUS, &item));
  return *this;
}

ArkUINode &ArkUINode::SetLinearGradient(const HRLinearGradient &linearGradient) {
  ArkUI_NumberValue value[] = {
    {.f32 = linearGradient.angle.has_value() ? linearGradient.angle.value() : NAN},
    {.i32 = linearGradient.direction.has_value() ? linearGradient.direction.value()
                                                 : ARKUI_LINEAR_GRADIENT_DIRECTION_CUSTOM},
    {.i32 = linearGradient.repeating.has_value() ? linearGradient.repeating.value() : false}};
  ArkUI_ColorStop colorStop = {.colors = linearGradient.colors.data(),
                               .stops = (float *)(linearGradient.stops.data()),
                               .size = (int)linearGradient.colors.size()};
  ArkUI_AttributeItem item = {value, sizeof(value) / sizeof(ArkUI_NumberValue), nullptr, &colorStop};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_LINEAR_GRADIENT, &item));
  return *this;
}

ArkUINode &ArkUINode::SetId(const int32_t &tag) {
  std::string tmpTag = std::to_string(tag);
  ArkUI_AttributeItem idItem = {.string = tmpTag.c_str()};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_ID, &idItem));
  return *this;
}

ArkUINode &ArkUINode::SetHitTestMode(const ArkUIHitTestMode mode) {
  ArkUI_NumberValue hitTestModeValue[] = {{.i32 = static_cast<int32_t>(mode)}};
  ArkUI_AttributeItem hitTestModeItem = {.value = hitTestModeValue,
                                         .size = sizeof(hitTestModeValue) / sizeof(ArkUI_NumberValue)};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_HIT_TEST_BEHAVIOR, &hitTestModeItem));
  return *this;
}

ArkUINode &ArkUINode::SetEnabled(bool enabled) {
  ArkUI_NumberValue value = {.i32 = int32_t(enabled)};
  ArkUI_AttributeItem item = {&value, 1, nullptr, nullptr};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_ENABLED, &item));
  return *this;
}

ArkUINode &ArkUINode::SetBackgroundImage(const std::string &uri) {
  ArkUI_NumberValue value[] = {{.i32 = ARKUI_IMAGE_REPEAT_NONE}};
  ArkUI_AttributeItem item = {value, sizeof(value) / sizeof(ArkUI_NumberValue), uri.c_str(), nullptr};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_BACKGROUND_IMAGE, &item));
  return *this;
}

ArkUINode &ArkUINode::SetBackgroundImagePosition(const HRPosition &position) {
  ArkUI_NumberValue value[] = {{.f32 = position.x}, {.f32 = position.y}};
  ArkUI_AttributeItem item = {value, sizeof(value) / sizeof(ArkUI_NumberValue), nullptr, nullptr};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_BACKGROUND_IMAGE_POSITION, &item));
  return *this;
}

ArkUINode &ArkUINode::SetBackgroundImageSize(const ArkUI_ImageSize sizeStyle) {
  ArkUI_NumberValue value[] = {{.i32 = sizeStyle}};
  ArkUI_AttributeItem item = {value, sizeof(value) / sizeof(ArkUI_NumberValue), nullptr, nullptr};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_BACKGROUND_IMAGE_SIZE_WITH_STYLE, &item));
  return *this;
}

ArkUINode &ArkUINode::SetBorderWidth(float top, float right, float bottom, float left) {
  top = std::max(top, 0.0f);
  right = std::max(right, 0.0f);
  bottom = std::max(bottom, 0.0f);
  left = std::max(left, 0.0f);
  ArkUI_NumberValue borderWidthValue[] = {{top}, {right}, {bottom}, {left}};
  ArkUI_AttributeItem borderWidthItem = {borderWidthValue, sizeof(borderWidthValue) / sizeof(ArkUI_NumberValue), nullptr, nullptr};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_BORDER_WIDTH, &borderWidthItem));
  return *this;
}

ArkUINode &ArkUINode::SetBorderColor(uint32_t top, uint32_t right, uint32_t bottom, uint32_t left) {
  uint32_t borderTopColor = 0xff000000;
  uint32_t bordeRightColor = 0xff000000;
  uint32_t borderBottomColor = 0xff000000;
  uint32_t borderLeftColor = 0xff000000;
  if (top) {
    borderTopColor = top;
  }
  if (right) {
    bordeRightColor = right;
  }
  if (bottom) {
    borderBottomColor = bottom;
  }
  if (left) {
    borderLeftColor = left;
  }
  ArkUI_NumberValue borderColorValue[] = {
    {.u32 = borderTopColor}, {.u32 = bordeRightColor}, {.u32 = borderBottomColor}, {.u32 = borderLeftColor}};
  ArkUI_AttributeItem borderColorItem = {borderColorValue, sizeof(borderColorValue) / sizeof(ArkUI_NumberValue), nullptr, nullptr};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_BORDER_COLOR, &borderColorItem));
  return *this;
}

ArkUINode &ArkUINode::SetBorderRadius(float topLeft, float topRight, float bottomLeft, float bottomRight) {
  ArkUI_NumberValue borderRadiusValue[] = {
    {topLeft}, {topRight},
    {bottomLeft}, {bottomRight}
  };

  ArkUI_AttributeItem borderRadiusItem = {borderRadiusValue, sizeof(borderRadiusValue) / sizeof(ArkUI_NumberValue), nullptr, nullptr};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_BORDER_RADIUS, &borderRadiusItem));
  return *this;
}

ArkUINode &ArkUINode::SetBorderStyle(ArkUI_BorderStyle top, ArkUI_BorderStyle right, ArkUI_BorderStyle bottom, ArkUI_BorderStyle left) {
  ArkUI_NumberValue borderStyleValue[] = {
    {.i32 = static_cast<int32_t>(top)},
    {.i32 = static_cast<int32_t>(right)},
    {.i32 = static_cast<int32_t>(bottom)},
    {.i32 = static_cast<int32_t>(left)}
  };
  ArkUI_AttributeItem borderStyleItem = {borderStyleValue, sizeof(borderStyleValue) / sizeof(ArkUI_NumberValue), nullptr, nullptr};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_BORDER_STYLE, &borderStyleItem));
  return *this;
}

ArkUINode &ArkUINode::SetShadow(const HRShadow &shadow) {
  float shadowOpacity = 1.f;
  if (shadow.shadowOpacity.has_value() && shadow.shadowOpacity.value() > 0 && shadow.shadowOpacity.value() < 1.f) {
    shadowOpacity = shadow.shadowOpacity.value();
  }
  uint32_t shadowColorValue = 0xff000000;
  if (shadow.shadowColor.has_value()) {
    shadowColorValue = shadow.shadowColor.value();
  }
  uint32_t alpha = static_cast<uint32_t>((float)((shadowColorValue >> 24) & (0xff)) * shadowOpacity);
  shadowColorValue = (alpha << 24) + (shadowColorValue & 0xffffff);
  ArkUI_NumberValue shadowValue[] = {{.f32 = shadow.shadowRadius},
                                     {.i32 = 0},
                                     {.f32 = static_cast<float>(shadow.shadowOffset.width)},
                                     {.f32 = static_cast<float>(shadow.shadowOffset.height)},
                                     {.i32 = 0},
                                     {.u32 = shadowColorValue},
                                     {.u32 = 0}};
  ArkUI_AttributeItem shadowItem = {.value = shadowValue, .size = sizeof(shadowValue) / sizeof(ArkUI_NumberValue)};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_CUSTOM_SHADOW, &shadowItem));
  return *this;
}

ArkUINode &ArkUINode::SetExpandSafeArea(){
//TODO  NODE_EXPAND_SAFE_AREA not define in devEco 5.0.0.400 will add in later
//  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_EXPAND_SAFE_AREA,nullptr ));
  return *this;    
}

ArkUINode &ArkUINode::SetTransitionMove(const ArkUI_TransitionEdge edgeType,int32_t duration,ArkUI_AnimationCurve curveType){
  ArkUI_NumberValue value[] = {{.i32 = edgeType}, {.i32 = duration}, {.i32 = curveType}};
  ArkUI_AttributeItem item = {value, sizeof(value) / sizeof(ArkUI_NumberValue), nullptr, nullptr};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_MOVE_TRANSITION, &item));
  return *this;    
}

ArkUINode &ArkUINode::SetTransitionOpacity(const ArkUI_AnimationCurve curveType,int32_t duration){
  ArkUI_NumberValue value[] = {{.f32 = 0},{.i32 = duration},{.i32 = curveType}};
  ArkUI_AttributeItem item = {value, sizeof(value) / sizeof(ArkUI_NumberValue), nullptr, nullptr};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_OPACITY_TRANSITION, &item));  
  return *this;     
}

ArkUINode &ArkUINode::SetTransitionTranslate(float distanceX,float distanceY,float distanceZ,ArkUI_AnimationCurve curveType,int32_t duration)
{
  ArkUI_NumberValue value[] = {{.f32 = distanceX},{.f32 = distanceY},{.f32 = distanceZ},{.i32 = duration},{.i32 = curveType}};
  ArkUI_AttributeItem item = {value, sizeof(value) / sizeof(ArkUI_NumberValue), nullptr, nullptr};
  MaybeThrow(NativeNodeApi::GetInstance()->setAttribute(nodeHandle_, NODE_TRANSLATE_TRANSITION, &item));      
  return *this;   
}

void ArkUINode::ResetNodeAttribute(ArkUI_NodeAttributeType type){
  MaybeThrow(NativeNodeApi::GetInstance()->resetAttribute(nodeHandle_, type));
}

void ArkUINode::RegisterClickEvent() {
  if (!hasClickEvent_) {
    MaybeThrow(NativeNodeApi::GetInstance()->registerNodeEvent(nodeHandle_, NODE_ON_CLICK, 0, nullptr));
    hasClickEvent_ = true;
  }
}

void ArkUINode::UnregisterClickEvent() {
  if (hasClickEvent_) {
    NativeNodeApi::GetInstance()->unregisterNodeEvent(nodeHandle_, NODE_ON_CLICK);
    hasClickEvent_ = false;
  }
}

void ArkUINode::RegisterAppearEvent() {
    if (!hasAppearEvent_) {
    MaybeThrow(NativeNodeApi::GetInstance()->registerNodeEvent(nodeHandle_, NODE_EVENT_ON_APPEAR, 0, nullptr));
    hasAppearEvent_ = true;
  }
}

void ArkUINode::UnregisterAppearEvent() {
  if (hasAppearEvent_) {
    NativeNodeApi::GetInstance()->unregisterNodeEvent(nodeHandle_, NODE_EVENT_ON_APPEAR);
    hasAppearEvent_ = false;
  }
}

void ArkUINode::RegisterDisappearEvent() {
    if (!hasDisappearEvent_) {
    MaybeThrow(NativeNodeApi::GetInstance()->registerNodeEvent(nodeHandle_, NODE_EVENT_ON_DISAPPEAR, 0, nullptr));
    hasDisappearEvent_ = true;
  }
}

void ArkUINode::UnregisterDisappearEvent() {
  if (hasDisappearEvent_) {
    NativeNodeApi::GetInstance()->unregisterNodeEvent(nodeHandle_, NODE_EVENT_ON_DISAPPEAR);
    hasDisappearEvent_ = false;
  }
}

void ArkUINode::RegisterAreaChangeEvent(){
  if (!hasAreaChangeEvent_){
    MaybeThrow(NativeNodeApi::GetInstance()->registerNodeEvent(nodeHandle_, NODE_EVENT_ON_AREA_CHANGE, 0, nullptr));
    hasAreaChangeEvent_ = true ; 
  }  
}

void ArkUINode::UnregisterAreaChangeEvent(){
  if (hasAreaChangeEvent_){
    NativeNodeApi::GetInstance()->unregisterNodeEvent(nodeHandle_, NODE_EVENT_ON_AREA_CHANGE);
    hasAreaChangeEvent_ = false ; 
  }      
}

} // namespace native
} // namespace render
} // namespace hippy
