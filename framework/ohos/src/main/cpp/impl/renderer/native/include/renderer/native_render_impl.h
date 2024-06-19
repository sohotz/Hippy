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
#include <memory>
#include "renderer/native_render.h"
#include "renderer/uimanager/hr_manager.h"
#include "renderer/uimanager/hr_mutation.h"

namespace hippy {
inline namespace render {
inline namespace native {

class NativeRenderImpl : public NativeRender {
public:
  NativeRenderImpl(uint32_t instance_id);
  ~NativeRenderImpl() = default;

  void InitRenderManager();
  
  uint32_t GetInstanceId() { return instance_id_; }
  
  void RegisterNativeXComponentHandle(OH_NativeXComponent *nativeXComponent, uint32_t root_id);
  void RegisterCustomTsRenderViews(uint32_t root_id, const std::set<std::string> &views, napi_ref builder_callback_ref, napi_env env);
  
  void DestroyRoot(uint32_t root_id);

  void CreateNode(uint32_t root_id, const std::vector<std::shared_ptr<HRCreateMutation>> &mutations);
  void UpdateNode(uint32_t root_id, const std::vector<std::shared_ptr<HRUpdateMutation>> &mutations);
  void MoveNode(uint32_t root_id, const std::shared_ptr<HRMoveMutation> &mutation);
  void MoveNode2(uint32_t root_id, const std::shared_ptr<HRMove2Mutation> &mutation);
  void DeleteNode(uint32_t root_id, const std::vector<std::shared_ptr<HRDeleteMutation>> &mutations);
  void UpdateLayout(uint32_t root_id, const std::vector<std::shared_ptr<HRUpdateLayoutMutation>> &mutations);
  void UpdateEventListener(uint32_t root_id, const std::vector<std::shared_ptr<HRUpdateEventListenerMutation>> &mutations);
  void EndBatch(uint32_t root_id);
  
  bool CheckRegisteredEvent(uint32_t root_id, uint32_t node_id, std::string &event_name);

  void CallUIFunction(uint32_t root_id, uint32_t node_id, const std::string &functionName,
                      const std::vector<HippyValue> params, std::function<void(const HippyValue &result)> callback);

  void SpanPosition(uint32_t root_id, uint32_t node_id, float x, float y);
  
  uint64_t AddEndBatchCallback(uint32_t root_id, const EndBatchCallback &cb) override;
  void RemoveEndBatchCallback(uint32_t root_id, uint64_t cbId) override;

private:
  uint32_t instance_id_;
  std::shared_ptr<HRManager> hr_manager_;
};

} // namespace native
} // namespace render
} // namespace hippy
