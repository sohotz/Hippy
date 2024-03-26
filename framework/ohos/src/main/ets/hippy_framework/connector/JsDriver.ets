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
import resmgr from "@ohos.resourceManager";
import { HippyLibrary } from '../../hippy_library/HippyLibrary';
import { LogUtils } from '../../support/utils/LogUtils';
import { HippyCallNativeParams } from '../bridge/HippyCallNativeParams';
import { HippyModuleManager } from '../modules/HippyModuleManager';
import { NativeCallback } from './NativeCallback';

export class JsDriver {
  private libHippy: HippyLibrary
  public instanceId: number = -1
  public moduleManager: HippyModuleManager | null = null

  constructor(libHippy: HippyLibrary, moduleManager: HippyModuleManager) {
    this.libHippy = libHippy
    this.moduleManager = moduleManager;
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
      false,
      false,
      callback,
      -1,
      domManagerId,
      vfsId,
      0
    )
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
    buffer: Uint8Array,
    callback: NativeCallback
  ) {
    this.libHippy?.JsDriver_LoadInstance(this.instanceId, buffer.buffer as ArrayBuffer)
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
    buffer: ArrayBuffer
  ) {
    LogUtils.i('hippy', 'JsDriver callNatives params, '
     + moduleName + ' - '
     + moduleFunc + ' - '
     + callId + ' - '
     + buffer);

    if (this.moduleManager) {
      const callNativeParams = new HippyCallNativeParams(moduleName, moduleFunc, callId, buffer)
      this.moduleManager.callNatives(callNativeParams);
    }

    LogUtils.i('hippy', 'JsDriver callNatives end');
  }
}
