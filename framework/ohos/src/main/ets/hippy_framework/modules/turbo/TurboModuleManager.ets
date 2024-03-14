/* Tencent is pleased to support the open source community by making Hippy available.
 * Copyright (C) 2018 THL A29 Limited, a Tencent company. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
import { LogUtils } from '../../../support/utils/LogUtils';
import { HippyEngineContext } from '../../HippyEngineContext';
import { HippyModuleManager } from '../HippyModuleManager';
import { HippyNativeModuleBase } from '../native';


export class TurboModuleManager {
  public static TAG: string = "TurboModuleManager";
  private mModuleMap: Map<string, HippyNativeModuleBase> = new Map<string, HippyNativeModuleBase>();
  private mHippyEngineContext: HippyEngineContext;

  public constructor(context: HippyEngineContext) {
    this.mHippyEngineContext = context;
  }

  public get(name: string): HippyNativeModuleBase | null {
    LogUtils.d("TurboModuleManager", "get " + name);
    const module: HippyNativeModuleBase | undefined = this.mModuleMap.get(name);
    if (module !== null && module !== undefined) {
      return module;
    }

    const hippyModuleManager: HippyModuleManager = this.mHippyEngineContext.getModuleManager();
    // HippyNativeModuleInfo moduleInfo = hippyModuleManager.getNativeModuleInfo().get(name);
    // if (moduleInfo == null) {
    //   return null;
    // }

    //   try {
    //   moduleInfo.initialize();
    // }
    //
    // catch
    // (
    // Throwable
    // throwable
    // )
    // {
    //   LogUtils.e(TAG, throwable.getMessage());
    //   return null;
    // }

    // module = moduleInfo.getInstance();
    // mModuleMap.put(name, module);
    // return module;
    return null;
  }

  public install(runtimeId: number) {

  }
}
