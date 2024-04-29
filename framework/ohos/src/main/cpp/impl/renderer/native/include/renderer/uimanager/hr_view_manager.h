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

#include <ace/xcomponent/native_interface_xcomponent.h>
#include "renderer/components/root_view.h"

namespace hippy {
inline namespace render {
inline namespace native {

class HRViewManager {
public:
  HRViewManager(uint32_t instance_id, uint32_t root_id);
  ~HRViewManager() = default;
  
  void AttachToNativeXComponent(OH_NativeXComponent* nativeXComponent);

private:
  std::shared_ptr<NativeRenderContext> ctx_;
  OH_NativeXComponent *nativeXComponent_;
  std::shared_ptr<RootView> root_view_;
};

} // namespace native
} // namespace render
} // namespace hippy
