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

#include "renderer/arkui/list_node.h"
#include "renderer/arkui/native_node_api.h"

namespace hippy {
inline namespace render {
inline namespace native {

ListNode::ListNode()
    : ArkUINode(NativeNodeApi::GetInstance()->createNode(ArkUI_NodeType::ARKUI_NODE_LIST)) {
}

ListNode::~ListNode() {}

void ListNode::AddChild(ArkUINode &child) {
  MaybeThrow(NativeNodeApi::GetInstance()->addChild(nodeHandle_, child.GetArkUINodeHandle()));
}

void ListNode::RemoveChild(ArkUINode &child) {
  MaybeThrow(NativeNodeApi::GetInstance()->removeChild(nodeHandle_, child.GetArkUINodeHandle()));
}

} // namespace native
} // namespace render
} // namespace hippy
