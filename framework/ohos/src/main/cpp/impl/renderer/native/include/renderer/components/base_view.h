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
#include "renderer/arkui/arkui_node.h"
#include "renderer/native_render_context.h"

namespace hippy {
inline namespace render {
inline namespace native {

class BaseView {
public:
  BaseView(std::shared_ptr<NativeRenderContext> &ctx);
  virtual ~BaseView() = default;
  
  void SetTag(int tag) { tag_ = tag; }
  void SetViewType(std::string &type) { view_type_ = type; }

//   virtual ArkUINode &GetLocalRootArkUINode() = 0;

private:
  std::shared_ptr<NativeRenderContext> ctx_;
  int tag_;
  std::string view_type_;
  std::vector<std::shared_ptr<BaseView>> children_;
  std::weak_ptr<BaseView> parent_;
};

}  // namespace native
}  // namespace render
}  // namespace hippy
