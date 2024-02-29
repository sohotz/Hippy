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
import { HippyAny, HippyModuleCallback } from '../../../../support/common/HippyTypes';
import { HippyEngineContext } from '../../../HippyEngineContext';
import { HippyNativeModuleBase } from '../HippyNativeModuleBase';

const CONSOLE_LOG_DOMAIN: number = 0x0002
const CONSOLE_LOG_TAG: string = 'hippy_console'

export class ConsoleModule extends HippyNativeModuleBase {
  public static readonly NAME = 'ConsoleModule'

  constructor(ctx: HippyEngineContext) {
    super(ctx)
  }

  call(method: string, params: HippyAny, callback: HippyModuleCallback | null): HippyAny {
    switch (method) {
      case 'log': {
        this.log(params as string)
        break;
      }
      case 'warn': {
        this.warn(params as string)
        break;
      }
      case 'info': {
        this.info(params as string)
        break;
      }
      case 'error': {
        this.error(params as string)
        break;
      }
      default:
        super.call(method, params, callback)
    }
    return null;
  }

  log(message: string) {
    hilog.debug(CONSOLE_LOG_DOMAIN, CONSOLE_LOG_TAG, '%{public}s', message)
  }

  warn(message: string) {
    hilog.warn(CONSOLE_LOG_DOMAIN, CONSOLE_LOG_TAG, '%{public}s', message)
  }

  info(message: string) {
    hilog.info(CONSOLE_LOG_DOMAIN, CONSOLE_LOG_TAG, '%{public}s', message)
  }

  error(message: string) {
    hilog.error(CONSOLE_LOG_DOMAIN, CONSOLE_LOG_TAG, '%{public}s', message)
  }
}
