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
#include "connector/dom_interceptor.h"

#include "dom/animation/animation_manager.h"
#include "dom/diff_utils.h"
#include "dom/dom_action_interceptor.h"
#include "dom/dom_event.h"
#include "dom/dom_node.h"
#include "dom/layer_optimized_render_manager.h"
#include "dom/render_manager.h"
#include "dom/root_node.h"
#include "dom/scene_builder.h"
#include "footstone/serializer.h"
#include "footstone/deserializer.h"
#include "footstone/one_shot_timer.h"
#include "footstone/string_view_utils.h"
#include "footstone/task_runner.h"
#include "footstone/time_delta.h"
#include "footstone/worker.h"
#include "oh_napi/data_holder.h"
#include "footstone/worker_impl.h"


namespace hippy {
inline namespace dom {

using DomNode = hippy::DomNode;
using Task = footstone::Task;
using TaskRunner = footstone::TaskRunner;
using TimeDelta = footstone::TimeDelta;
using OneShotTimer = footstone::timer::OneShotTimer;
using Serializer = footstone::value::Serializer;
using Deserializer = footstone::value::Deserializer;
using WorkerImpl = footstone::runner::WorkerImpl;
using StringViewUtils = footstone::stringview::StringViewUtils;

using HippyValueArrayType = footstone::value::HippyValue::HippyValueArrayType;

constexpr char kDomWorkerName[] = "dom_worker";
constexpr char kDomRunnerName[] = "dom_task_runner";

class DomInterceptor : public DomManager {
  public:
  DomInterceptor(HippyDomInterceptor handler) : DomManager(DomManagerType::kJson), handler_(handler) {}
  ~DomInterceptor() override = default;

  void SetRenderManager(const std::weak_ptr<RenderManager> &render_manager) override {
    throw std::runtime_error("Not implemented");
  }

  std::weak_ptr<RenderManager> GetRenderManager() override {
    throw std::runtime_error("Not implemented");
  }

  std::shared_ptr<DomNode> GetNode(const std::weak_ptr<RootNode> &weak_root_node,
                                   uint32_t id) override {
    throw std::runtime_error("Not implemented");
  }

  void CreateDomNodes(const std::weak_ptr<RootNode> &weak_root_node,
                      std::vector<std::shared_ptr<DomInfo>> &&nodes,
                      bool needSortByIndex) override {
    auto root_node = weak_root_node.lock();
    if (!root_node || nodes.size() == 0) {
      return;
    }
    handler_.CreateDomNodes(handler_.context, root_node->GetId(), GetNodesJson(nodes).str().c_str(), needSortByIndex);
  }

  void UpdateDomNodes(const std::weak_ptr<RootNode> &weak_root_node,
                      std::vector<std::shared_ptr<DomInfo>> &&nodes) override {
    auto root_node = weak_root_node.lock();
    if (!root_node || nodes.size() == 0) {
      return;
    }
    handler_.UpdateDomNodes(handler_.context, root_node->GetId(), GetNodesJson(nodes).str().c_str());
  }

  void MoveDomNodes(const std::weak_ptr<RootNode> &weak_root_node,
                    std::vector<std::shared_ptr<DomInfo>> &&nodes) override {
    auto root_node = weak_root_node.lock();
    if (!root_node || nodes.size() == 0) {
      return;
    }
    handler_.MoveDomNodes(handler_.context, root_node->GetId(), GetNodesJson(nodes).str().c_str());
  }

  void DeleteDomNodes(const std::weak_ptr<RootNode> &weak_root_node,
                      std::vector<std::shared_ptr<DomInfo>> &&nodes) override {
    auto root_node = weak_root_node.lock();
    if (!root_node || nodes.size() == 0) {
      return;
    }
    handler_.DeleteDomNodes(handler_.context, root_node->GetId(), GetNodesJson(nodes).str().c_str());
  }

  void UpdateAnimation(const std::weak_ptr<RootNode> &weak_root_node,
                       std::vector<std::shared_ptr<DomNode>> &&nodes) override {
    auto root_node = weak_root_node.lock();
    if (!root_node || nodes.size() == 0) {
      return;
    }
    // todo
    // handler_.UpdateAnimation(root_node->GetId(), GetNodesJson(nodes).str().c_str());
  }

  void EndBatch(const std::weak_ptr<RootNode> &weak_root_node) override {
    auto root_node = weak_root_node.lock();
    if (!root_node) {
      return;
    }
    handler_.EndBatch(handler_.context, root_node->GetId());
  }

  void AddEventListener(const std::weak_ptr<RootNode> &weak_root_node, uint32_t dom_id,
                        const std::string &event_name, uint64_t listener_id, bool use_capture,
                        const EventCallback &cb) override {
    auto root_node = weak_root_node.lock();
    if (!root_node) {
      return;
    }
    handler_.AddEventListener(handler_.context, root_node->GetId(), dom_id, event_name.c_str(),
                              listener_id, use_capture, &cb);
  }

  void RemoveEventListener(const std::weak_ptr<RootNode> &weak_root_node, uint32_t dom_id,
                           const std::string &event_name, uint64_t listener_id) override {
    auto root_node = weak_root_node.lock();
    if (!root_node) {
      return;
    }
    handler_.RemoveEventListener(handler_.context, root_node->GetId(), dom_id, event_name.c_str(), listener_id);
  }

  void CallFunction(const std::weak_ptr<RootNode> &weak_root_node, uint32_t dom_id,
                    const std::string &name, const DomArgument &param,
                    const CallFunctionCallback &cb) override {
    auto root_node = weak_root_node.lock();
    if (!root_node) {
      return;
    }
    handler_.CallFunction(handler_.context, root_node->GetId(), dom_id, name.c_str(), &param, &cb);
  }

  void SetRootSize(const std::weak_ptr<RootNode> &weak_root_node, float width,
                   float height) override {
    auto root_node = weak_root_node.lock();
    if (!root_node) {
      return;
    }
    handler_.SetRootSize(handler_.context, root_node->GetId(), width, height);
  }

  void DoLayout(const std::weak_ptr<RootNode> &weak_root_node) override {
    auto root_node = weak_root_node.lock();
    if (!root_node) {
      return;
    }
    handler_.DoLayout(handler_.context, root_node->GetId());
  }

  void PostTask(const Scene &&scene) override { throw std::runtime_error("Not implemented"); }
  uint32_t PostDelayedTask(const Scene &&scene, footstone::TimeDelta delay) override {
    throw std::runtime_error("Not implemented");
  }
  void CancelTask(uint32_t id) override { throw std::runtime_error("Not implemented"); }

  byte_string GetSnapShot(const std::shared_ptr<RootNode> &root_node) override {
    throw std::runtime_error("Not implemented");
  }

  bool SetSnapShot(const std::shared_ptr<RootNode> &root_node, const byte_string &buffer) override {
    throw std::runtime_error("Not implemented");
  }

 private:
  friend class DomNode;
  std::ostringstream GetNodesJson(std::vector<std::shared_ptr<DomInfo>> &nodes) {
    size_t count = nodes.size();
    std::ostringstream oss;
    oss << '[';
    for (size_t i = 0; i < count; ++i) {
      if (i != 0) {
        oss << ',';
      }
      oss << StringViewUtils::ToStdString(StringViewUtils::ConvertEncoding(*nodes[i]->stringify, string_view::Encoding::Utf8).utf8_value());
    }
    oss << ']';
    return oss;
  }

  uint32_t id_;
  std::unordered_map<uint32_t, std::shared_ptr<BaseTimer>> timer_map_;
  HippyDomInterceptor handler_;
};

} // namespace dom
} // namespace hippy

