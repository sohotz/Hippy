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

#include "renderer/native_render_impl.h"
#include <memory>
#include <sys/types.h>

namespace hippy {
inline namespace render {
inline namespace native {

class NativeRenderProvider {
public:
  NativeRenderProvider(uint32_t instance_id);
  ~NativeRenderProvider() = default;
  
  uint32_t GetInstanceId() { return instance_id_; }

  void RegisterNativeXComponentHandle(OH_NativeXComponent *nativeXComponent, uint32_t root_id);

private:
  uint32_t instance_id_;
  std::shared_ptr<NativeRenderImpl> render_impl_;
};

} // namespace native
} // namespace render
} // namespace hippy
