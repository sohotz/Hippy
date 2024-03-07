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
  ModuleLoadStatus,
  V8InitParams
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
import { HippyAPIProvider } from './HippyAPIProvider';
import { HRRenderViewCreator } from '../renderer_native/components/HippyRenderRegisterMap';
import { HippyGlobalConfigs } from './HippyGlobalConfigs';
import { TimeMonitor } from '../support/utils/TimeMonitor';
import { TextUtils } from '../support/utils/TextUtils';
import { HippyAssetBundleLoader } from './bridge/bundleloader/HippyAssetBundleLoader';
import { HippyBundleLoader } from './bridge/bundleloader/HippyBundleLoader';
import { HippyFileBundleLoader } from './bridge/bundleloader/HippyFileBundleLoader';
import { HippyThirdPartyAdapter } from './adapter/thirdparty/HippyThirdPartyAdapter';
import { HippyEngineContextImpl } from './HippyEngineContextImpl';
import { HippyModuleManager } from './modules/HippyModuleManager';
import { LogUtils } from '../support/utils/LogUtils';

export class HippyEngineManagerImpl implements HippyEngine, FrameworkProxy {
  private libHippy: HippyLibrary
  private resourceManager: resmgr.ResourceManager | null = null

  private providers: Array<HippyAPIProvider> | null = null

  public vfsManager: VfsManager
  public jsDriver: JsDriver | null = null
  public domMgr: DomManager | null = null
  public nativeRenderProvider: NativeRenderProvider | null = null
  public nativeRenderer: NativeRenderer | null = null

  public mGlobalConfigs: HippyGlobalConfigs
  public mCoreBundleLoader: HippyBundleLoader | null = null;
  public mJsBundleLoader: HippyBundleLoader | null = null;
  public mDebugMode: boolean;
  public mServerBundleName: string;
  public enableV8Serialization: boolean;
  public mRemoteServerUrl: string;
  public mGroupId: number;
  public mMonitor: TimeMonitor;
  public mServerHost: string;
  public mThirdPartyAdapter: HippyThirdPartyAdapter | null;
  public v8InitParams: V8InitParams | null;
  public mEngineContext: HippyEngineContextImpl | null = null;
  public moduleManager: HippyModuleManager | null = null;

  constructor(params: EngineInitParams) {
    this.libHippy = params.libHippy
    this.libHippy = params.libHippy
    this.providers = params.providers
    this.vfsManager = new VfsManager(this.libHippy)
    const resourceLoader = new HippyResourceLoader()
    this.vfsManager.addProcessorAtFirst(new DefaultProcessor(resourceLoader))
    this.mGlobalConfigs = new HippyGlobalConfigs(params)
    this.mDebugMode = params.debugMode;
    this.mServerBundleName = params.debugMode ? params.debugBundleName : "";
    this.enableV8Serialization = params.enableV8Serialization;
    this.mServerHost = params.debugServerHost;
    this.mRemoteServerUrl = params.remoteServerUrl;
    this.mGroupId = params.groupId;
    this.mThirdPartyAdapter = params.thirdPartyAdapter;
    this.v8InitParams = params.v8InitParams;
    this.mMonitor = new TimeMonitor();


    if (!params.context) {
      return
    }
    this.resourceManager = params.context.resourceManager
    // create core bundle loader
    let coreBundleLoader: HippyBundleLoader | null = null;
    if (!TextUtils.isEmpty(params.coreJSAssetsPath)) {
        coreBundleLoader = new HippyAssetBundleLoader(params.context, params.coreJSAssetsPath,
                    !TextUtils.isEmpty(params.codeCacheTag), params.codeCacheTag);
    } else if (!TextUtils.isEmpty(params.coreJSFilePath)) {
        coreBundleLoader = new HippyFileBundleLoader(params.coreJSFilePath,
                 !TextUtils.isEmpty(params.codeCacheTag), params.codeCacheTag);
    }
    this.mCoreBundleLoader = coreBundleLoader;
    // this.mPreloadBundleLoader = preloadBundleLoader;
    // TODO
    // this.mProviders = params.providers;
    // this.mProcessors = params.processors;
  }

  initEngine(initCallbak: HippyEngineInitCallback): void {
    this.domMgr = new DomManager(this.libHippy)
    this.mEngineContext = new HippyEngineContextImpl(this.domMgr, this.mGlobalConfigs);
    this.moduleManager = new HippyModuleManager(this.mEngineContext)
    let customRenderViewCreatorMap: Map<string, HRRenderViewCreator> | null = null
    if (this.providers) {
      customRenderViewCreatorMap = new Map()
      for (let provider of this.providers) {
        let map = provider.getCustomRenderViewCreatorMap()
        if (map) {
          map.forEach((value, key) => {
            customRenderViewCreatorMap?.set(key, value)
          })
        }
      }
    }

    this.nativeRenderProvider = new NativeRenderProvider(this.libHippy, customRenderViewCreatorMap)
    this.nativeRenderProvider.getNativeRenderImpl().setFrameworkProxy(this)
    this.nativeRenderer = new NativeRenderer(this.libHippy, this.nativeRenderProvider)
    this.nativeRenderer.attachToDom(this.domMgr.instanceId)
    this.domMgr.attachToRenderer(this.nativeRenderer.instanceId)
    let rootNodeId = this.getNativeRenderContext().getRootId()
    this.domMgr.createRootNode(rootNodeId, PixelUtil.getDensity())
    this.domMgr.attachToRoot(rootNodeId)
    this.jsDriver = new JsDriver(this.libHippy, this.moduleManager)
    this.jsDriver.initialize(this.domMgr.instanceId, this.vfsManager.instanceId, (result: number, reason: string) => {
      if (!this.jsDriver || !this.domMgr || !this.vfsManager || !this.resourceManager) {
        return
      }
      hilog.info(0x0000, 'hippy', 'jsDriver initialize callback, instanceId: %{public}d', this.jsDriver.instanceId);
      if (result != 0) {
        //return;
      }

      this.jsDriver.attachToDom(this.domMgr.instanceId)
      this.jsDriver.attachToRoot(rootNodeId)

      this.jsDriver.runScriptFromUri(this.getBundlePath(),
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
    this.loadJsModule(loadParams, loadCallback)
  }

  getVfsManager(): VfsManager {
    return this.vfsManager;
  }

  getBundlePath(): string {
    if (this.mCoreBundleLoader) {
      return this.mCoreBundleLoader.getPath()
    }
    return ""
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

  loadJsModule(loadParams: ModuleLoadParams, loadCallback: HippyModuleLoadCallback) {
    if (!this.jsDriver || !this.vfsManager || !this.resourceManager) {
      return
    }
    let coreBundleLoader: HippyBundleLoader | null = null;
    if (!TextUtils.isEmpty(loadParams.jsAssetsPath)) {
      coreBundleLoader = new HippyAssetBundleLoader(this.mGlobalConfigs.getContext()!, loadParams.jsAssetsPath,
        !TextUtils.isEmpty(loadParams.codeCacheTag), loadParams.codeCacheTag);
    } else if (!TextUtils.isEmpty(loadParams.jsFilePath)) {
      coreBundleLoader = new HippyFileBundleLoader(loadParams.jsFilePath,
        !TextUtils.isEmpty(loadParams.codeCacheTag), loadParams.codeCacheTag);
    }
    this.mJsBundleLoader = coreBundleLoader;
    this.jsDriver.runScriptFromUri(this.mJsBundleLoader!.getPath(),
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
