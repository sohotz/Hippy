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

#include "renderer/native_render_manager.h"
#include "renderer/native_render_provider_napi.h"
#include "renderer/native_render_provider_manager.h"
#include "renderer/api/hippy_view_provider.h"
#include <cstdint>
#include <iostream>
#include <utility>
#include "footstone/logging.h"
#include "footstone/macros.h"
#include "dom/root_node.h"
#include "oh_napi/ark_ts.h"
#include "oh_napi/oh_measure_text.h"

#define USE_C_MEASURE 1

constexpr char kId[] = "id";
constexpr char kPid[] = "pId";
constexpr char kIndex[] = "index";
constexpr char kName[] = "name";
constexpr char kWidth[] = "width";
constexpr char kHeight[] = "height";
constexpr char kLeft[] = "left";
constexpr char kTop[] = "top";
constexpr char kProps[] = "props";
constexpr char kDeleteProps[] = "deleteProps";
constexpr char kFontStyle[] = "fontStyle";
constexpr char kLetterSpacing[] = "letterSpacing";
constexpr char kColor[] = "kColor";
constexpr char kFontSize[] = "fontSize";
constexpr char kFontFamily[] = "fontFamily";
constexpr char kFontWeight[] = "fontWeight";
constexpr char kTextDecorationLine[] = "textDecorationLine";
constexpr char kTextShadowOffset[] = "textShadowOffset";
constexpr char kTextShadowRadius[] = "textShadowRadius";
constexpr char kTextShadowColor[] = "textShadowColor";
constexpr char kLineHeight[] = "lineHeight";
constexpr char kTextAlign[] = "textAlign";
constexpr char kText[] = "text";
constexpr char kEnableScale[] = "enableScale";
constexpr char kNumberOfLines[] = "numberOfLines";

#define MARK_DIRTY_PROPERTY(STYLES, FIND_STYLE, NODE) \
  do {                                                \
    FOOTSTONE_DCHECK(NODE != nullptr);                \
    if (STYLES->find(FIND_STYLE) != STYLES->end()) {  \
      NODE->MarkDirty();                              \
      return;                                         \
    }                                                 \
  } while (0)