EXTERN_C_START
uint32_t HippyCreateDomInterceptor(HippyDomInterceptor handler) {
  std::shared_ptr<hippy::DomManager> dom_manager = std::make_shared<hippy::DomInterceptor>(handler);
  auto dom_manager_id = hippy::global_data_holder_key.fetch_add(1);
  hippy::global_data_holder.Insert(dom_manager_id, dom_manager);
  std::string worker_name = hippy::dom::kDomWorkerName + std::to_string(dom_manager_id);
  auto worker = std::make_shared<hippy::WorkerImpl>(worker_name, false);
  worker->Start();
  auto runner = std::make_shared<hippy::TaskRunner>(hippy::dom::kDomRunnerName);
  runner->SetWorker(worker);
  worker->Bind({runner});
  dom_manager->SetTaskRunner(runner);
  dom_manager->SetWorker(worker);
  
  hippy::global_dom_manager_num_holder.Insert(dom_manager_id, 1u);
  return dom_manager_id;
}

void HippyDestroyDomInterceptor(uint32_t dom_interceptor_id) {
  uint32_t dom_manager_num = 0;
  auto flag = hippy::global_dom_manager_num_holder.Find(dom_interceptor_id, dom_manager_num);
  FOOTSTONE_CHECK(dom_manager_num == 1);
  std::any dom_manager;
  flag = hippy::global_data_holder.Find(dom_interceptor_id, dom_manager);
  FOOTSTONE_CHECK(flag);
  // auto dom_manager_object = std::any_cast<std::shared_ptr<hippy::DomManager>>(dom_manager);
  // dom_manager_object->GetWorker()->Terminate();
  flag = hippy::global_data_holder.Erase(dom_interceptor_id);
  FOOTSTONE_DCHECK(flag);
}
EXTERN_C_END