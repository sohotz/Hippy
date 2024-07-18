/*
 *
 * Tencent is pleased to support the open source community by making
 * Hippy available.
 *
 * Copyright (C) 2023 THL A29 Limited, a Tencent company.
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

#include "driver/napi/js_class_definition.h"
#include <ark_runtime/jsvm.h>

namespace hippy {
inline namespace driver {
inline namespace napi {

class JSHClassDefinition: public ClassDefinition {
 public:
  JSHClassDefinition(JSVM_Env env, JSVM_Value value);
  virtual ~JSHClassDefinition();

  inline auto GetValue() {
    JSVM_Value result = 0;
    OH_JSVM_GetReferenceValue(env_, value_ref_, &result);
    return result;
  }
 private:
//   JSVM_Value value_;
  JSVM_Env env_;
  JSVM_Ref value_ref_;
};

}
}
}
