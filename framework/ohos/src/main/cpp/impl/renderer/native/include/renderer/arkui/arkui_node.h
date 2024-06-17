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

#pragma once

#include <arkui/native_node.h>
#include <arkui/native_type.h>
#include <memory>
#include <string>
#include "footstone/logging.h"
#include "renderer/utils/hr_types.h"

#define HIPPY_OHOS_MEM_CHECK 0

namespace hippy {
inline namespace render {
inline namespace native {

// ArkUI_NativeModule API ref:
// https://gitee.com/openharmony/docs/blob/master/zh-cn/application-dev/reference/apis-arkui/_ark_u_i___native_module.md#arkui_nodeattributetype

enum class ArkUIHitTestMode : int32_t {
  DEFAULT,
  BLOCK,
  TRANSPARENT,
  NONE,
};

class ArkUINode {
protected:
  ArkUINode(const ArkUINode &other) = delete;
  ArkUINode &operator=(const ArkUINode &other) = delete;

  ArkUINode &operator=(ArkUINode &&other) noexcept;
  ArkUINode(ArkUINode &&other) noexcept;

public:
  using Alignment = ArkUI_Alignment;
  
  ArkUINode(ArkUI_NodeHandle nodeHandle);
  virtual ~ArkUINode();

  ArkUI_NodeHandle GetArkUINodeHandle();

  void MarkDirty();
  
  void AddChild(ArkUINode &child);
  void InsertChild(ArkUINode &child, int32_t index);
  void RemoveChild(ArkUINode &child);

  virtual ArkUINode &SetPosition(const HRPosition &position);
  virtual ArkUINode &SetSize(const HRSize &size);
  virtual ArkUINode &SetWidth(float width);
  virtual ArkUINode &SetHeight(float height);
  virtual ArkUINode &SetSizePercent(const HRSize &size);
  virtual ArkUINode &SetWidthPercent(float percent);
  virtual ArkUINode &SetHeightPercent(float percent);
  virtual ArkUINode &SetVisibility(bool visibility);
  virtual ArkUINode &SetBackgroundColor(uint32_t color);
  virtual ArkUINode &SetOpacity(float opacity);
  virtual ArkUINode &SetTransform(const HRTransform &transform, float pointScaleFactor);
  virtual ArkUINode &SetMatrix(const HRMatrix &transformMatrix, float pointScaleFactor);
  virtual ArkUINode &SetRotate(const HRRotate &rotate);
  virtual ArkUINode &SetScale(const HRScale &scale);
  virtual ArkUINode &SetTranslate(const HRTranslate &translate, float pointScaleFactor);
  virtual ArkUINode &SetClip(bool clip);
  virtual ArkUINode &SetZIndex(int32_t zIndex);
  virtual ArkUINode &SetAccessibilityText(const std::string &accessibilityLabel);
  virtual ArkUINode &SetFocusable(bool focusable);
  virtual ArkUINode &SetFocusStatus(int32_t focus);
  virtual ArkUINode &SetLinearGradient(const HRLinearGradient &linearGradient);
  virtual ArkUINode &SetId(const int32_t &tag);
  virtual ArkUINode &SetHitTestMode(const ArkUIHitTestMode mode);
  virtual ArkUINode &SetEnabled(bool enabled);
  virtual ArkUINode &SetBackgroundImage(const std::string &uri);
  virtual ArkUINode &SetBackgroundImagePosition(const HRPosition &position);
  virtual ArkUINode &SetBackgroundImageSize(const ArkUI_ImageSize sizeStyle);
  virtual ArkUINode &SetBorderWidth(float top, float right, float bottom, float left);
  virtual ArkUINode &SetBorderColor(uint32_t top, uint32_t right, uint32_t bottom, uint32_t left);
  virtual ArkUINode &SetBorderRadius(float topLeft, float topRight, float bottomLeft, float bottomRight);
  virtual ArkUINode &SetBorderStyle(ArkUI_BorderStyle top, ArkUI_BorderStyle right, ArkUI_BorderStyle bottom, ArkUI_BorderStyle left);
  virtual ArkUINode &SetShadow(const HRShadow &shadow);
  virtual HRSize GetSize() const;
  virtual uint32_t GetTotalChildCount() const;

  virtual void OnNodeEvent(ArkUI_NodeEvent *event) {}

  void RegisterClickEvent();
  void UnregisterClickEvent();
  void RegisterAppearEvent();
  void UnregisterAppearEvent();
  void RegisterDisappearEvent();
  void UnregisterDisappearEvent();

protected:
  void MaybeThrow(int32_t status) {
    if (status != 0) {
      auto message = std::string("ArkUINode operation failed with status: ") + std::to_string(status);
      FOOTSTONE_LOG(ERROR) << message;
      throw std::runtime_error(std::move(message));
    }
  }

  ArkUI_NodeHandle nodeHandle_;
  
  bool hasClickEvent_ = false;
  bool hasAppearEvent_ = false;
  bool hasDisappearEvent_ = false;
};

} // namespace native
} // namespace render
} // namespace hippy
