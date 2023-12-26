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

import hilog from '@ohos.hilog';
import { HippyLogAdapter, HippyLogSeverity } from './HippyLogAdapter';

const HIPPY_LOG_DOMAIN: number = 0x0003

export class DefaultLogAdapter implements HippyLogAdapter {
  onReceiveLogMessage(level: number, tag: string, msg: string) {
    switch (level) {
      case HippyLogSeverity.LOG_SEVERITY_INFO:
        hilog.info(HIPPY_LOG_DOMAIN, tag, msg)
        break;
      case HippyLogSeverity.LOG_SEVERITY_WARNING:
        hilog.warn(HIPPY_LOG_DOMAIN, tag, msg)
        break;
      case HippyLogSeverity.LOG_SEVERITY_ERROR:
        // fall through
      case HippyLogSeverity.LOG_SEVERITY_FATAL:
        hilog.error(HIPPY_LOG_DOMAIN, tag, msg)
        break;
      case HippyLogSeverity.LOG_SEVERITY_DEBUG:
        // fall through
      default:
        hilog.debug(HIPPY_LOG_DOMAIN, tag, msg)
    }
  }
}
