/*
 * Tencent is pleased to support the open source community by making
 * Hippy available.
 *
 * Copyright (C) 2022 THL A29 Limited, a Tencent company.
 * All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <js_native_api.h>
#include <js_native_api_types.h>
#include "footstone/string_view.h"
#include "footstone/task_runner.h"

namespace hippy {
inline namespace framework {
inline namespace renderer {
inline namespace native {

void SetRenderDelegate(napi_env env, napi_ref ts_render_provider_ref);
void ClearRenderDelegate();

void CallRenderDelegateSetIdMethod(const std::string& method, uint32_t id);
void CallRenderDelegateMethod(const std::string& method, uint32_t root_id, const std::pair<uint8_t*, size_t>& buffer);
void CallRenderDelegateMethod(const std::string& method, uint32_t root_id);
void CallRenderDelegateMoveNodeMethod(const std::string& method, uint32_t root_id, uint32_t pid, const std::pair<uint8_t*, size_t>& buffer);
void CallRenderDelegateMoveNodeMethod(const std::string& method, uint32_t root_id, std::vector<int32_t>& moved_ids, int32_t to_pid, int32_t from_pid, int32_t index);
void CallRenderDelegateDeleteNodeMethod(const std::string& method, uint32_t root_id, std::vector<uint32_t>& ids);
void CallRenderDelegateCallFunctionMethod(const std::string& method, uint32_t root_id,
  uint32_t node_id, uint32_t cb_id, const std::string& functionName, const std::pair<uint8_t*, size_t>& buffer);
void CallRenderDelegateMeasureMethod(const std::string& method, uint32_t root_id,
  uint32_t node_id, const float width, const int32_t width_mode, const float height, const int32_t height_mode, int64_t& result);

}
}
}
}
