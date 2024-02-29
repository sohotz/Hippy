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
import { DefaultProcessor } from '../vfs/DefaultProcessor'
import { DomManager } from './connector/DomManager';
import { FrameworkProxy } from '../renderer_native/FrameworkProxy';
import { JsDriver } from './connector/JsDriver';
import { NativeRenderer } from './connector/NativeRenderer';
import { VfsManager } from '../vfs/VfsManager';
import { NativeRenderContext } from '../renderer_native/NativeRenderContext';
import { NativeRenderProvider } from '../renderer_native/NativeRenderProvider';
import { PixelUtil } from '../support/utils/PixelUtil';
import { HippyLibrary } from '../hippy_library/HippyLibrary';
import { HippyException } from '../support/common/HippyException';
import { HippyResourceLoader } from './adapter/http/HippyResourceLoader';

export class HippyEngineImpl implements HippyEngine, FrameworkProxy {
  private libHippy: HippyLibrary
  private resourceManager: resmgr.ResourceManager

  public vfsManager: VfsManager
  public jsDriver: JsDriver | null = null
  public domMgr: DomManager | null = null
  public nativeRenderProvider: NativeRenderProvider | null = null
  public nativeRenderer: NativeRenderer | null = null

  constructor(params: EngineInitParams) {
    this.libHippy = params.libHippy
    this.resourceManager = params.context.resourceManager
    this.vfsManager = new VfsManager(this.libHippy)
    const resourceLoader = new HippyResourceLoader()
    this.vfsManager.addProcessorAtFirst(new DefaultProcessor(resourceLoader))
  }

  initEngine(initCallbak: HippyEngineInitCallback): void {
    this.domMgr = new DomManager(this.libHippy)
    this.nativeRenderProvider = new NativeRenderProvider(this.libHippy)
    this.nativeRenderProvider.getNativeRenderImpl().setFrameworkProxy(this)
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

  getVfsManager(): VfsManager {
    return this.vfsManager;
  }

  getBundlePath(): string {
    // TODO(eksea)
    return "asset:/vue2";
  }

  getEngineId(): number {
    return 0;
  }

  onFirstViewAdded(): void {
  }

  handleNativeException(exception: HippyException): void {
  }

  updateDimension(width: number, height: number, shouldUseScreenDisplay: boolean, systemUiVisibilityChanged: boolean): void {
  }

  onSizeChanged(rootId: number, w: number, h: number, ow: number, oh: number): void {
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
