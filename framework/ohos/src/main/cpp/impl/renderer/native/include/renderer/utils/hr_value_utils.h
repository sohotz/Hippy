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

#include <arkui/native_type.h>
#include "footstone/hippy_value.h"

namespace hippy {
inline namespace render {
inline namespace native {

using HippyValue = footstone::HippyValue;
using HippyValueObjectType = footstone::value::HippyValue::HippyValueObjectType;

class HRValueUtils {
public:
  static uint32_t GetUint32(const HippyValue &value) {
    double d = 0;
    bool result = value.ToDouble(d);
    if (result) {
      return static_cast<uint32_t>(d);
    }
    return 0;
  }

  static uint32_t GetUint32(const HippyValue &value, uint32_t defaultValue) {
    double d = 0;
    bool result = value.ToDouble(d);
    if (result) {
      return static_cast<uint32_t>(d);
    }
    return defaultValue;
  }

  static int32_t GetInt32(const HippyValue &value) {
    double d = 0;
    bool result = value.ToDouble(d);
    if (result) {
      return static_cast<int32_t>(d);
    }
    return 0;
  }

  static int32_t GetInt32(const HippyValue &value, int32_t defaultValue) {
    double d = 0;
    bool result = value.ToDouble(d);
    if (result) {
      return static_cast<int32_t>(d);
    }
    return defaultValue;
  }

  static float GetFloat(const HippyValue &value) {
    double d = 0;
    bool result = value.ToDouble(d);
    if (result) {
      return static_cast<float>(d);
    }
    return 0;
  }

  static float GetFloat(const HippyValue &value, float defaultValue) {
    double d = 0;
    bool result = value.ToDouble(d);
    if (result) {
      return static_cast<float>(d);
    }
    return defaultValue;
  }

  static double GetDouble(const HippyValue &value) {
    double d = 0;
    bool result = value.ToDouble(d);
    if (result) {
      return d;
    }
    return 0;
  }

  static double GetDouble(const HippyValue &value, double defaultValue) {
    double d = 0;
    bool result = value.ToDouble(d);
    if (result) {
      return d;
    }
    return defaultValue;
  }

  static std::string GetString(const HippyValue &value) {
    std::string str;
    bool result = value.ToString(str);
    if (result) {
      return str;
    }
    return "";
  }
  
  static bool GetBool(const HippyValue &value, bool defaultValue) {
    bool b = false;
    bool result = value.ToBoolean(b);
    if (result) {
      return b;
    }
    return defaultValue;
  }

};

} // namespace native
} // namespace render
} // namespace hippy
