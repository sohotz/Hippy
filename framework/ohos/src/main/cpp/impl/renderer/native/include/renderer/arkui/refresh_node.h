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

#include "renderer/arkui/arkui_node.h"
#include <cmath>
#include <cstdint>

namespace hippy {
inline namespace render {
inline namespace native {

class RefreshNodeDelegate {
public:
  virtual ~RefreshNodeDelegate() = default;
  virtual void OnRefreshing() {}
  virtual void OnStateChange(int32_t state) {}
  virtual void OnOffsetChange(float_t offset) {}
};

class RefreshNode : public ArkUINode {
protected:
  RefreshNodeDelegate *refreshNodeDelegate_ = nullptr;  
public:
  RefreshNode();
  ~RefreshNode();
  void OnNodeEvent(ArkUI_NodeEvent *event) override;
  void SetNodeDelegate(RefreshNodeDelegate *refreshNodeDelegate);  
  void SetRefreshing(bool beRefreshed);
};

} // namespace native
} // namespace render
} // namespace hippy
