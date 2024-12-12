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
#include "oh_napi/oh_measure_text.h"
#include "renderer/text_measure/text_measurer.h"

namespace hippy {
inline namespace render {
inline namespace native {

class TextMeasureCache {
public:
  std::shared_ptr<OhMeasureText> used_ = nullptr;
  std::shared_ptr<OhMeasureText> builded_ = nullptr;
};

class TextMeasureManager {
public:
  TextMeasureManager() {}
  ~TextMeasureManager() {}
  
  void SaveNewOhMeasureText(uint32_t node_id, std::shared_ptr<OhMeasureText> text_measurer) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = text_measurer_map_.find(node_id);
    if (it != text_measurer_map_.end()) {
      auto cache = it->second;
      cache->builded_ = text_measurer;
    } else {
      auto cache = std::make_shared<TextMeasureCache>();
      cache->builded_ = text_measurer;
      text_measurer_map_[node_id] = cache;
    }
  }
  
  bool HasNewOhMeasureText(uint32_t node_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = text_measurer_map_.find(node_id);
    if (it != text_measurer_map_.end()) {
      auto cache = it->second;
      if (cache->builded_) {
        return true;
      }
    }
    return false;
  }
  
  std::shared_ptr<OhMeasureText> UseNewOhMeasureText(uint32_t node_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = text_measurer_map_.find(node_id);
    if (it != text_measurer_map_.end()) {
      auto cache = it->second;
      if (cache->builded_) {
        cache->used_ = cache->builded_;
        cache->builded_ = nullptr;
        return cache->used_;
      }
    }
    return nullptr;
  }
  
  std::shared_ptr<OhMeasureText> GetUsedOhMeasureText(uint32_t node_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = text_measurer_map_.find(node_id);
    if (it != text_measurer_map_.end()) {
      auto cache = it->second;
      return cache->used_;
    }
    return nullptr;
  }

  void EraseOhMeasureText(uint32_t node_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    text_measurer_map_.erase(node_id);
  }
  
private:
  std::map<uint32_t, std::shared_ptr<TextMeasureCache>> text_measurer_map_;
  std::mutex mutex_;
};

} // namespace native
} // namespace render
} // namespace hippy
