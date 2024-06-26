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
import { HippyEngineContext } from 'hippy';
import { HippyModulePromise } from 'hippy/src/main/ets/hippy_framework/modules/HippyModulePromise';
import { HippyNativeModuleBase } from 'hippy/src/main/ets/hippy_framework/modules/native';
import { HippyNativeModule } from 'hippy/src/main/ets/support/annotation/HippyNativeModule';
import { HippyAny } from 'hippy/src/main/ets/support/common/HippyTypes';
import { LogUtils } from 'hippy/src/main/ets/support/utils/LogUtils';


@HippyNativeModule({ name: "ExampleNativeModule" })
export class ExampleNativeModule extends HippyNativeModuleBase {
  public static readonly NAME = 'ExampleNativeModule'

  constructor(ctx: HippyEngineContext) {
    super(ctx)
  }

  public call(method: string, params: Array<HippyAny>, promise: HippyModulePromise): HippyAny {
    switch (method) {
      case 'test': {
        this.test();
        break;
      }
      case 'testPromise': {
        this.testPromise(params, promise);
        break;
      }
      default:
        super.call(method, params, promise);
    }
    return null;
  }

  public test() {
    LogUtils.i(ExampleNativeModule.NAME, 'module test');
  }

  public testPromise(params: Array<HippyAny>, promise: HippyModulePromise) {
      promise.resolve('test');
  }
}
