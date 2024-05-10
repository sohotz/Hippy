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

#include "renderer/native_render_provider.h"
#include "footstone/logging.h"
#include "footstone/macros.h"
#include "oh_napi/oh_napi_task_runner.h"

namespace hippy {
inline namespace render {
inline namespace native {

NativeRenderProvider::NativeRenderProvider(uint32_t instance_id) : instance_id_(instance_id) {
  render_impl_ = std::make_shared<NativeRenderImpl>(instance_id);
  render_impl_->InitRenderManager();
}

void NativeRenderProvider::RegisterNativeXComponentHandle(OH_NativeXComponent *nativeXComponent, uint32_t root_id) {
  // 说明：虽然该注册方法来自主线程调用，但也需要异步，否则 rootView 不能 attach 到 xcomponent 上。
  OhNapiTaskRunner *taskRunner = OhNapiTaskRunner::Instance(ts_env_);
  taskRunner->RunAsyncTask([render_impl = render_impl_, nativeXComponent = nativeXComponent, root_id = root_id]() {
    render_impl->RegisterNativeXComponentHandle(nativeXComponent, root_id);
  });
}

void NativeRenderProvider::CreateNode(uint32_t root_id, const std::vector<std::shared_ptr<HRCreateMutation>> &mutations) {
  OhNapiTaskRunner *taskRunner = OhNapiTaskRunner::Instance(ts_env_);
  taskRunner->RunAsyncTask([render_impl = render_impl_, root_id = root_id, mutations = mutations]() {
    render_impl->CreateNode(root_id, mutations);
  });
}

void NativeRenderProvider::UpdateNode(uint32_t root_id, const std::vector<std::shared_ptr<HRUpdateMutation>> &mutations) {
  OhNapiTaskRunner *taskRunner = OhNapiTaskRunner::Instance(ts_env_);
  taskRunner->RunAsyncTask([render_impl = render_impl_, root_id = root_id, mutations = mutations]() {
    render_impl->UpdateNode(root_id, mutations);
  });
}

void NativeRenderProvider::MoveNode(uint32_t root_id, const std::shared_ptr<HRMoveMutation> &mutation) {
  OhNapiTaskRunner *taskRunner = OhNapiTaskRunner::Instance(ts_env_);
  taskRunner->RunAsyncTask([render_impl = render_impl_, root_id = root_id, mutation = mutation]() {
    render_impl->MoveNode(root_id, mutation);
  });
}

void NativeRenderProvider::MoveNode2(uint32_t root_id, const std::shared_ptr<HRMove2Mutation> &mutation) {
  OhNapiTaskRunner *taskRunner = OhNapiTaskRunner::Instance(ts_env_);
  taskRunner->RunAsyncTask([render_impl = render_impl_, root_id = root_id, mutation = mutation]() {
    render_impl->MoveNode2(root_id, mutation);
  });
}

void NativeRenderProvider::DeleteNode(uint32_t root_id, const std::vector<std::shared_ptr<HRDeleteMutation>> &mutations) {
  OhNapiTaskRunner *taskRunner = OhNapiTaskRunner::Instance(ts_env_);
  taskRunner->RunAsyncTask([render_impl = render_impl_, root_id = root_id, mutations = mutations]() {
    render_impl->DeleteNode(root_id, mutations);
  });
}

void NativeRenderProvider::UpdateLayout(uint32_t root_id, const std::vector<std::shared_ptr<HRUpdateLayoutMutation>> &mutations) {
  OhNapiTaskRunner *taskRunner = OhNapiTaskRunner::Instance(ts_env_);
  taskRunner->RunAsyncTask([render_impl = render_impl_, root_id = root_id, mutations = mutations]() {
    render_impl->UpdateLayout(root_id, mutations);
  });
}

void NativeRenderProvider::UpdateEventListener(uint32_t root_id,
                         const std::vector<std::shared_ptr<HRUpdateEventListenerMutation>> &mutations) {
  OhNapiTaskRunner *taskRunner = OhNapiTaskRunner::Instance(ts_env_);
  taskRunner->RunAsyncTask([render_impl = render_impl_, root_id = root_id, mutations = mutations]() {
    render_impl->UpdateEventListener(root_id, mutations);
  });
}

void NativeRenderProvider::EndBatch(uint32_t root_id) {
  OhNapiTaskRunner *taskRunner = OhNapiTaskRunner::Instance(ts_env_);
  taskRunner->RunAsyncTask([render_impl = render_impl_, root_id = root_id]() {
    render_impl->EndBatch(root_id);
  });
}

} // namespace native
} // namespace render
} // namespace hippy
