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

#include "renderer/native_render_impl.h"

namespace hippy {
inline namespace render {
inline namespace native {

NativeRenderImpl::NativeRenderImpl(uint32_t instance_id) : instance_id_(instance_id) {
  hr_manager_ = std::make_shared<HRManager>(instance_id);
}

void NativeRenderImpl::RegisterNativeXComponentHandle(OH_NativeXComponent *nativeXComponent, uint32_t root_id) {
  auto view_manager = hr_manager_->GetViewManager(root_id);
  if (!view_manager) {
    return;
  }
  view_manager->AttachToNativeXComponent(nativeXComponent);
}

} // namespace native
} // namespace render
} // namespace hippy
