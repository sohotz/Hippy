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

#include "renderer/components/base_view.h"
#include "renderer/arkui/swiper_node.h"

namespace hippy {
inline namespace render {
inline namespace native {

class PagerView : public BaseView, public SwiperNodeDelegate {
  public:
  PagerView(std::shared_ptr<NativeRenderContext> &ctx);
  ~PagerView();

  SwiperNode &GetLocalRootArkUINode() override;
  bool SetProp(const std::string &propKey, const HippyValue &propValue) override;

  void OnChildInserted(std::shared_ptr<BaseView> const &childView, int32_t index) override;
  void OnChildRemoved(std::shared_ptr<BaseView> const &childView) override;

  void OnChange(const int32_t &index) override;
  void OnAnimationStart(const int32_t &currentIndex, const int32_t &targetIndex,
                        const float_t &currentOffset, const float_t &targetOffset,
                        const float_t &swipeVelocity) override;
  void OnAnimationEnd(const int32_t &currentIndex, const float_t &finalOffset) override;
  void OnContentDidScroll(const int32_t &swiperPageIndex, const int32_t &windowPageIndex,
                          const float_t &pageMoveRatio, const float_t &pageAxisSize) override;
  void OnGestureSwipe(const int32_t &swiperPageIndex,
                      const float_t &elementOffsetFromStart) override;
  void OnTouchIntercept(const int32_t &eventEnum) override;
  void OnNodeTouchEvent(const ArkUI_UIInputEvent *inputEvent) override;

  void WangzCheck(const HippyValue &value);

  void Call(const std::string &method, const std::vector<HippyValue> params,
            std::function<void(const HippyValue &result)> callback) override;

  int initialPage_ = 0;
  int index_ = 0;
  float prevMargin_ = 0;
  float nextMargin_ = 0;
  bool disableSwipe_ = true;
  bool vertical_ = false;

  private:
  SwiperNode swiperNode_;
  void SendScrollStateChangeEvent(const std::string &state);
};

} // namespace native
} // namespace render
} // namespace hippy
