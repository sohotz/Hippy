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

#include "dom/dom_node.h"
#include "dom/dom_event.h"

namespace hippy {
inline namespace dom {

void DomEvent::StopPropagation() {
  prevent_bubble_ = true;
  prevent_capture_ = true;
}

uint32_t DomEvent::GetCurrentTargetId() {
  if (current_target_id_) {
    return current_target_id_;
  }
  auto dom_node = current_target_.lock();
  if (dom_node) {
    return dom_node->GetId();
  }
  return 0;
}

uint32_t DomEvent::GetTargetId() {
  if (target_id_) {
    return target_id_;
  }
  auto dom_node = target_.lock();
  if (dom_node) {
    return dom_node->GetId();
  }
  return 0;
}

}
}
