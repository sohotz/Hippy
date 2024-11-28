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

#include "napi/native_api.h"
#include <cstdint>

EXTERN_C_START

typedef struct {
  void (*CreateDomNodes)(void *root_node, void *nodes[], size_t node_count,
                         bool need_sort_by_index);
  void (*UpdateDomNodes)(void *root_node, void *nodes[], size_t node_count);
  void (*MoveDomNodes)(void *root_node, void *nodes[], size_t node_count);
  void (*DeleteDomNodes)(void *root_node, void *nodes[], size_t node_count);
  void (*UpdateAnimation)(void *root_node, void *nodes[], size_t node_count);
  void (*EndBatch)(void *root_node);
  void (*AddEventListener)(void *root_node, uint32_t dom_id, const char *event_name,
                           uint64_t listener_id, bool use_capture, const void *cb);
  void (*RemoveEventListener)(void *root_node, uint32_t dom_id, const char *event_name,
                              uint64_t listener_id);
  void (*CallFunction)(void *root_node, uint32_t dom_id, const char *name, const void *param,
                       const void *cb);
  void (*SetRootSize)(void *root_node, float width, float height);
  void (*DoLayout)(void *root_node);
} HippyDomInterceptor;

uint32_t HippyCreateDomInterceptor(HippyDomInterceptor handler);
void HippyDestroyDomInterceptor(uint32_t dom_interceptor_id);

EXTERN_C_END
