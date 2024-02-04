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
import { HippyEngine } from '.';
import { DomManager } from './connector/DomManager';
import { JsDriver } from './connector/JsDriver';
import { NativeRenderer } from './connector/NativeRenderer';
import { VfsManager } from '../vfs/VfsManager';
import { NativeRenderContext } from '../renderer_native/NativeRenderContext';
import { NativeRenderImpl } from '../renderer_native/NativeRenderImpl';
import { NativeRenderProvider } from '../renderer_native/NativeRenderProvider';
import { PixelUtil } from '../support/utils/PixelUtil';
import { HippyRenderView } from '../renderer_native/HippyRenderView';
import { HippyLibrary } from '../hippy_library/HippyLibrary';

export class HippyEngineImpl implements HippyEngine {
  private libHippy: HippyLibrary
  private resourceManager: resmgr.ResourceManager

  public vfsManager: VfsManager
  public jsDriver: JsDriver
  public domMgr: DomManager
  public nativeRenderProvider: NativeRenderProvider
  public nativeRenderer: NativeRenderer

  constructor(
    libHippy: HippyLibrary,
    resourceManager: resmgr.ResourceManager) {
    this.libHippy = libHippy
    this.resourceManager = resourceManager

    this.vfsManager = new VfsManager(libHippy)
    this.domMgr = new DomManager(libHippy)
    this.nativeRenderProvider = new NativeRenderProvider(libHippy)
    this.nativeRenderer = new NativeRenderer(libHippy, this.nativeRenderProvider)
    this.nativeRenderer.attachToDom(this.domMgr.instanceId)
    this.domMgr.attachToRenderer(this.nativeRenderer.instanceId)
    let rootNodeId = this.getNativeRenderContext().getRootId()
    this.domMgr.createRootNode(rootNodeId, PixelUtil.getDensity())
    this.domMgr.attachToRoot(rootNodeId)
    this.jsDriver = new JsDriver(libHippy)
    this.jsDriver.initialize(this.domMgr.instanceId, this.vfsManager.instanceId, (result: number, reason: string) => {
      hilog.info(0x0000, 'hippy', 'jsDriver initialize callback, instanceId: %{public}d', this.jsDriver.instanceId);
      if (result != 0) {
        //return;
      }

      this.jsDriver.attachToDom(this.domMgr.instanceId)
      this.jsDriver.attachToRoot(rootNodeId)

      this.jsDriver.runScriptFromUri("asset:/vue2/vendor.android.js",
        this.resourceManager,
        false, "", this.vfsManager.instanceId, (result: number, reason: string) => {
          hilog.info(0x0000, 'hippy', 'jsDriver runScriptFromUri callback, instanceId: %{public}d', this.jsDriver.instanceId);
          if (result != 0) {
            //return;
          }

          this.loadModule()
        })
    })
  }

  loadModule() {
    this.loadJsModule()
  }

  loadJsModule() {
    this.jsDriver.runScriptFromUri("asset:/vue2/index.android.js",
      this.resourceManager,
      false, "", this.vfsManager.instanceId, (result: number, reason: string) => {
        hilog.info(0x0000, 'hippy', 'jsDriver runScriptFromUri callback, instanceId: %{public}d', this.jsDriver.instanceId);
        if (result != 0) {
          //return;
        }

      })
    this.jsDriver.loadInstance('')
  }

  public async runJsBundle(resourceManager: resmgr.ResourceManager) {
  }

  public getNativeRenderContext(): NativeRenderContext {
    return this.nativeRenderProvider.getNativeRenderImpl().getNativeRenderContext()
  }

  public getNativeRenderProvider(): NativeRenderProvider {
    return this.nativeRenderProvider
  }
}
