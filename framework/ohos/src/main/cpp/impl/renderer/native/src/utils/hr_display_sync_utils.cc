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

#include "renderer/utils/hr_display_sync_utils.h"

namespace hippy {
inline namespace render {
inline namespace native {
const char *DO_FRAME = "frameUpdate";
std::map<int, std::vector<int>> sListeners;

void HRDisplaySyncUtils::registerDoFrameListener(uint32_t rendererId, uint32_t rootId) {
    sListeners[(int)rendererId].push_back((int)rootId);
    if (!sEnablePostFrame) {
        sEnablePostFrame = true;
        startPostFrame();
    }
}

void HRDisplaySyncUtils::unregisterDoFrameListener(uint32_t rendererId, uint32_t rootId) {
    auto it = sListeners.find((int)rendererId);
    if (it != sListeners.end()) {
        auto& roots = it->second;
        roots.erase(std::remove(roots.begin(), roots.end(), rootId), roots.end());
        if (roots.empty()) {
            sListeners.erase(it);
        }
    }

    if (sListeners.empty()) {
        sEnablePostFrame = false;
        stopPostFrame();
    }
}

void HRDisplaySyncUtils::handleDoFrameCallback() {
    for (const auto& entry : sListeners) {
        auto rendererId = entry.first;
        const std::vector<int>& rootList = entry.second;
        if (!rootList.empty()) {
            for (int rootId : rootList) {
                HREventUtils::SendRootEvent((uint32_t)rendererId, (uint32_t)rootId, DO_FRAME, nullptr);
            }
        }
    }
}

void HRDisplaySyncUtils::startPostFrame() {
    if (!s_BackDisplaySync) {
        s_BackDisplaySync = OH_DisplaySoloist_Create(true);
    }
    
    DisplaySoloist_ExpectedRateRange rateRange = {0, 120, 60};
    OH_DisplaySoloist_SetExpectedFrameRateRange(s_BackDisplaySync, &rateRange);
    OH_DisplaySoloist_Start(s_BackDisplaySync, FrameCallback, nullptr);
}

void HRDisplaySyncUtils::stopPostFrame() {
    if (s_BackDisplaySync) {
        OH_DisplaySoloist_Stop(s_BackDisplaySync);
        OH_DisplaySoloist_Destroy(s_BackDisplaySync);
        s_BackDisplaySync = nullptr;
    }
}

void HRDisplaySyncUtils::FrameCallback(long long timestamp, long long targetTimestamp, void *data) {
    handleDoFrameCallback();
}


} // namespace native
} // namespace render
} // namespace hippy