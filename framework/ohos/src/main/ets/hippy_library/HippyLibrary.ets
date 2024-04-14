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
import { JsDriver } from '../hippy_framework/connector/JsDriver';
import { NativeCallback } from '../hippy_framework/connector/NativeCallback';
import { NativeRenderProvider } from '../renderer_native/NativeRenderProvider';
import { HippyAny } from '../support/common/HippyTypes';
import { FetchResourceCallback } from '../vfs/FetchResourceCallback';
import { ResourceDataHolder } from '../vfs/ResourceDataHolder';
import { VfsManager } from '../vfs/VfsManager';

export interface HippyLibrary {
  DomManager_SetRenderManager(instanceId: number, renderId: number): void;
  DomManager_CreateDomManager(): number;
  DomManager_DestroyDomManager(domManagerId: number): void;
  DomManager_CreateRoot(rootId: number, density: number): void;
  DomManager_DestroyRoot(rootId: number): void;
  DomManager_ReleaseRootResources(rootId: number): void;
  DomManager_SetDomManager(rootId: number, instanceId: number): void;

  JsDriver_CreateJsDriver(
    jsDriver: JsDriver,
    globalConfig: string,
    useLowMemoryMode: boolean,
    enableV8Serialization: boolean,
    isDevModule: boolean,
    callback: NativeCallback,
    groupId: number,
    domManagerId: number,
    v8ParamArray: Array<number> | null,
    vfsId: number,
    devtoolsId: number
  ): number;
  JsDriver_DestroyJsDriver(instanceId: number, useLowMemoryMode: boolean, isReload: boolean, callback: NativeCallback): void;
  JsDriver_LoadInstance(instanceId: number, buffer: ArrayBuffer): void;
  JsDriver_UnloadInstance(instanceId: number,str: string): void;
  JsDriver_RunScriptFromUri(
    instanceId: number,
    uri: string,
    assetManager: resmgr.ResourceManager,
    canUseCodeCache: boolean,
    codeCacheDir: string,
    vfsId: number,
    callback: NativeCallback
  ): boolean;
  JsDriver_SetRootNode(instanceId: number, rootId: number): void;
  JsDriver_SetDomManager(instanceId: number, domManagerId: number): void;
  JsDriver_OnNativeInitEnd(instanceId: number, startTime: number, endTime: number): void;
  JsDriver_OnFirstFrameEnd(instanceId: number, time: number): void;
  JsDriver_OnResourceLoadEnd(
    instanceId: number,
    uri: string,
    startTime: number,
    endTime: number,
    retCode: number,
    errorMsg: string): void;
  JsDriver_CallFunction(instanceId: number, action: string, callback: NativeCallback, buffer: ArrayBuffer): void;

  NativeRenderer_CreateNativeRenderManager(renderProvider: NativeRenderProvider, density: number): number;
  NativeRenderer_DestroyNativeRenderManager(instanceId: number): void;
  NativeRenderer_SetDomManager(instanceId: number, domManagerId: number): void;

  NativeRenderImpl_UpdateRootSize(instanceId: number, rootId: number, width: number, height: number): void;
  NativeRenderImpl_UpdateNodeSize(instanceId: number, rootId: number, nodeId: number, width: number, height: number, isSync: boolean): void;
  NativeRenderImpl_OnReceivedEvent(instanceId: number, rootId: number, nodeId: number, eventName: string, buffer: ArrayBuffer | null, useCapture: boolean, useBubble: boolean): void;
  NativeRenderImpl_DoCallBack(instanceId: number, result: number, functionName: string, rootId: number, nodeId: number, callbackId: number, buffer: ArrayBuffer | null): void;
  NativeRenderImpl_DoMeasureText(instanceId: number, props:string[] | null, width: number, widthMode: number, height: number, heightMode: number): number[];

  VfsManager_CreateVfs(vfsDelegate: VfsManager): number;
  VfsManager_DestroyVfs(instanceId: number): void;
  VfsManager_OnJniDelegateCallback(holder: ResourceDataHolder): void;
  VfsManager_OnJniDelegateInvokeProgress(callbackId: number, total: number, loaded: number): void;
  VfsManager_OnJniDelegateInvokeAsync(id: number, holder: ResourceDataHolder, callback: FetchResourceCallback): void;
  VfsManager_OnJniDelegateInvokeSync(id: number, holder: ResourceDataHolder): void;

}
