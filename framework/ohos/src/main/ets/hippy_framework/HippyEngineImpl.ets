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
import hilog from '@ohos.hilog';
import { EngineInitParams,
  EngineInitStatus,
  HippyEngine, HippyEngineInitCallback,
  HippyModuleLoadCallback,
  ModuleLoadParams,
  ModuleLoadStatus
} from '.';
import { DomManager } from './connector/DomManager';
import { JsDriver } from './connector/JsDriver';
import { NativeRenderer } from './connector/NativeRenderer';
import { VfsManager } from '../vfs/VfsManager';
import { NativeRenderContext } from '../renderer_native/NativeRenderContext';
import { NativeRenderProvider } from '../renderer_native/NativeRenderProvider';
import { PixelUtil } from '../support/utils/PixelUtil';
import { HippyLibrary } from '../hippy_library/HippyLibrary';

export class HippyEngineImpl implements HippyEngine {
  private libHippy: HippyLibrary
  private resourceManager: resmgr.ResourceManager

  public vfsManager: VfsManager | null = null
  public jsDriver: JsDriver | null = null
  public domMgr: DomManager | null = null
  public nativeRenderProvider: NativeRenderProvider | null = null
  public nativeRenderer: NativeRenderer | null = null

  constructor(params: EngineInitParams) {
    this.libHippy = params.libHippy
    this.resourceManager = params.context.resourceManager
  }

  initEngine(initCallbak: HippyEngineInitCallback): void {
    this.vfsManager = new VfsManager(this.libHippy)
    this.domMgr = new DomManager(this.libHippy)
    this.nativeRenderProvider = new NativeRenderProvider(this.libHippy)
    this.nativeRenderer = new NativeRenderer(this.libHippy, this.nativeRenderProvider)
    this.nativeRenderer.attachToDom(this.domMgr.instanceId)
    this.domMgr.attachToRenderer(this.nativeRenderer.instanceId)
    let rootNodeId = this.getNativeRenderContext().getRootId()
    this.domMgr.createRootNode(rootNodeId, PixelUtil.getDensity())
    this.domMgr.attachToRoot(rootNodeId)
    this.jsDriver = new JsDriver(this.libHippy)
    this.jsDriver.initialize(this.domMgr.instanceId, this.vfsManager.instanceId, (result: number, reason: string) => {
      if (!this.jsDriver || !this.domMgr || !this.vfsManager) {
        return
      }
      hilog.info(0x0000, 'hippy', 'jsDriver initialize callback, instanceId: %{public}d', this.jsDriver.instanceId);
      if (result != 0) {
        //return;
      }

      this.jsDriver.attachToDom(this.domMgr.instanceId)
      this.jsDriver.attachToRoot(rootNodeId)

      this.jsDriver.runScriptFromUri("asset:/vue2/vendor.android.js",
        this.resourceManager,
        false, "", this.vfsManager.instanceId, (result: number, reason: string) => {
          if (!this.jsDriver) {
            return
          }
          hilog.info(0x0000, 'hippy', 'jsDriver runScriptFromUri callback, instanceId: %{public}d', this.jsDriver.instanceId);
          if (result != 0) {
            //return;
          }

          if (initCallbak) {
            initCallbak(EngineInitStatus.STATUS_OK, "")
          }
        })
    })
  }

  loadModule(loadParams: ModuleLoadParams, loadCallback: HippyModuleLoadCallback) {
    this.loadJsModule()
  }

  loadJsModule() {
    if (!this.jsDriver || !this.vfsManager) {
      return
    }
    this.jsDriver.runScriptFromUri("asset:/vue2/index.android.js",
      this.resourceManager,
      false, "", this.vfsManager.instanceId, (result: number, reason: string) => {
        if (!this.jsDriver) {
          return
        }
        hilog.info(0x0000, 'hippy', 'jsDriver runScriptFromUri callback, instanceId: %{public}d', this.jsDriver.instanceId);
        if (result != 0) {
          //return;
        }

      })
    this.jsDriver.loadInstance('')
  }

  public getNativeRenderContext(): NativeRenderContext {
    return this.nativeRenderProvider?.getNativeRenderImpl().getNativeRenderContext() as NativeRenderContext
  }

}
