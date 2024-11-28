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

#include <cstdint>
#include <future>
#include <map>
#include <memory>
#include <vector>

#include "dom/animation/animation_manager.h"
#include "dom/dom_action_interceptor.h"
#include "dom/dom_argument.h"
#include "dom/dom_event.h"
#include "dom/dom_listener.h"
#include "dom/layout_node.h"
#include "dom/scene.h"
#include "footstone/hippy_value.h"
#include "footstone/logging.h"
#include "footstone/macros.h"
#include "footstone/task_runner.h"
#include "footstone/time_delta.h"
#include "footstone/base_timer.h"
#include "footstone/worker.h"

namespace hippy {
inline namespace dom {

class AnimationManager;
class DomNode;
class RenderManager;
class RootNode;
class LayerOptimizedRenderManager;
class DomEvent;
struct DomInfo;

using EventCallback = std::function<void(const std::shared_ptr<DomEvent>&)>;
using CallFunctionCallback = std::function<void(std::shared_ptr<DomArgument>)>;

// This class is used to mainpulate dom. Please note that the member
// function of this class must be run in dom thread. If you want to call
// in other thread please use PostTask.
// Example:
//    std::vector<std::function<void()>> ops;
//    ops.emplace_back([]() {
//      some_ops();
//    });
//    dom_manager->PostTask(Scene(std::move(ops)));
class DomManager {
 public:
  using byte_string = std::string;
  using HippyValue = footstone::value::HippyValue;
  using TaskRunner = footstone::runner::TaskRunner;
  using Task = footstone::Task;
  using BaseTimer = footstone::timer::BaseTimer;
  using Worker = footstone::Worker;

  DomManager() = default;
  virtual ~DomManager() = default;

  DomManager(DomManager&) = delete;
  DomManager& operator=(DomManager&) = delete;

  inline std::shared_ptr<TaskRunner> GetTaskRunner() { return task_runner_; }
  inline void SetTaskRunner(std::shared_ptr<TaskRunner> runner) {
    task_runner_ =  runner;
  }
  inline void SetWorker(std::shared_ptr<Worker> worker) {
    worker_ = worker;
  }
  inline std::shared_ptr<Worker> GetWorker() {
    return worker_;
  }

  virtual void SetRenderManager(const std::weak_ptr<RenderManager>& render_manager) = 0;
  virtual std::weak_ptr<RenderManager> GetRenderManager() = 0;
  virtual std::shared_ptr<DomNode> GetNode(const std::weak_ptr<RootNode>& weak_root_node, uint32_t id) = 0;

  virtual void CreateDomNodes(const std::weak_ptr<RootNode>& weak_root_node,
                              std::vector<std::shared_ptr<DomInfo>>&& nodes, bool needSortByIndex) = 0;
  virtual void UpdateDomNodes(const std::weak_ptr<RootNode>& weak_root_node,
                              std::vector<std::shared_ptr<DomInfo>>&& nodes) = 0;
  virtual void MoveDomNodes(const std::weak_ptr<RootNode>& weak_root_node,
                            std::vector<std::shared_ptr<DomInfo>>&& nodes) = 0;
  virtual void DeleteDomNodes(const std::weak_ptr<RootNode>& weak_root_node,
                              std::vector<std::shared_ptr<DomInfo>>&& nodes) = 0;
  virtual void UpdateAnimation(const std::weak_ptr<RootNode>& weak_root_node,
                               std::vector<std::shared_ptr<DomNode>>&& nodes) = 0;
  virtual void EndBatch(const std::weak_ptr<RootNode>& root_node) = 0;
  // 返回0代表失败，正常id从1开始
  virtual void AddEventListener(const std::weak_ptr<RootNode>& weak_root_node, uint32_t dom_id,
                                const std::string& event_name, uint64_t listener_id, bool use_capture,
                                const EventCallback& cb) = 0;
  virtual void RemoveEventListener(const std::weak_ptr<RootNode>& weak_root_node, uint32_t id, const std::string& name,
                                   uint64_t listener_id) = 0;
  virtual void CallFunction(const std::weak_ptr<RootNode>& weak_root_node, uint32_t id, const std::string& name,
                            const DomArgument& param, const CallFunctionCallback& cb) = 0;
  virtual void SetRootSize(const std::weak_ptr<RootNode>& weak_root_node, float width, float height) = 0;
  virtual void DoLayout(const std::weak_ptr<RootNode>& weak_root_node) = 0;
  virtual void PostTask(const Scene&& scene) = 0;
  virtual uint32_t PostDelayedTask(const Scene&& scene, footstone::TimeDelta delay) = 0;
  virtual void CancelTask(uint32_t id) = 0;

  virtual byte_string GetSnapShot(const std::shared_ptr<RootNode>& root_node) = 0;
  virtual bool SetSnapShot(const std::shared_ptr<RootNode>& root_node, const byte_string& buffer) = 0;

  void RecordDomStartTimePoint() {
    if (dom_start_time_point_.ToEpochDelta() == footstone::TimeDelta::Zero()) {
      dom_start_time_point_ = footstone::TimePoint::SystemNow();
    }
  }
  void RecordDomEndTimePoint() {
    if (dom_end_time_point_.ToEpochDelta() == footstone::TimeDelta::Zero() &&
        dom_start_time_point_.ToEpochDelta() != footstone::TimeDelta::Zero()) {
      dom_end_time_point_ = footstone::TimePoint::SystemNow();
    }
  }
  inline auto GetDomStartTimePoint() { return dom_start_time_point_; }
  inline auto GetDomEndTimePoint() { return dom_end_time_point_; }

 private:
  std::shared_ptr<TaskRunner> task_runner_;
  std::shared_ptr<Worker> worker_;

  footstone::TimePoint dom_start_time_point_;
  footstone::TimePoint dom_end_time_point_;
};

class DomManagerImpl : public DomManager {
 public:
  DomManagerImpl() = default;
  ~DomManagerImpl() override = default;

  void SetRenderManager(const std::weak_ptr<RenderManager>& render_manager) override;
  std::weak_ptr<RenderManager> GetRenderManager() override {
        return render_manager_;
  }
  std::shared_ptr<DomNode> GetNode(const std::weak_ptr<RootNode>& weak_root_node,
                                   uint32_t id) override;

  void CreateDomNodes(const std::weak_ptr<RootNode>& weak_root_node,
                      std::vector<std::shared_ptr<DomInfo>>&& nodes, bool needSortByIndex) override;
  void UpdateDomNodes(const std::weak_ptr<RootNode>& weak_root_node,
                      std::vector<std::shared_ptr<DomInfo>>&& nodes) override;
  void MoveDomNodes(const std::weak_ptr<RootNode>& weak_root_node,
                    std::vector<std::shared_ptr<DomInfo>>&& nodes) override;
  void DeleteDomNodes(const std::weak_ptr<RootNode>& weak_root_node,
                      std::vector<std::shared_ptr<DomInfo>>&& nodes) override;
  void UpdateAnimation(const std::weak_ptr<RootNode>& weak_root_node,
                       std::vector<std::shared_ptr<DomNode>>&& nodes) override;
  void EndBatch(const std::weak_ptr<RootNode>& root_node) override;
  // 返回0代表失败，正常id从1开始
  void AddEventListener(const std::weak_ptr<RootNode>& weak_root_node,
                        uint32_t dom_id,
                        const std::string& event_name,
                        uint64_t listener_id,
                        bool use_capture,
                        const EventCallback& cb) override;
  void RemoveEventListener(const std::weak_ptr<RootNode>& weak_root_node,
                           uint32_t id,
                           const std::string& name,
                           uint64_t listener_id) override;
  void CallFunction(const std::weak_ptr<RootNode>& weak_root_node,
                    uint32_t id,
                    const std::string& name,
                    const DomArgument& param,
                    const CallFunctionCallback& cb) override;
  void SetRootSize(const std::weak_ptr<RootNode>& weak_root_node, float width, float height) override;
  void DoLayout(const std::weak_ptr<RootNode>& weak_root_node) override;
  void PostTask(const Scene&& scene) override;
  uint32_t PostDelayedTask(const Scene&& scene, footstone::TimeDelta delay) override;
  void CancelTask(uint32_t id) override;

  byte_string GetSnapShot(const std::shared_ptr<RootNode>& root_node) override;
  bool SetSnapShot(const std::shared_ptr<RootNode>& root_node, const byte_string& buffer) override;
    
 private:
  friend class DomNode;

  uint32_t id_;
  std::shared_ptr<LayerOptimizedRenderManager> optimized_render_manager_;
  std::weak_ptr<RenderManager> render_manager_;
  std::unordered_map<uint32_t, std::shared_ptr<BaseTimer>> timer_map_;
};
}  // namespace dom
}  // namespace hippy
