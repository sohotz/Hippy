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

#include <string>
#include <sys/types.h>
#include "renderer/arkui/arkui_node.h"
#include "renderer/native_render_context.h"
#include "footstone/hippy_value.h"

namespace hippy {
inline namespace render {
inline namespace native {

using HippyValue = footstone::HippyValue;
using HippyValueObjectType = footstone::value::HippyValue::HippyValueObjectType;

class BaseView : public std::enable_shared_from_this<BaseView> {
public:
  BaseView(std::shared_ptr<NativeRenderContext> &ctx);
  virtual ~BaseView() = default;
  
  std::shared_ptr<NativeRenderContext> &GetCtx() { return ctx_; }
  uint32_t GetTag() { return tag_; }
  std::string &GetViewType() { return view_type_; }
  std::vector<std::shared_ptr<BaseView>> &GetChildren() { return children_; }
  std::weak_ptr<BaseView> &GetParent() { return parent_; }
  
  void SetTag(uint32_t tag) { tag_ = tag; }
  void SetViewType(std::string &type) { view_type_ = type; }
  void SetParent(std::shared_ptr<BaseView> parent) { parent_ = parent; }

  virtual ArkUINode &GetLocalRootArkUINode() = 0;
  
  bool SetProp(const std::string &propKey, HippyValue &propValue);
  void AddSubRenderView(std::shared_ptr<BaseView> &subView, int32_t index);
  void RemoveSubView(std::shared_ptr<BaseView> &subView);
  void RemoveFromParentView();
  bool IsImageSpan();
  void SetRenderViewFrame(const HRRect &frame);
  void UpdateEventListener(HippyValueObjectType &newEvents);
  bool CheckRegisteredEvent(std::string &eventName);
  
protected:
  virtual void OnChildInserted(std::shared_ptr<BaseView> const &childView, int index) {}
  virtual void OnChildRemoved(std::shared_ptr<BaseView> const &childView) {}
  virtual void UpdateRenderViewFrame(const HRRect &frame);

private:
  std::shared_ptr<NativeRenderContext> ctx_;
  uint32_t tag_;
  std::string view_type_;
  std::vector<std::shared_ptr<BaseView>> children_;
  std::weak_ptr<BaseView> parent_;
};

}  // namespace native
}  // namespace render
}  // namespace hippy
