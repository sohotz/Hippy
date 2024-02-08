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
import { HippyEngineContext } from '../../../HippyEngineContext';
import { HippyNativeModuleBase } from '../HippyNativeModuleBase';
import HashMap from '@ohos.util.HashMap';

export class TimerModule extends HippyNativeModuleBase {
  public static readonly NAME = 'TimerModule';

  private timerInfo = new HashMap<string, number>();

  constructor(ctx: HippyEngineContext) {
    super(ctx)
  }

  setTimeout(timeout: number, callId: string, handler: Function): void {
    const timerId = setTimeout(handler, timeout);
    this.timerInfo.set(callId, timerId);
  }

  clearTimeout(callId: string): void {
    clearTimeout(this.timerInfo.get(callId));
    this.timerInfo.remove(callId);
  }

  setInterval(interval: number, callId: string, handler: Function): void {
    const timerId = setInterval(handler, interval);
    this.timerInfo.set(callId, timerId);
  }

  clearInterval(callId: string): void {
    clearInterval(this.timerInfo.get(callId));
    this.timerInfo.remove(callId);
  }
}
