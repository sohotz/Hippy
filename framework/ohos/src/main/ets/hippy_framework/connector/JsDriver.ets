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
import resmgr from "@ohos.resourceManager";
import { HippyLibrary } from '../../hippy_library/HippyLibrary';
import { HippyEngineContext } from '../HippyEngineContext';
import { HippyModuleManager } from '../modules/HippyModuleManager';
import { NativeCallback } from './NativeCallback';

export class JsDriver {
  private libHippy: HippyLibrary
  public instanceId: number = -1

  public moduleManager: HippyModuleManager | null = null

  constructor(libHippy: HippyLibrary) {
    this.libHippy = libHippy
  }

  initialize(
    domManagerId: number,
    vfsId: number,
    callback: NativeCallback
  ) {
    // TODO(hot):
    let globalConfig = '{"Platform":{"OS":"android","PackageName":"com.openhippy.example","APILevel":31,"VersionName":"","Localization":{"country":"CN","language":"zh","direction":0},"NightMode":false,"SDKVersion":"3.0.112"},"HostConfig":{"appVersion":"","appName":"com.openhippy.example","extra":{},"url":"","nightMode":false},"Dimensions":{"screenPhysicalPixels":{"densityDpi":440.0,"width":392.7272644042969,"scale":2.75,"statusBarHeight":31.0,"navigationBarHeight":0.0,"fontScale":2.75,"height":916.3636474609375},"windowPhysicalPixels":{"densityDpi":440.0,"width":392.7272644042969,"scale":2.75,"statusBarHeight":31.0,"navigationBarHeight":0.0,"fontScale":2.75,"height":869.0908813476562}}}';

    this.instanceId = this.onCreate(
      globalConfig,
      false,
      true,
      false,
      callback,
      -1,
      domManagerId,
      vfsId,
      0
    )

    this.moduleManager = new HippyModuleManager(new HippyEngineContext())
  }

  onCreate(
    globalConfig: string,
    useLowMemoryMode: boolean,
    enableV8Serialization: boolean,
    isDevModule: boolean,
    callback: NativeCallback,
    groupId: number,
    domManagerId: number,
    //v8InitParams: ,
    vfsId: number,
    devtoolsId: number
  ): number {
    return this.libHippy?.JsDriver_CreateJsDriver(
      this,
      globalConfig,
      useLowMemoryMode,
      enableV8Serialization,
      isDevModule,
      callback,
      groupId,
      domManagerId,
      //v8InitParams: ,
      vfsId,
      devtoolsId
    )
  }

  onDestroy(
    useLowMemoryMode: boolean,
    isReload: boolean,
    callback: NativeCallback
  ) {
    this.libHippy?.JsDriver_DestroyJsDriver(
      this.instanceId,
      useLowMemoryMode,
      isReload,
      callback
    )
  }

  loadInstance(
    str: string
  ) {
    // TODO(hot):
    const arr = new Uint8Array([-1,13,111,34,4,110,97,109,101,34,4,68,101,109,111,34,2,105,100,73,20,34,6,112,97,114,97,109,115,111,34,13,109,115,103,70,114,111,109,78,97,116,105,118,101,34,41,72,105,32,106,115,32,100,101,118,101,108,111,112,101,114,44,32,73,32,99,111,109,101,32,102,114,111,109,32,110,97,116,105,118,101,32,99,111,100,101,33,34,10,115,111,117,114,99,101,80,97,116,104,34,21,118,117,101,50,47,105,110,100,101,120,46,97,110,100,114,111,105,100,46,106,115,123,2,123,3]);

    this.libHippy?.JsDriver_LoadInstance(this.instanceId, arr.buffer as ArrayBuffer)
  }

  unloadInstance(
    str: string
  ) {
    this.libHippy?.JsDriver_UnloadInstance(this.instanceId, str)
  }

  runScriptFromUri(
    uri: string,
    assetManager: resmgr.ResourceManager,
    canUseCodeCache: boolean,
    codeCacheDir: string,
    vfsId: number,
    callback: NativeCallback
  ): boolean {
    return this.libHippy?.JsDriver_RunScriptFromUri(
      this.instanceId,
      uri,
      assetManager,
      canUseCodeCache,
      codeCacheDir,
      vfsId,
      callback
    )
  }

  attachToRoot(
    rootId: number
  ) {
    this.libHippy?.JsDriver_SetRootNode(this.instanceId, rootId)
  }

  attachToDom(
    domManagerId: number
  ) {
    this.libHippy?.JsDriver_SetDomManager(this.instanceId, domManagerId)
  }

  onNativeInitEnd(
    startTime: number,
    endTime: number
  ) {
    this.libHippy?.JsDriver_OnNativeInitEnd(this.instanceId, startTime, endTime)
  }

  onFirstFrameEnd(
    time: number
  ) {
    this.libHippy?.JsDriver_OnFirstFrameEnd(this.instanceId, time)
  }

  onResourceLoadEnd(
    uri: string,
    startTime: number,
    endTime: number,
    retCode: number,
    errorMsg: string
  ) {
    this.libHippy?.JsDriver_OnResourceLoadEnd(this.instanceId, uri, startTime, endTime, retCode, errorMsg)
  }

  callFunction(
    action: string,
    callback: NativeCallback,
    buffer: ArrayBuffer
  ) {
    this.libHippy?.JsDriver_CallFunction(this.instanceId, action, callback, buffer)
  }

  public callNatives(
    moduleName: string,
    moduleFunc: string,
    callId: string,
    buffer: string
  ) {
    hilog.info(0x0000, 'hippy', 'JsDriver callNatives params, %{public}s - %{public}s - %{public}s - %{public}s',
      moduleName, moduleFunc, callId, buffer);

    // TODO(hot):
    // test code for native module
    moduleName = 'ConsoleModule'
    moduleFunc = 'log'
    let param1 = 'hello native module!'

    if (this.moduleManager) {
      let module = this.moduleManager.getModule(moduleName)
      if (module) {
        module.call(moduleFunc, param1, null)
      }
    }

    hilog.info(0x0000, 'hippy', 'JsDriver callNatives end');
  }
}