namespace hippy {
inline namespace render {
inline namespace native {

static bool IsMeasureNode(const std::string &name) {
  return name == "Text" || name == "TextInput";
}

std::atomic<uint32_t> NativeRenderManager::unique_native_render_manager_id_{1};
footstone::utils::PersistentObjectMap<uint32_t, std::shared_ptr<hippy::NativeRenderManager>> NativeRenderManager::persistent_map_;

StyleFilter::StyleFilter() {
  // 过滤属性列表说明：
  // 1 传递到ts的属性需要过滤，否则业务页面填一堆渲染用不到的属性，序列化/反序列化的时候严重影响性能；
  // 2 这个列表是从Android复制过来的，ts不好自动收集，删了nativeBackgroundAndroid；
  styles_ = {
    "backgroundColor",
    "borderColor",
    "borderRadius",
    "borderStyle",
    "borderWidth",
    "borderBottomColor",
    "borderBottomStyle",
    "borderBottomWidth",
    "borderBottomLeftRadius",
    "borderBottomRightRadius",
    "borderLeftColor",
    "borderLeftStyle",
    "borderLeftWidth",
    "linearGradient",
    "borderRightColor",
    "borderRightStyle",
    "borderRightWidth",
    "shadowColor",
    "shadowOffset",
    "shadowOffsetX",
    "shadowOffsetY",
    "shadowOpacity",
    "shadowRadius",
    "borderTopColor",
    "borderTopStyle",
    "borderTopWidth",
    "borderTopLeftRadius",
    "borderTopRightRadius",
    "zIndex",
    "backgroundImage",
    "backgroundPositionX",
    "backgroundPositionY",
    "backgroundSize",
    "capInsets",
    "defaultSource",
    "resizeMode",
    "tintColor",
    "tintColorBlendMode",
    "src",
    "fakeBold",
    "backgroundColor",
    "breakStrategy",
    "color",
    "ellipsizeMode",
    "enableScale",
    "fontFamily",
    "fontSize",
    "fontStyle",
    "fontWeight",
    "letterSpacing",
    "lineHeight",
    "lineSpacingExtra",
    "lineSpacingMultiplier",
    "numberOfLines",
    "opacity",
    "text",
    "textAlign",
    "textDecorationColor",
    "textDecorationLine",
    "textDecorationStyle",
    "textShadowColor",
    "textShadowOffset",
    "textShadowRadius",
    "verticalAlign",
    "width",
    "height",
    "left",
    "top",
    "visibility",
    "transform",
    "opacity",
    "overflow",
    "direction",
  };
}

NativeRenderManager::NativeRenderManager() : RenderManager("NativeRenderManager"),
      serializer_(std::make_shared<footstone::value::Serializer>()) {
  id_ = unique_native_render_manager_id_.fetch_add(1);
}

NativeRenderManager::~NativeRenderManager() {
  ArkTS arkTs(ts_env_);
  arkTs.DeleteReference(ts_render_provider_ref_);
  ts_render_provider_ref_ = 0;
  ts_env_ = 0;
  
  if (enable_ark_c_api_) {
    NativeRenderProviderManager::RemoveRenderProvider(id_);
  }
}

void NativeRenderManager::SetRenderDelegate(napi_env ts_env, bool enable_ark_c_api, napi_ref ts_render_provider_ref,
    std::set<std::string> &custom_views, std::set<std::string> &custom_measure_views, std::map<std::string, std::string> &mapping_views,
    std::string &bundle_path) {
  persistent_map_.Insert(id_, shared_from_this());
  ts_env_ = ts_env;
  ts_render_provider_ref_ = ts_render_provider_ref;
  CallRenderDelegateSetIdMethod(ts_env_, ts_render_provider_ref_, "setInstanceId", id_);
  custom_measure_views_ = custom_measure_views;
  
  enable_ark_c_api_ = enable_ark_c_api;
  if (enable_ark_c_api) {
    c_render_provider_ = std::make_shared<NativeRenderProvider>(id_, bundle_path);
    c_render_provider_->SetTsEnv(ts_env);
    NativeRenderProviderManager::AddRenderProvider(id_, c_render_provider_);
    c_render_provider_->RegisterCustomTsRenderViews(ts_env, ts_render_provider_ref, custom_views, mapping_views);
  }
  
  NativeRenderManager::GetStyleFilter();
}

void NativeRenderManager::InitDensity(double density) {
  density_ = static_cast<float>(density);
}

void NativeRenderManager::CreateRenderNode(std::weak_ptr<RootNode> root_node,
                                           std::vector<std::shared_ptr<hippy::dom::DomNode>>&& nodes) {
  if (enable_ark_c_api_) {
    CreateRenderNode_C(root_node, std::move(nodes));
  } else {
    CreateRenderNode_TS(root_node, std::move(nodes));
  }
}

void NativeRenderManager::CreateRenderNode_TS(std::weak_ptr<RootNode> root_node, std::vector<std::shared_ptr<DomNode>> &&nodes) {
  auto root = root_node.lock();
  if (!root) {
    return;
  }
  
  uint32_t root_id = root->GetId();

  serializer_->Release();
  serializer_->WriteHeader();

  auto len = nodes.size();
  footstone::value::HippyValue::HippyValueArrayType dom_node_array;
  dom_node_array.resize(len);
  for (uint32_t i = 0; i < len; i++) {
    const auto& render_info = nodes[i]->GetRenderInfo();
    footstone::value::HippyValue::HippyValueObjectType dom_node;
    dom_node[kId] = footstone::value::HippyValue(render_info.id);
    dom_node[kPid] = footstone::value::HippyValue(render_info.pid);
    dom_node[kIndex] = footstone::value::HippyValue(render_info.index);
    dom_node[kName] = footstone::value::HippyValue(nodes[i]->GetViewName());

    if (IsMeasureNode(nodes[i]->GetViewName())) {
#if USE_C_MEASURE
      auto weak_node = nodes[i]->weak_from_this();
      MeasureFunction measure_function = [WEAK_THIS, root_node, weak_node](float width, LayoutMeasureMode width_measure_mode,
                                                                           float height, LayoutMeasureMode height_measure_mode,
                                                                           void *layoutContext) -> LayoutSize {
        DEFINE_SELF(NativeRenderManager)
        if (!self) {
          return LayoutSize{0, 0};
        }
        int64_t result;
        self->DoMeasureText(root_node, weak_node, self->DpToPx(width), static_cast<int32_t>(width_measure_mode),
                            self->DpToPx(height), static_cast<int32_t>(height_measure_mode), result);
        LayoutSize layout_result;
        layout_result.width = self->PxToDp(static_cast<float>((int32_t)(0xFFFFFFFF & (result >> 32))));
        layout_result.height = self->PxToDp(static_cast<float>((int32_t)(0xFFFFFFFF & result)));
        return layout_result;
      };
#else
      int32_t id =  footstone::check::checked_numeric_cast<uint32_t, int32_t>(nodes[i]->GetId());
      MeasureFunction measure_function = [WEAK_THIS, root_id, id](float width, LayoutMeasureMode width_measure_mode,
                                                                  float height, LayoutMeasureMode height_measure_mode,
                                                                  void *layoutContext) -> LayoutSize {
        DEFINE_SELF(NativeRenderManager)
        if (!self) {
          return LayoutSize{0, 0};
        }
        int64_t result;
        self->CallNativeMeasureMethod(root_id, id, self->DpToPx(width), static_cast<int32_t>(width_measure_mode),
                                      self->DpToPx(height), static_cast<int32_t>(height_measure_mode), result);
        LayoutSize layout_result;
        layout_result.width = self->PxToDp(static_cast<float>((int32_t)(0xFFFFFFFF & (result >> 32))));
        layout_result.height = self->PxToDp(static_cast<float>((int32_t)(0xFFFFFFFF & result)));
        return layout_result;
      };
#endif
      nodes[i]->GetLayoutNode()->SetMeasureFunction(measure_function);
    } else if (IsCustomMeasureNode(nodes[i]->GetViewName())) {
      int32_t id =  footstone::check::checked_numeric_cast<uint32_t, int32_t>(nodes[i]->GetId());
      MeasureFunction measure_function = [WEAK_THIS, root_id, id](float width, LayoutMeasureMode width_measure_mode,
                                                                  float height, LayoutMeasureMode height_measure_mode,
                                                                  void *layoutContext) -> LayoutSize {
        DEFINE_SELF(NativeRenderManager)
        if (!self) {
          return LayoutSize{0, 0};
        }
        int64_t result;
        self->CallNativeCustomMeasureMethod(root_id, id, self->DpToPx(width), static_cast<int32_t>(width_measure_mode),
                                            self->DpToPx(height), static_cast<int32_t>(height_measure_mode), result);
        LayoutSize layout_result;
        layout_result.width = static_cast<float>((int32_t)(0xFFFFFFFF & (result >> 32)));
        layout_result.height = static_cast<float>((int32_t)(0xFFFFFFFF & result));
        return layout_result;
      };
      nodes[i]->GetLayoutNode()->SetMeasureFunction(measure_function);
    }

    footstone::value::HippyValue::HippyValueObjectType props;
    // 样式属性
    auto style = nodes[i]->GetStyleMap();
    auto iter = style->begin();
    auto style_filter = NativeRenderManager::GetStyleFilter();
    while (iter != style->end()) {
      if (style_filter->Enable(iter->first)) {
        props[iter->first] = *(iter->second);
      }
      iter++;
    }
    // 用户自定义属性
    auto dom_ext = *nodes[i]->GetExtStyle();
    iter = dom_ext.begin();
    while (iter != dom_ext.end()) {
      props[iter->first] = *(iter->second);
      iter++;
    }
  
    dom_node[kProps] = props;
    dom_node_array[i] = dom_node;
  }
  serializer_->WriteValue(HippyValue(dom_node_array));
  std::pair<uint8_t *, size_t> buffer_pair = serializer_->Release();
  
  CallNativeMethod("createNode", root->GetId(), buffer_pair);
}

void NativeRenderManager::CreateRenderNode_C(std::weak_ptr<RootNode> root_node, std::vector<std::shared_ptr<DomNode>> &&nodes) {
  auto root = root_node.lock();
  if (!root) {
    return;
  }
  
  uint32_t root_id = root->GetId();
  auto len = nodes.size();
  std::vector<std::shared_ptr<HRCreateMutation>> mutations;
  mutations.resize(len);
  for (uint32_t i = 0; i < len; i++) {
    const auto& render_info = nodes[i]->GetRenderInfo();
    auto m = std::make_shared<HRCreateMutation>();
    m->tag_ = render_info.id;
    m->parent_tag_ = render_info.pid;
    m->index_ = render_info.index;
    m->view_name_ = nodes[i]->GetViewName();

    if (IsMeasureNode(nodes[i]->GetViewName())) {
      auto weak_node = nodes[i]->weak_from_this();
      MeasureFunction measure_function = [WEAK_THIS, root_node, weak_node](float width, LayoutMeasureMode width_measure_mode,
                                                                           float height, LayoutMeasureMode height_measure_mode,
                                                                           void *layoutContext) -> LayoutSize {
        DEFINE_SELF(NativeRenderManager)
        if (!self) {
          return LayoutSize{0, 0};
        }
        int64_t result;
        self->DoMeasureText(root_node, weak_node, self->DpToPx(width), static_cast<int32_t>(width_measure_mode),
                            self->DpToPx(height), static_cast<int32_t>(height_measure_mode), result);
        LayoutSize layout_result;
        layout_result.width = self->PxToDp(static_cast<float>((int32_t)(0xFFFFFFFF & (result >> 32))));
        layout_result.height = self->PxToDp(static_cast<float>((int32_t)(0xFFFFFFFF & result)));
        return layout_result;
      };
      nodes[i]->GetLayoutNode()->SetMeasureFunction(measure_function);
    } else if (IsCustomMeasureNode(nodes[i]->GetViewName())) {
      int32_t id =  footstone::check::checked_numeric_cast<uint32_t, int32_t>(nodes[i]->GetId());
      MeasureFunction measure_function = [WEAK_THIS, root_id, id](float width, LayoutMeasureMode width_measure_mode,
                                                                  float height, LayoutMeasureMode height_measure_mode,
                                                                  void *layoutContext) -> LayoutSize {
        DEFINE_SELF(NativeRenderManager)
        if (!self) {
          return LayoutSize{0, 0};
        }
        int64_t result;
        self->CallNativeCustomMeasureMethod(root_id, id, self->DpToPx(width), static_cast<int32_t>(width_measure_mode),
                                            self->DpToPx(height), static_cast<int32_t>(height_measure_mode), result);
        LayoutSize layout_result;
        layout_result.width = static_cast<float>((int32_t)(0xFFFFFFFF & (result >> 32)));
        layout_result.height = static_cast<float>((int32_t)(0xFFFFFFFF & result));
        return layout_result;
      };
      nodes[i]->GetLayoutNode()->SetMeasureFunction(measure_function);
    } else if (IsCustomMeasureCNode(nodes[i]->GetViewName())) {
      int32_t id =  footstone::check::checked_numeric_cast<uint32_t, int32_t>(nodes[i]->GetId());
      MeasureFunction measure_function = [WEAK_THIS, root_id, id](float width, LayoutMeasureMode width_measure_mode,
                                                                  float height, LayoutMeasureMode height_measure_mode,
                                                                  void *layoutContext) -> LayoutSize {
        DEFINE_SELF(NativeRenderManager)
        if (!self) {
          return LayoutSize{0, 0};
        }
        LayoutSize layout_result = self->CallNativeCustomMeasureMethod_C(root_id, static_cast<uint32_t>(id), width, width_measure_mode, height, height_measure_mode);
        return layout_result;
      };
      nodes[i]->GetLayoutNode()->SetMeasureFunction(measure_function);
    }

    footstone::value::HippyValue::HippyValueObjectType props;
    // 样式属性
    auto style = nodes[i]->GetStyleMap();
    auto iter = style->begin();
    auto style_filter = NativeRenderManager::GetStyleFilter();
    while (iter != style->end()) {
      if (style_filter->Enable(iter->first)) {
        props[iter->first] = *(iter->second);
      }
      iter++;
    }
    // 用户自定义属性
    auto dom_ext = *nodes[i]->GetExtStyle();
    iter = dom_ext.begin();
    while (iter != dom_ext.end()) {
      props[iter->first] = *(iter->second);
      iter++;
    }
  
    m->props_ = props;
    auto parentNode = nodes[i]->GetParent();
    if (parentNode && parentNode->GetViewName() == "Text") {
      m->is_parent_text_ = true;
    }
    mutations[i] = m;
  }
  
  c_render_provider_->CreateNode(root_id, mutations);
}

void NativeRenderManager::UpdateRenderNode(std::weak_ptr<RootNode> root_node,
                                           std::vector<std::shared_ptr<DomNode>>&& nodes) {
  if (enable_ark_c_api_) {
    UpdateRenderNode_C(root_node, std::move(nodes));
  } else {
    UpdateRenderNode_TS(root_node, std::move(nodes));
  }
}

void NativeRenderManager::UpdateRenderNode_TS(std::weak_ptr<RootNode> root_node, std::vector<std::shared_ptr<DomNode>> &&nodes) {
  auto root = root_node.lock();
  if (!root) {
    return;
  }

  for (const auto &n : nodes) {
    auto node = root->GetNode(n->GetId());
    if (node == nullptr)
      continue;
    if (n->GetViewName() == "Text") {
      MarkTextDirty(root_node, n->GetId());
    }
  }

  serializer_->Release();
  serializer_->WriteHeader();

  auto len = nodes.size();
  footstone::value::HippyValue::HippyValueArrayType dom_node_array;
  dom_node_array.resize(len);
  for (uint32_t i = 0; i < len; i++) {
    const auto &render_info = nodes[i]->GetRenderInfo();
    footstone::value::HippyValue::HippyValueObjectType dom_node;
    dom_node[kId] = footstone::value::HippyValue(render_info.id);
    dom_node[kPid] = footstone::value::HippyValue(render_info.pid);
    dom_node[kIndex] = footstone::value::HippyValue(render_info.index);
    dom_node[kName] = footstone::value::HippyValue(nodes[i]->GetViewName());

    footstone::value::HippyValue::HippyValueObjectType diff_props;
    footstone::value::HippyValue::HippyValueArrayType del_props;
    auto diff = nodes[i]->GetDiffStyle();
    if (diff) {
      auto iter = diff->begin();
      while (iter != diff->end()) {
        FOOTSTONE_DCHECK(iter->second != nullptr);
        if (iter->second) {
          diff_props[iter->first] = *(iter->second);
        }
        iter++;
      }
    }
    auto del = nodes[i]->GetDeleteProps();
    if (del) {
      auto iter = del->begin();
      while (iter != del->end()) {
        del_props.emplace_back(footstone::value::HippyValue(*iter));
        iter++;
      }
    }
    dom_node[kProps] = diff_props;
    dom_node[kDeleteProps] = del_props;
    dom_node_array[i] = dom_node;
  }
  serializer_->WriteValue(HippyValue(dom_node_array));
  std::pair<uint8_t *, size_t> buffer_pair = serializer_->Release();

  CallNativeMethod("updateNode", root->GetId(), buffer_pair);
}

void NativeRenderManager::UpdateRenderNode_C(std::weak_ptr<RootNode> root_node, std::vector<std::shared_ptr<DomNode>> &&nodes) {
  auto root = root_node.lock();
  if (!root) {
    return;
  }

  for (const auto &n : nodes) {
    auto node = root->GetNode(n->GetId());
    if (node == nullptr)
      continue;
    if (n->GetViewName() == "Text") {
      MarkTextDirty(root_node, n->GetId());
    }
  }

  uint32_t root_id = root->GetId();
  auto len = nodes.size();
  std::vector<std::shared_ptr<HRUpdateMutation>> mutations;
  mutations.resize(len);
  for (uint32_t i = 0; i < len; i++) {
    const auto &render_info = nodes[i]->GetRenderInfo();
    auto m = std::make_shared<HRUpdateMutation>();
    m->tag_ = render_info.id;
    m->parent_tag_ = render_info.pid;
    m->index_ = render_info.index;
    m->view_name_ = nodes[i]->GetViewName();

    footstone::value::HippyValue::HippyValueObjectType diff_props;
    std::vector<std::string> del_props;
    auto diff = nodes[i]->GetDiffStyle();
    if (diff) {
      auto iter = diff->begin();
      while (iter != diff->end()) {
        FOOTSTONE_DCHECK(iter->second != nullptr);
        if (iter->second) {
          diff_props[iter->first] = *(iter->second);
        }
        iter++;
      }
    }
    auto del = nodes[i]->GetDeleteProps();
    if (del) {
      auto iter = del->begin();
      while (iter != del->end()) {
        del_props.emplace_back(*iter);
        iter++;
      }
    }
    m->props_ = diff_props;
    m->delete_props_ = del_props;
    mutations[i] = m;
  }
  c_render_provider_->UpdateNode(root_id, mutations);
}

void NativeRenderManager::MoveRenderNode(std::weak_ptr<RootNode> root_node,
                                         std::vector<std::shared_ptr<DomNode>> &&nodes) {
  if (enable_ark_c_api_) {
    MoveRenderNode_C(root_node, std::move(nodes));
  } else {
    MoveRenderNode_TS(root_node, std::move(nodes));
  }
}

void NativeRenderManager::MoveRenderNode_TS(std::weak_ptr<RootNode> root_node, std::vector<std::shared_ptr<DomNode>> &&nodes) {
  auto root = root_node.lock();
  if (!root) {
    return;
  }

  serializer_->Release();
  serializer_->WriteHeader();

  auto len = nodes.size();
  footstone::value::HippyValue::HippyValueArrayType dom_node_array;
  dom_node_array.resize(len);
  uint32_t pid;
  for (uint32_t i = 0; i < len; i++) {
    const auto &render_info = nodes[i]->GetRenderInfo();
    footstone::value::HippyValue::HippyValueObjectType dom_node;
    dom_node[kId] = footstone::value::HippyValue(render_info.id);
    dom_node[kPid] = footstone::value::HippyValue(render_info.pid);
    dom_node[kIndex] = footstone::value::HippyValue(render_info.index);
    dom_node_array[i] = dom_node;
    pid = render_info.pid;
  }
  serializer_->WriteValue(HippyValue(dom_node_array));
  std::pair<uint8_t *, size_t> buffer_pair = serializer_->Release();

  CallRenderDelegateMoveNodeMethod(ts_env_, ts_render_provider_ref_, "moveNode", root->GetId(), pid, buffer_pair);
}

void NativeRenderManager::MoveRenderNode_C(std::weak_ptr<RootNode> root_node, std::vector<std::shared_ptr<DomNode>> &&nodes) {
  auto root = root_node.lock();
  if (!root) {
    return;
  }

  uint32_t root_id = root->GetId();
  auto len = nodes.size();
  auto m = std::make_shared<HRMoveMutation>();
  std::vector<HRMoveNodeInfo> node_infos;
  for (uint32_t i = 0; i < len; i++) {
    const auto &render_info = nodes[i]->GetRenderInfo();
    m->parent_tag_ = render_info.pid;
    node_infos.push_back(HRMoveNodeInfo(render_info.id, render_info.index));
  }
  m->node_infos_ = node_infos;
  c_render_provider_->MoveNode(root_id, m);
}

void NativeRenderManager::DeleteRenderNode(std::weak_ptr<RootNode> root_node,
                                           std::vector<std::shared_ptr<DomNode>>&& nodes) {
  if (enable_ark_c_api_) {
    DeleteRenderNode_C(root_node, std::move(nodes));
  } else {
    DeleteRenderNode_TS(root_node, std::move(nodes));
  }
}

void NativeRenderManager::DeleteRenderNode_TS(std::weak_ptr<RootNode> root_node, std::vector<std::shared_ptr<DomNode>> &&nodes) {
  auto root = root_node.lock();
  if (!root) {
    return;
  }

  std::vector<uint32_t> ids;
  ids.resize(nodes.size());
  for (size_t i = 0; i < nodes.size(); i++) {
    ids[i] = nodes[i]->GetRenderInfo().id;
  }

  CallRenderDelegateDeleteNodeMethod(ts_env_, ts_render_provider_ref_, "deleteNode", root->GetId(), ids);
}

void NativeRenderManager::DeleteRenderNode_C(std::weak_ptr<RootNode> root_node, std::vector<std::shared_ptr<DomNode>> &&nodes) {
  auto root = root_node.lock();
  if (!root) {
    return;
  }

  uint32_t root_id = root->GetId();
  auto len = nodes.size();
  std::vector<std::shared_ptr<HRDeleteMutation>> mutations;
  mutations.resize(len);
  for (uint32_t i = 0; i < len; i++) {
    const auto &render_info = nodes[i]->GetRenderInfo();
    auto m = std::make_shared<HRDeleteMutation>();
    m->tag_ = render_info.id;
    mutations[i] = m;
  }
  c_render_provider_->DeleteNode(root_id, mutations);
}

void NativeRenderManager::UpdateLayout(std::weak_ptr<RootNode> root_node,
                                       const std::vector<std::shared_ptr<DomNode>>& nodes) {
  if (enable_ark_c_api_) {
    UpdateLayout_C(root_node, std::move(nodes));
  } else {
    UpdateLayout_TS(root_node, std::move(nodes));
  }
}

void NativeRenderManager::UpdateLayout_TS(std::weak_ptr<RootNode> root_node, const std::vector<std::shared_ptr<DomNode>> &nodes) {
  auto root = root_node.lock();
  if (!root) {
    return;
  }

  serializer_->Release();
  serializer_->WriteHeader();

  auto len = nodes.size();
  footstone::value::HippyValue::HippyValueArrayType dom_node_array;
  dom_node_array.resize(len);
  for (uint32_t i = 0; i < len; i++) {
    footstone::value::HippyValue::HippyValueObjectType dom_node;
    dom_node[kId] = footstone::value::HippyValue(nodes[i]->GetId());
    const auto &result = nodes[i]->GetRenderLayoutResult();
    dom_node[kWidth] = footstone::value::HippyValue(DpToPx(result.width));
    dom_node[kHeight] = footstone::value::HippyValue(DpToPx(result.height));
    dom_node[kLeft] = footstone::value::HippyValue(DpToPx(result.left));
    dom_node[kTop] = footstone::value::HippyValue(DpToPx(result.top));
    if (IsMeasureNode(nodes[i]->GetViewName())) {
      dom_node["paddingLeft"] = footstone::value::HippyValue(DpToPx(result.paddingLeft));
      dom_node["paddingTop"] = footstone::value::HippyValue(DpToPx(result.paddingTop));
      dom_node["paddingRight"] = footstone::value::HippyValue(DpToPx(result.paddingRight));
      dom_node["paddingBottom"] = footstone::value::HippyValue(DpToPx(result.paddingBottom));
    }
    dom_node_array[i] = dom_node;
  }
  serializer_->WriteValue(HippyValue(dom_node_array));
  std::pair<uint8_t *, size_t> buffer_pair = serializer_->Release();

  CallNativeMethod("updateLayout", root->GetId(), buffer_pair);
}

void NativeRenderManager::UpdateLayout_C(std::weak_ptr<RootNode> root_node, const std::vector<std::shared_ptr<DomNode>> &nodes) {
  auto root = root_node.lock();
  if (!root) {
    return;
  }

  uint32_t root_id = root->GetId();
  auto len = nodes.size();
  std::vector<std::shared_ptr<HRUpdateLayoutMutation>> mutations;
  mutations.resize(len);
  for (uint32_t i = 0; i < len; i++) {
    const auto &result = nodes[i]->GetRenderLayoutResult();
    auto m = std::make_shared<HRUpdateLayoutMutation>();
    m->tag_ = nodes[i]->GetId();
    m->left_ = result.left;
    m->top_ = result.top;
    m->width_ = result.width;
    m->height_ = result.height;
    if (IsMeasureNode(nodes[i]->GetViewName())) {
      m->padding_left_ = result.paddingLeft;
      m->padding_top_ = result.paddingTop;
      m->padding_right_ = result.paddingRight;
      m->padding_bottom_ = result.paddingBottom;
    }
    mutations[i] = m;
  }
  c_render_provider_->UpdateLayout(root_id, mutations);
}

void NativeRenderManager::MoveRenderNode(std::weak_ptr<RootNode> root_node,
                                         std::vector<int32_t>&& moved_ids,
                                         int32_t from_pid,
                                         int32_t to_pid,
                                         int32_t index) {
  if (enable_ark_c_api_) {
    MoveRenderNode_C(root_node, std::move(moved_ids), from_pid, to_pid, index);
  } else {
    MoveRenderNode_TS(root_node, std::move(moved_ids), from_pid, to_pid, index);
  }
}

void NativeRenderManager::MoveRenderNode_TS(std::weak_ptr<RootNode> root_node, std::vector<int32_t> &&moved_ids, int32_t from_pid,
                       int32_t to_pid, int32_t index) {
  auto root = root_node.lock();
  if (!root) {
    return;
  }

  CallRenderDelegateMoveNodeMethod(ts_env_, ts_render_provider_ref_, "moveNode2", root->GetId(), moved_ids, to_pid,
                                   from_pid, index);
}

void NativeRenderManager::MoveRenderNode_C(std::weak_ptr<RootNode> root_node, std::vector<int32_t> &&moved_ids, int32_t from_pid,
                      int32_t to_pid, int32_t index) {
  auto root = root_node.lock();
  if (!root) {
    return;
  }

  uint32_t root_id = root->GetId();
  auto m = std::make_shared<HRMove2Mutation>();
  std::vector<uint32_t> tags;
  for (uint32_t i = 0; i < moved_ids.size(); i++) {
    tags.push_back((uint32_t)moved_ids[i]);
  }
  m->tags_ = tags;
  m->to_parent_tag_ = (uint32_t)to_pid;
  m->from_parent_tag_ = (uint32_t)from_pid;
  m->index_ = index;
  c_render_provider_->MoveNode2(root_id, m);
}

void NativeRenderManager::EndBatch(std::weak_ptr<RootNode> root_node) {
  if (enable_ark_c_api_) {
    EndBatch_C(root_node);
  } else {
    EndBatch_TS(root_node);
  }
}

void NativeRenderManager::EndBatch_TS(std::weak_ptr<RootNode> root_node) {
  auto root = root_node.lock();
  if (root) {
    CallNativeMethod("endBatch", root->GetId());
  }
}

void NativeRenderManager::EndBatch_C(std::weak_ptr<RootNode> root_node) {
  auto root = root_node.lock();
  if (root) {
    uint32_t root_id = root->GetId();
    c_render_provider_->EndBatch(root_id);
  }
}

void NativeRenderManager::BeforeLayout(std::weak_ptr<RootNode> root_node){}

void NativeRenderManager::AfterLayout(std::weak_ptr<RootNode> root_node) {
  // 更新布局信息前处理事件监听
  HandleListenerOps(root_node, event_listener_ops_, "updateEventListener");
}

void NativeRenderManager::AddEventListener(std::weak_ptr<RootNode> root_node,
                                           std::weak_ptr<DomNode> dom_node, const std::string& name) {
  auto node = dom_node.lock();
  if (node) {
    event_listener_ops_[node->GetId()].emplace_back(ListenerOp(true, dom_node, name));
  }
}

void NativeRenderManager::RemoveEventListener(std::weak_ptr<RootNode> root_node,
                                              std::weak_ptr<DomNode> dom_node, const std::string& name) {
  auto node = dom_node.lock();
  if (node) {
    event_listener_ops_[node->GetId()].emplace_back(ListenerOp(false, dom_node, name));
  }
}

void NativeRenderManager::CallFunction(std::weak_ptr<RootNode> root_node,
                                       std::weak_ptr<DomNode> domNode, const std::string& name, const DomArgument& param,
                                        uint32_t cb_id) {
  if (enable_ark_c_api_) {
    CallFunction_C(root_node, domNode, name, param, cb_id);
  } else {
    CallFunction_TS(root_node, domNode, name, param, cb_id);
  }
}

void NativeRenderManager::CallFunction_TS(std::weak_ptr<RootNode> root_node, std::weak_ptr<DomNode> domNode,
                                          const std::string &name, const DomArgument &param, uint32_t cb_id) {
  auto root = root_node.lock();
  if (!root) {
    return;
  }

  std::shared_ptr<DomNode> node = domNode.lock();
  if (node == nullptr) {
    FOOTSTONE_LOG(ERROR) << "CallJs bad node";
    return;
  }

  std::vector<uint8_t> param_bson;
  param.ToBson(param_bson);

  void *new_buffer = malloc(param_bson.size());
  FOOTSTONE_DCHECK(new_buffer != nullptr);
  if (!new_buffer) {
    FOOTSTONE_LOG(ERROR) << "NativeRenderManager::CallFunction, malloc fail, size = " << param_bson.size();
    return;
  }
  memcpy(new_buffer, param_bson.data(), param_bson.size());
  auto buffer_pair = std::make_pair(reinterpret_cast<uint8_t *>(new_buffer), param_bson.size());

  CallRenderDelegateCallFunctionMethod(ts_env_, ts_render_provider_ref_, "callUIFunction", root->GetId(), node->GetId(),
                                       cb_id, name, buffer_pair);
}

void NativeRenderManager::CallFunction_C(std::weak_ptr<RootNode> root_node, std::weak_ptr<DomNode> domNode,
                                         const std::string &name, const DomArgument &param, uint32_t cb_id) {
  auto root = root_node.lock();
  if (!root) {
    return;
  }

  std::shared_ptr<DomNode> node = domNode.lock();
  if (node == nullptr) {
    FOOTSTONE_LOG(ERROR) << "CallJs bad node";
    return;
  }

  HippyValue hippy_value;
  param.ToObject(hippy_value);
  
  HippyValueArrayType params;
  if (hippy_value.IsArray()) {
    hippy_value.ToArray(params);
  }
  
  c_render_provider_->CallUIFunction(root->GetId(), node->GetId(), cb_id, name, params);
}

void NativeRenderManager::ReceivedEvent(std::weak_ptr<RootNode> root_node, uint32_t dom_id,
                                        const std::string& event_name, const std::shared_ptr<HippyValue>& params,
                                        bool capture, bool bubble) {
  auto manager = dom_manager_.lock();
  FOOTSTONE_DCHECK(manager != nullptr);
  if (manager == nullptr) return;

  auto root = root_node.lock();
  FOOTSTONE_DCHECK(root != nullptr);
  if (root == nullptr) return;

  std::vector<std::function<void()>> ops = {[weak_dom_manager = dom_manager_, weak_root_node = root_node, dom_id,
                                             params = std::move(params), use_capture = capture, use_bubble = bubble,
                                             event_name = std::move(event_name)] {
    auto manager = weak_dom_manager.lock();
    if (manager == nullptr) return;

    auto root = weak_root_node.lock();
    if (root == nullptr) return;

    auto node = manager->GetNode(root, dom_id);
    if (node == nullptr) return;

    auto event = std::make_shared<DomEvent>(event_name, node, use_capture, use_bubble, params);
    node->HandleEvent(event);
  }};
  manager->PostTask(Scene(std::move(ops)));
}

float NativeRenderManager::DpToPx(float dp) const { return dp * density_; }

float NativeRenderManager::PxToDp(float px) const { return px / density_; }

void NativeRenderManager::CallNativeMethod(const std::string& method, uint32_t root_id, const std::pair<uint8_t*, size_t>& buffer) {
  hippy::CallRenderDelegateMethod(ts_env_, ts_render_provider_ref_, method, root_id, buffer);
}

void NativeRenderManager::CallNativeMethod(const std::string& method, uint32_t root_id) {
  hippy::CallRenderDelegateMethod(ts_env_, ts_render_provider_ref_, method, root_id);
}

void NativeRenderManager::CallNativeMeasureMethod(const uint32_t root_id, const int32_t id, const float width, const int32_t width_mode,
                                                 const float height, const int32_t height_mode, int64_t& result) {
  CallRenderDelegateMeasureMethod(ts_env_, ts_render_provider_ref_, "measure", root_id, static_cast<uint32_t>(id), width, width_mode, height, height_mode, result);
}

void NativeRenderManager::CallNativeCustomMeasureMethod(const uint32_t root_id, const int32_t id, const float width,
                                                        const int32_t width_mode, const float height,
                                                        const int32_t height_mode, int64_t &result) {
  CallRenderDelegateMeasureMethod(ts_env_, ts_render_provider_ref_, "customMeasure", root_id, static_cast<uint32_t>(id),
                                  width, width_mode, height, height_mode, result);
}

LayoutSize NativeRenderManager::CallNativeCustomMeasureMethod_C(uint32_t root_id, uint32_t node_id,
    float width, LayoutMeasureMode width_measure_mode,
    float height, LayoutMeasureMode height_measure_mode) {
  return c_render_provider_->CustomMeasure(root_id, node_id, width, width_measure_mode, height, height_measure_mode);
}

std::string HippyValueToString(const HippyValue &value) {
  std::string sv;
  if (value.IsString()) {
    value.ToString(sv);
  } else if(value.IsDouble()) {
    double d;
    value.ToDouble(d);
    sv = std::to_string(d);
  } else if(value.IsInt32()) {
    int32_t i;
    value.ToInt32(i);
    sv = std::to_string(i);
  } else if(value.IsUInt32()) {
    uint32_t ui;
    value.ToUint32(ui);
    sv = std::to_string(ui);
  } else {
    FOOTSTONE_LOG(ERROR) << "Measure Text : unknow value type";
  }
  return sv;
}

void CollectAllProps(std::map<std::string, std::string> &propMap, std::shared_ptr<DomNode> node) {
  propMap.clear();
  // 样式属性
  auto style = node->GetStyleMap();
  auto iter = style->begin();
  while (iter != style->end()) {
    propMap[iter->first] = HippyValueToString(*(iter->second));
    iter++;
  }
  // 用户自定义属性
  auto dom_ext = *node->GetExtStyle();
  iter = dom_ext.begin();
  while (iter != dom_ext.end()) {
    propMap[iter->first] = HippyValueToString(*(iter->second));
    iter++;
  }
}

void NativeRenderManager::DoMeasureText(const std::weak_ptr<RootNode> root_node, const std::weak_ptr<hippy::dom::DomNode> dom_node,
                   const float width, const int32_t width_mode,
                   const float height, const int32_t height_mode, int64_t &result) {
  auto dom_manager = dom_manager_.lock();
  FOOTSTONE_DCHECK(dom_manager != nullptr);
  if (dom_manager == nullptr) {
    return;
  }

  auto root = root_node.lock();
  FOOTSTONE_DCHECK(root != nullptr);
  if (root == nullptr) {
    return;
  }
  
  auto node = dom_node.lock();
  if (node == nullptr) {
    return;
  }
    
  std::vector<std::shared_ptr<DomNode>> imageSpanNode;
  std::map<std::string, std::string> textPropMap;
  std::map<std::string, std::string> spanPropMap;
  CollectAllProps(textPropMap, node);

  float density = GetDensity();
  OhMeasureText measureInst;
  OhMeasureResult measureResult;

  measureInst.StartMeasure(textPropMap);

  if (node->GetChildCount() == 0) {
    measureInst.AddText(textPropMap);
  } else {
    for(uint32_t i = 0; i < node->GetChildCount(); i++) {
      auto child = node->GetChildAt(i);
      CollectAllProps(spanPropMap, child);
      if (child->GetViewName() == "Text") {
        measureInst.AddText(spanPropMap);
      } else if (child->GetViewName() == "Image") {
        if (spanPropMap.find("width") != spanPropMap.end() && spanPropMap.find("height") != spanPropMap.end()) {
          measureInst.AddImage(spanPropMap);
          imageSpanNode.push_back(child);
        } else {
          FOOTSTONE_LOG(ERROR) << "Measure Text : ImageSpan without size";
        }
      }
    }
  }
  measureResult = measureInst.EndMeasure(static_cast<int>(width), static_cast<int>(width_mode),
                                         static_cast<int>(height), static_cast<int>(height_mode), density);

  if(measureResult.spanPos.size() > 0 && measureResult.spanPos.size() == imageSpanNode.size()) {
    for(uint32_t i = 0; i < imageSpanNode.size(); i++) {
      double x = measureResult.spanPos[i].x;
      double y = measureResult.spanPos[i].y;
      // 把 c 测量到的imageSpan的位置，通知给ArkTS组件
      if (enable_ark_c_api_) {
        c_render_provider_->SpanPosition(root->GetId(), imageSpanNode[i]->GetId(), float(x), float(y));
      } else {
        CallRenderDelegateSpanPositionMethod(ts_env_, ts_render_provider_ref_, "spanPosition", root->GetId(), imageSpanNode[i]->GetId(), float(x), float(y));
      }
    }
  }
  result = static_cast<int64_t>(ceil(measureResult.width)) << 32 | static_cast<int64_t>(ceil(measureResult.height));
}

void NativeRenderManager::HandleListenerOps(std::weak_ptr<RootNode> root_node,
                                            std::map<uint32_t, std::vector<ListenerOp>>& ops,
                                            const std::string& method_name) {
  if (enable_ark_c_api_) {
    HandleListenerOps_C(root_node, ops, method_name);
  } else {
    HandleListenerOps_TS(root_node, ops, method_name);
  }
}

void NativeRenderManager::HandleListenerOps_TS(std::weak_ptr<RootNode> root_node, std::map<uint32_t, std::vector<ListenerOp>> &ops,
                          const std::string &method_name) {
  auto root = root_node.lock();
  if (!root) {
    return;
  }

  if (ops.empty()) {
    return;
  }

  footstone::value::HippyValue::HippyValueArrayType event_listener_ops;
  for (auto iter = ops.begin(); iter != ops.end(); ++iter) {
    footstone::value::HippyValue::HippyValueObjectType op;
    footstone::value::HippyValue::HippyValueObjectType events;

    const std::vector<ListenerOp> &listener_ops = iter->second;
    const auto len = listener_ops.size();
    std::vector<ListenerOp>::size_type index = 0;
    for (; index < len; index++) {
      const ListenerOp &listener_op = listener_ops[index];
      std::shared_ptr<DomNode> dom_node = listener_op.dom_node.lock();
      if (dom_node == nullptr) {
        break;
      }
      events[listener_op.name] = footstone::value::HippyValue(listener_op.add);
    }
    if (index == len) {
      op[kId] = footstone::value::HippyValue(iter->first);
      op[kProps] = events;
      event_listener_ops.emplace_back(op);
    }
  }

  ops.clear();
  if (event_listener_ops.empty()) {
    return;
  }

  serializer_->Release();
  serializer_->WriteHeader();
  serializer_->WriteValue(HippyValue(event_listener_ops));
  std::pair<uint8_t *, size_t> buffer_pair = serializer_->Release();
  CallNativeMethod(method_name, root->GetId(), buffer_pair);
}

void NativeRenderManager::HandleListenerOps_C(std::weak_ptr<RootNode> root_node, std::map<uint32_t, std::vector<ListenerOp>> &ops,
                         const std::string &method_name) {
  auto root = root_node.lock();
  if (!root) {
    return;
  }

  if (ops.empty()) {
    return;
  }

  uint32_t root_id = root->GetId();
  std::vector<std::shared_ptr<HRUpdateEventListenerMutation>> mutations;
  for (auto iter = ops.begin(); iter != ops.end(); ++iter) {
    auto m = std::make_shared<HRUpdateEventListenerMutation>();
    footstone::value::HippyValue::HippyValueObjectType events;

    const std::vector<ListenerOp> &listener_ops = iter->second;
    const auto len = listener_ops.size();
    std::vector<ListenerOp>::size_type index = 0;
    for (; index < len; index++) {
      const ListenerOp &listener_op = listener_ops[index];
      std::shared_ptr<DomNode> dom_node = listener_op.dom_node.lock();
      if (dom_node == nullptr) {
        break;
      }
      events[listener_op.name] = footstone::value::HippyValue(listener_op.add);
    }
    if (index == len) {
      m->tag_ = iter->first;
      m->props_ = events;
      mutations.push_back(m);
    }
  }
  ops.clear();
  if (mutations.empty()) {
    return;
  }
  c_render_provider_->UpdateEventListener(root_id, mutations);
}

void NativeRenderManager::MarkTextDirty(std::weak_ptr<RootNode> weak_root_node, uint32_t node_id) {
  auto root_node = weak_root_node.lock();
  FOOTSTONE_DCHECK(root_node);
  if (root_node) {
    auto node = root_node->GetNode(node_id);
    FOOTSTONE_DCHECK(node);
    if (node) {
      auto diff_style = node->GetDiffStyle();
      if (diff_style) {
        MARK_DIRTY_PROPERTY(diff_style, kFontStyle, node->GetLayoutNode());
        MARK_DIRTY_PROPERTY(diff_style, kLetterSpacing, node->GetLayoutNode());
        MARK_DIRTY_PROPERTY(diff_style, kColor, node->GetLayoutNode());
        MARK_DIRTY_PROPERTY(diff_style, kFontSize, node->GetLayoutNode());
        MARK_DIRTY_PROPERTY(diff_style, kFontFamily, node->GetLayoutNode());
        MARK_DIRTY_PROPERTY(diff_style, kFontWeight, node->GetLayoutNode());
        MARK_DIRTY_PROPERTY(diff_style, kTextDecorationLine, node->GetLayoutNode());
        MARK_DIRTY_PROPERTY(diff_style, kTextShadowOffset, node->GetLayoutNode());
        MARK_DIRTY_PROPERTY(diff_style, kTextShadowRadius, node->GetLayoutNode());
        MARK_DIRTY_PROPERTY(diff_style, kTextShadowColor, node->GetLayoutNode());
        MARK_DIRTY_PROPERTY(diff_style, kLineHeight, node->GetLayoutNode());
        MARK_DIRTY_PROPERTY(diff_style, kTextAlign, node->GetLayoutNode());
        MARK_DIRTY_PROPERTY(diff_style, kText, node->GetLayoutNode());
        MARK_DIRTY_PROPERTY(diff_style, kEnableScale, node->GetLayoutNode());
        MARK_DIRTY_PROPERTY(diff_style, kNumberOfLines, node->GetLayoutNode());
      }
    }
  }
}

bool NativeRenderManager::IsCustomMeasureNode(const std::string &name) {
  if (custom_measure_views_.find(name) != custom_measure_views_.end()) {
    return true;
  }
  return false;
}

bool NativeRenderManager::IsCustomMeasureCNode(const std::string &name) {
  auto custom_measure_c_views = HippyViewProvider::GetCustomMeasureViews();
  if (custom_measure_c_views.find(name) != custom_measure_c_views.end()) {
    return true;
  }
  return false;
}

void NativeRenderManager::RegisterNativeXComponentHandle(OH_NativeXComponent *nativeXComponent, uint32_t root_id, uint32_t node_id) {
  if (enable_ark_c_api_) {
    c_render_provider_->RegisterNativeXComponentHandle(nativeXComponent, root_id, node_id);
  }
}

void NativeRenderManager::DestroyRoot(uint32_t root_id) {
  if (enable_ark_c_api_) {
    c_render_provider_->DestroyRoot(root_id);
  }
}

bool NativeRenderManager::GetViewParent(uint32_t root_id, uint32_t node_id, uint32_t &parent_id, std::string &parent_view_type) {
  if (enable_ark_c_api_) {
    return c_render_provider_->GetViewParent(root_id, node_id, parent_id, parent_view_type);
  }
  return false;
}

bool NativeRenderManager::GetViewChildren(uint32_t root_id, uint32_t node_id, std::vector<uint32_t> &children_ids, std::vector<std::string> &children_view_types) {
  if (enable_ark_c_api_) {
    return c_render_provider_->GetViewChildren(root_id, node_id, children_ids, children_view_types);
  }
  return false;
}

void NativeRenderManager::CallViewMethod(uint32_t root_id, uint32_t node_id, const std::string &method, const std::vector<HippyValue> params, std::function<void(const HippyValue &result)> callback) {
  if (enable_ark_c_api_) {
    c_render_provider_->CallViewMethod(root_id, node_id, method, params, callback);
  }
}

}  // namespace native
}  // namespace render
}  // namespace hippy
