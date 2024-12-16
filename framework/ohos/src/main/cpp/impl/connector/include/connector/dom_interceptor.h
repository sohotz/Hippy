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
  char context[128];
  void (*CreateDomNodes)(const char* context, uint32_t dom_interceptor_id, const char* nodes, bool need_sort_by_index);
  void (*UpdateDomNodes)(const char* context, uint32_t dom_interceptor_id, const char* nodes);
  void (*MoveDomNodes)(const char* context, uint32_t dom_interceptor_id, const char* nodes);
  void (*DeleteDomNodes)(const char* context, uint32_t dom_interceptor_id, const char* nodes);
  void (*UpdateAnimation)(const char* context, uint32_t dom_interceptor_id, const char* nodes);
  void (*EndBatch)(const char* context, uint32_t dom_interceptor_id);
  void (*AddEventListener)(const char* context, uint32_t dom_interceptor_id, uint32_t dom_id, const char *event_name,
                           uint64_t listener_id, bool use_capture);
  void (*RemoveEventListener)(const char* context, uint32_t dom_interceptor_id, uint32_t dom_id, const char *event_name,
                              uint64_t listener_id);
  void (*CallFunction)(const char* context, uint32_t dom_interceptor_id, uint32_t dom_id, const char *name, const void *param,
                       const void *cb);
  void (*SetRootSize)(const char* context, uint32_t dom_interceptor_id, float width, float height);
  void (*DoLayout)(const char* context, uint32_t dom_interceptor_id);
} HippyDomInterceptor;

uint32_t HippyCreateDomInterceptor(HippyDomInterceptor handler);
void HippyDomInterceptorSendEvent(uint32_t dom_interceptor_id, uint32_t root_id, const char *param);
void HippyDestroyDomInterceptor(uint32_t dom_interceptor_id);
EXTERN_C_END
