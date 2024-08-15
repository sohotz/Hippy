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
import { HippyEngineContext, HippyRootView } from 'hippy';
import { HippyModulePromise } from 'hippy/src/main/ets/hippy_framework/modules/HippyModulePromise';
import { HippyNativeModuleBase } from 'hippy/src/main/ets/hippy_framework/modules/native';
import { HRPosition } from 'hippy/src/main/ets/renderer_native/utils/HRTypes';
import { HippyNativeModule } from 'hippy/src/main/ets/support/annotation/HippyNativeModule';
import { HippyAny, HippyMap } from 'hippy/src/main/ets/support/common/HippyTypes';
import { LogUtils } from 'hippy/src/main/ets/support/utils/LogUtils';
import { buildExampleMenuView, ExampleMenuView } from './ExampleMenuView';


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
      case 'showMenuWith': {
        this.showMenuWith(params, promise);
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

  public showMenuWith(params: Array<HippyAny>, promise: HippyModulePromise) {
    // let map = params[0] as HippyMap
    // let instanceId: string = map.get('instanceId') as string
    // let fromView: string = map.get('fromView') as string
    // let rootId = parseInt(instanceId)
    // let fromViewId = parseInt(fromView)

    // Test code
    let rootId = 10
    let fromViewId = 367

    let nativeRender = this.ctx.getNativeRender()
    if (!nativeRender) {
      return
    }
    let pos = nativeRender.getViewPositionInRoot(rootId, fromViewId) as HRPosition
    let view = new ExampleMenuView()
    let viewId = nativeRender.addBizViewInRoot(rootId, pos, wrapBuilder(buildExampleMenuView), view)

    view.nativeRender = nativeRender
    view.rootId = rootId
    view.viewId = viewId
  }
}
