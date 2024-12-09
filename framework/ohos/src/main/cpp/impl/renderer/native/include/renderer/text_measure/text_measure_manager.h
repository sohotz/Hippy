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

#include <map>
#include <mutex>
#include "renderer/text_measure/text_measurer.h"

namespace hippy {
inline namespace render {
inline namespace native {

class TextMeasureManager {
public:
  TextMeasureManager() {}
  ~TextMeasureManager() {}
  
  void SetTextMeasurer(uint32_t node_id, std::shared_ptr<TextMeasurer> text_measurer) {
    std::lock_guard<std::mutex> lock(mutex_);
    text_measurer_map_[node_id] = text_measurer;
  }
  
  std::shared_ptr<TextMeasurer> GetTextMeasurer(uint32_t node_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    return text_measurer_map_[node_id];
  }
  
  void EraseTextMeasurer(uint32_t node_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    text_measurer_map_.erase(node_id);
  }
  
private:
  std::map<uint32_t, std::shared_ptr<TextMeasurer>> text_measurer_map_;
  std::mutex mutex_;
};

} // namespace native
} // namespace render
} // namespace hippy
