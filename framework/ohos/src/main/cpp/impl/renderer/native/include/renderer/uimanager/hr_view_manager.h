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
#include <js_native_api.h>
#include <js_native_api_types.h>
#include <map>
#include "footstone/serializer.h"
#include "renderer/components/custom_view.h"
#include "renderer/components/root_view.h"
#include "renderer/uimanager/hr_mutation.h"

namespace hippy {
inline namespace render {
inline namespace native {

using HippyValueObjectType = footstone::value::HippyValue::HippyValueObjectType;

class HRViewManager {
public:
  HRViewManager(uint32_t instance_id, uint32_t root_id, std::shared_ptr<NativeRender> &native_render,
    napi_env ts_env, napi_ref ts_render_provider_ref,
    std::set<std::string> &custom_views, std::map<std::string, std::string> &mapping_views);
  ~HRViewManager() = default;
  
  void AttachToNativeXComponent(OH_NativeXComponent* nativeXComponent, uint32_t node_id);

  int GetRootTag() {
    return (int)root_id_;
  }

  void AddMutations(std::shared_ptr<HRMutation> &m);

  void ApplyMutations();
  void ApplyMutation(std::shared_ptr<HRMutation> &m);
  
  std::shared_ptr<BaseView> CreateRenderView(uint32_t tag, std::string &view_name, bool is_parent_text);
  void RemoveRenderView(uint32_t tag);
  void RemoveFromRegistry(std::shared_ptr<BaseView> &renderView);
  void InsertSubRenderView(uint32_t parentTag, std::shared_ptr<BaseView> &childView, int32_t index);
  void MoveRenderView(std::vector<HRMoveNodeInfo> nodeInfos, uint32_t parentTag);
  void Move2RenderView(std::vector<uint32_t> tags, uint32_t newParentTag, uint32_t oldParentTag, int index);
  void UpdateProps(std::shared_ptr<BaseView> &view, const HippyValueObjectType &props, const std::vector<std::string> &deleteProps = std::vector<std::string>());
  void UpdateProps(uint32_t tag, const HippyValueObjectType &props, const std::vector<std::string> &deleteProps = std::vector<std::string>());
  void UpdateEventListener(uint32_t tag, HippyValueObjectType &props);
  bool CheckRegisteredEvent(uint32_t tag, std::string &eventName);
  void SetRenderViewFrame(uint32_t tag, const HRRect &frame, const HRPadding &padding);

  void CallViewMethod(uint32_t tag, const std::string &method, const std::vector<HippyValue> params,
                      std::function<void(const HippyValue &result)> callback);
  
  LayoutSize CallCustomMeasure(uint32_t tag,
    float width, LayoutMeasureMode width_measure_mode,
    float height, LayoutMeasureMode height_measure_mode);

  uint64_t AddEndBatchCallback(const EndBatchCallback &cb);
  void RemoveEndBatchCallback(uint64_t cbId);
  void NotifyEndBatchCallbacks();
  
  bool GetViewParent(uint32_t node_id, uint32_t &parent_id, std::string &parent_view_type);
  bool GetViewChildren(uint32_t node_id, std::vector<uint32_t> &children_ids, std::vector<std::string> &children_view_types);

private:
  void MaybeAttachRootNode(OH_NativeXComponent *nativeXComponent, bool isRoot, std::shared_ptr<BaseView> &view);
  void MaybeDetachRootNode(OH_NativeXComponent *nativeXComponent, bool isRoot, std::shared_ptr<BaseView> &view);
  
  bool IsCustomTsRenderView(std::string &view_name);
  std::shared_ptr<BaseView> CreateCustomTsRenderView(uint32_t tag, std::string &view_name, bool is_parent_text);
  void UpdateCustomTsProps(std::shared_ptr<BaseView> &view, const HippyValueObjectType &props, const std::vector<std::string> &deleteProps = std::vector<std::string>());
  void UpdateCustomTsEventListener(uint32_t tag, HippyValueObjectType &props);
  void SetCustomTsRenderViewFrame(uint32_t tag, const HRRect &frame, const HRPadding &padding);
  
  std::shared_ptr<BaseView> CreateCustomRenderView(uint32_t tag, std::string &view_name, bool is_parent_text);
  
  std::shared_ptr<NativeRenderContext> ctx_;
  uint32_t root_id_;
  std::unordered_map<uint32_t, OH_NativeXComponent *> nativeXComponentMap_;
  std::shared_ptr<RootView> root_view_;
  std::map<uint32_t, std::shared_ptr<BaseView>> view_registry_;
  std::vector<std::shared_ptr<HRMutation>> mutations_;
  uint64_t end_batch_callback_id_count_ = 0;
  std::map<uint64_t, EndBatchCallback> end_batch_callback_map_;
  
  std::shared_ptr<footstone::value::Serializer> serializer_;
  
  std::map<std::string, std::string> mapping_render_views_;
  std::set<std::string> custom_ts_render_views_;
  napi_env ts_env_ = nullptr;
  napi_ref ts_render_provider_ref_ = nullptr;
};

} // namespace native
} // namespace render
} // namespace hippy
