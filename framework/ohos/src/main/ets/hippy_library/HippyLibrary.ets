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
import { NativeRenderProvider } from '../renderer_native/NativeRenderProvider';

export interface HippyLibrary {
  DomManager_SetRenderManager(instanceId: number, renderId: number);
  DomManager_CreateDomManager(): number;
  DomManager_DestroyDomManager(domManagerId: number);
  DomManager_CreateRoot(rootId: number, density: number);
  DomManager_DestroyRoot(rootId: number);
  DomManager_ReleaseRootResources(rootId: number);
  DomManager_SetDomManager(rootId: number, instanceId: number);

  JsDriver_CreateJsDriver(
    jsDriver: JsDriver,
    globalConfig: string,
    useLowMemoryMode: boolean,
    enableV8Serialization: boolean,
    isDevModule: boolean,
    callback: (result: number, reason: string) => void,
    groupId: number,
    domManagerId: number,
    //v8InitParams: ,
    vfsId: number,
    devtoolsId: number
  );
  JsDriver_DestroyJsDriver(instanceId: number, useLowMemoryMode: boolean, isReload: boolean);
  JsDriver_LoadInstance(instanceId: number, buffer: ArrayBuffer);
  JsDriver_UnloadInstance(instanceId: number,str: string);
  JsDriver_RunScriptFromUri(
    instanceId: number,
    uri: string,
    assetManager: resmgr.ResourceManager,
    canUseCodeCache: boolean,
    codeCacheDir: string,
    vfsId: number,
    callback: (result: number, reason: string) => void
  );
  JsDriver_SetRootNode(instanceId: number, rootId: number);
  JsDriver_SetDomManager(instanceId: number, domManagerId: number);
  JsDriver_OnNativeInitEnd(instanceId: number, startTime: number, endTime: number);
  JsDriver_OnFirstFrameEnd(instanceId: number, time: number);
  JsDriver_OnResourceLoadEnd(
    instanceId: number,
    uri: string,
    startTime: number,
    endTime: number,
    retCode: number,
    errorMsg: string);
  JsDriver_CallFunction(instanceId: number, action: string, buffer: string);

  NativeRenderer_CreateNativeRenderManager(renderProvider: NativeRenderProvider, density: number);
  NativeRenderer_DestroyNativeRenderManager(instanceId: number);
  NativeRenderer_GetNativeRendererInstance(instanceId: number);
  NativeRenderer_SetDomManager(instanceId: number, domManagerId: number);

  NativeRenderImpl_UpdateRootSize(instanceId: number, rootId: number, width: number, height: number);
  NativeRenderImpl_UpdateNodeSize(instanceId: number, rootId: number, nodeId: number, width: number, height: number, isSync: boolean);
  NativeRenderImpl_OnReceivedEvent(instanceId: number, rootId: number, nodeId: number, eventName: string, buffer: ArrayBuffer | null, useCapture: boolean, useBubble: boolean);
  NativeRenderImpl_DoCallBack(instanceId: number, result: number, functionName: string, rootId: number, nodeId: number, callbackId: number, buffer: ArrayBuffer | null);

  VfsManager_CreateVfs();
  VfsManager_DestroyVfs(instanceId: number);
}
