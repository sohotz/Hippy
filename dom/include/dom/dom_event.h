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

#include <any>
#include <memory>
#include <string>

#include "dom/dom_event.h"
#include "footstone/hippy_value.h"

namespace hippy {
inline namespace dom {

class DomNode;

enum class EventPhase: uint8_t {
  kNone = 0,
  kCapturePhase = 1,
  kAtTarget = 2,
  kBubblePhase =3
};

class DomEvent {
 public:
  using HippyValue = footstone::value::HippyValue;
  DomEvent(std::string type, std::weak_ptr<DomNode> target, bool can_capture, bool can_bubble,
           std::shared_ptr<HippyValue> value)
      : type_(std::move(type)),
        target_(target),
        current_target_(target),
        prevent_capture_(false),
        prevent_bubble_(false),
        can_capture_(can_capture),
        can_bubble_(can_bubble),
        value_(value) {}
  DomEvent(std::string type, std::weak_ptr<DomNode> target, bool can_capture = false, bool can_bubble = false)
      : DomEvent(std::move(type), target, can_capture, can_bubble, nullptr) {}
  DomEvent(std::string type, std::weak_ptr<DomNode> target, std::shared_ptr<HippyValue> value)
      : DomEvent(std::move(type), target, false, false, value) {}
  DomEvent(std::string type, uint32_t target_id, bool can_capture, bool can_bubble, std::shared_ptr<std::string> stringify_value)
      : DomEvent(std::move(type), std::weak_ptr<DomNode>(), can_capture, can_bubble, nullptr) {
        target_id_ = target_id;
        current_target_id_ = target_id;
        stringify_value_ = stringify_value;
      }
  void StopPropagation();
  inline void SetValue(std::shared_ptr<HippyValue> value) {
    value_ = value;
  }
  inline std::shared_ptr<HippyValue> GetValue() {
    return value_;
  }
  inline std::shared_ptr<std::string> GetStringifyValue() {
    return stringify_value_;
  }
  inline bool IsPreventCapture() {
    return prevent_capture_;
  }
  inline bool IsPreventBubble() {
    return prevent_bubble_;
  }
  inline bool CanCapture() {
    return can_capture_;
  }
  inline bool CanBubble() {
    return can_bubble_;
  }
  inline void SetCurrentTarget(std::weak_ptr<DomNode> current) {
    current_target_ = current;
    current_target_id_ = 0;
  }
  inline void SetCurrentTargetId(uint32_t id) {
    current_target_ = std::weak_ptr<DomNode>();
    current_target_id_ = id;
  }
  inline std::weak_ptr<DomNode> GetCurrentTarget() {
    return current_target_;
  }
  uint32_t GetCurrentTargetId();
  inline void SetTarget(std::weak_ptr<DomNode> target) {
    target_ = target;
    target_id_ = 0;
  }
  inline void SetTargetId(uint32_t id) {
    target_ = std::weak_ptr<DomNode>();
    target_id_ = id;
  }
  inline std::weak_ptr<DomNode> GetTarget() {
    return target_;
  }
  uint32_t GetTargetId();
  inline std::string GetType() {
    return type_;
  }
  inline EventPhase GetEventPhase() {
    return event_phase_;
  }
  inline void SetEventPhase(EventPhase event_phase) {
    event_phase_ = event_phase;
  }

 private:
  std::string type_;
  std::weak_ptr<DomNode> target_;
  uint32_t target_id_ = 0;
  std::weak_ptr<DomNode> current_target_;
  uint32_t current_target_id_ = 0;
  bool prevent_capture_;
  bool prevent_bubble_;
  bool can_capture_;
  bool can_bubble_;
  EventPhase event_phase_ = EventPhase::kNone;
  std::shared_ptr<HippyValue> value_;
  std::shared_ptr<std::string> stringify_value_ = nullptr;
};

}  // namespace dom
}  // namespace hippy
