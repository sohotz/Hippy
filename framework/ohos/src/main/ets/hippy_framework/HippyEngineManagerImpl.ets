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
import {
  BackPressHandler,
  EngineInitParams,
  EngineInitStatus,
  HippyEngine,
  HippyEngineInitCallback,
  HippyModuleDestroyCallback,
  HippyModuleLoadCallback,
  ModuleLoadParams,
  V8InitParams
} from '.';
import { DefaultProcessor } from '../vfs/DefaultProcessor'
import { DomManager } from './connector/DomManager';
import { FrameworkProxy } from '../renderer_native/FrameworkProxy';
import { NativeRenderer } from './connector/NativeRenderer';
import { VfsManager } from '../vfs/VfsManager';
import { NativeRenderContext } from '../renderer_native/NativeRenderContext';
import { NativeRenderProvider } from '../renderer_native/NativeRenderProvider';
import { HippyLibrary } from '../hippy_library/HippyLibrary';
import { HippyException } from '../support/common/HippyException';
import { HippyAPIProvider } from './bridge/HippyAPIProvider';
import { HRRenderViewCreator } from '../renderer_native/components/HippyRenderRegisterMap';
import { HippyGlobalConfigs } from './HippyGlobalConfigs';
import { MonitorGroupType, TimeMonitor } from '../support/utils/TimeMonitor';
import { TextUtils } from '../support/utils/TextUtils';
import { HippyAssetBundleLoader } from './bridge/bundleloader/HippyAssetBundleLoader';
import { HippyBundleLoader } from './bridge/bundleloader/HippyBundleLoader';
import { HippyFileBundleLoader } from './bridge/bundleloader/HippyFileBundleLoader';
import { HippyThirdPartyAdapter } from './adapter/thirdparty/HippyThirdPartyAdapter';
import { HippyEngineContext } from './HippyEngineContext'
import { HippyEngineContextImpl } from './HippyEngineContextImpl';
import { LogUtils } from '../support/utils/LogUtils';
import { HippyAny, HippyMap } from '../support/common/HippyTypes';
import { DeviceEventModule } from './modules/native';
import { HippyRemoteBundleLoader } from './bridge/bundleloader/HippyRemoteBundleLoader';
import { EventDispatcher } from './modules/javascript/EventDispatcher';
import { HippyRootView } from '../renderer_native';
import { DevtoolsManager } from './devsupport/DevtoolsManager';
import { DevSupportManager } from './devsupport/DevSupportManager';
import { DevServerCallBack } from './devsupport/DevServerCallBack';
import { HippyJsException } from './common/HippyJsException';

export class HippyEngineManagerImpl implements HippyEngine, FrameworkProxy, DevServerCallBack {
  private libHippy: HippyLibrary
  private resourceManager: resmgr.ResourceManager | null = null
  private providers: Array<HippyAPIProvider> | null = null
  private params: EngineInitParams;
  public domMgr: DomManager | null = null
  public nativeRenderProvider: NativeRenderProvider | null = null
  public nativeRenderer: NativeRenderer | null = null
  public mGlobalConfigs: HippyGlobalConfigs
  public mCoreBundleLoader: HippyBundleLoader | null = null;
  public mDebugMode: boolean;
  public mServerBundleName: string;
  public enableV8Serialization: boolean;
  public mRemoteServerUrl: string;
  public mGroupId: number;
  public mMonitor: TimeMonitor;
  public mServerHost: string;
  public mThirdPartyAdapter: HippyThirdPartyAdapter | null;
  public v8InitParams: V8InitParams | null;
  public nativeParams: HippyMap | null = null;
  public mEngineContext: HippyEngineContextImpl | null = null;
  private mDevtoolsManager: DevtoolsManager | null = null;
  private mDevSupportManager: DevSupportManager | null = null;

  private destroyCallback: Function | null = null;
  private rootView: HippyRootView | null = null;

  constructor(params: EngineInitParams) {
    this.params = params;
    this.libHippy = params.libHippy
    this.providers = params.providers
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
    // this.mProcessors = params.processors;
  }

  onInitDevError(e: HippyJsException): void {
     // TODO
  }

  onDebugReLoad(): void {
    if (this.rootView) {
      this.destroyModule(this.rootView.getRootId(), ()=> {
        this.mEngineContext?.destroyBridge(true);
      });
    }
  }

  onBackPressed(handler: BackPressHandler): boolean {
    if (this.mEngineContext != null
      && this.mEngineContext.getModuleManager().getNativeModule(DeviceEventModule.NAME)
        != null) {
      let nativeModule = this.mEngineContext.getModuleManager().getNativeModule(DeviceEventModule.NAME)
      if (nativeModule instanceof DeviceEventModule) {
        return nativeModule.onBackPressed(handler);
      } else {
        return false;
      }
    } else {
      return false;
    }
  }

  getHippyEngineContext(): HippyEngineContext | null {
    return this.mEngineContext
  }

  public initDevModule() {
    if (this.params.debugMode) {
      this.mDevSupportManager = new DevSupportManager(
        this.mGlobalConfigs,
        this.params.debugMode,
        this.params.debugServerHost,
        this.params.debugBundleName,
        this.params.remoteServerUrl);
      let url = this.mDevSupportManager.createResourceUrl(this.mServerBundleName);
      this.mCoreBundleLoader = new HippyRemoteBundleLoader(url as string, false, "");
      // 注入调试回调
      this.mDevSupportManager.setDevCallback(this);
    }
  }

  public restartEngine(initCallback: HippyEngineInitCallback) {
    // 性能打点
    this.mMonitor.startPoint(MonitorGroupType.ENGINE_INITIALIZE, TimeMonitor.MONITOR_POINT_INIT_NATIVE_ENGINE);
    this.domMgr = new DomManager(this.libHippy)
    // 初始化 HippyEngineContext
    this.mEngineContext = new HippyEngineContextImpl(
      this.params,
      this.domMgr,
      this.mGlobalConfigs,
      this.mMonitor,
      this.mCoreBundleLoader,
      this.mDevSupportManager,
    );
    // 提供给 Render 模块的 Proxy
    this.nativeRenderProvider = this.mEngineContext.getNativeRenderProvider()
    if (this.nativeRenderProvider) {
      this.nativeRenderProvider.getNativeRenderImpl().setFrameworkProxy(this);
    }

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

    // initBridge
    this.mEngineContext.getBridgeManager().initBridge((result: number, reason: string) => {
      this.initBridgeCallback(result, reason, initCallback)
    });
  }

  initBridgeCallback = (result: number, reason: string, initCallback: HippyEngineInitCallback) => {
    const jsDriver = this.mEngineContext?.getJsDriver();
    if (!jsDriver) {
      return
    }
    LogUtils.i('hippy', 'jsDriver runScriptFromUri callback, instanceId: %{public}d' + jsDriver.instanceId);
    if (result != 0) {
      return;
    }

    if (initCallback) {
      initCallback(EngineInitStatus.STATUS_OK, "")
    }

  }

  public sendEvent(event: string, params: HippyAny): void {
    if (this.mEngineContext != null && this.mEngineContext.getModuleManager() != null) {
      const eventModule = this.mEngineContext.getModuleManager().getJavaScriptModule(EventDispatcher.MODULE_NAME);
      if (eventModule != null) {
        (eventModule as EventDispatcher).receiveNativeEvent(event, params);
      }
    }
  }

  public initEngine(initCallback: HippyEngineInitCallback): void {
    this.initDevModule();
    this.restartEngine(initCallback);
  }

  loadModule(loadParams: ModuleLoadParams, loadCallback: HippyModuleLoadCallback): HippyRootView | null {
    const jsDriver = this.mEngineContext?.getJsDriver();
    if (!jsDriver || !this.getVfsManager() || !this.resourceManager) {
      return null;
    }
    if (!this.mDebugMode) {
      let coreBundleLoader: HippyBundleLoader | null = null;
      if (!TextUtils.isEmpty(loadParams.jsAssetsPath)) {
        coreBundleLoader = new HippyAssetBundleLoader(this.mGlobalConfigs.getContext()!, loadParams.jsAssetsPath,
          !TextUtils.isEmpty(loadParams.codeCacheTag), loadParams.codeCacheTag);
      } else if (!TextUtils.isEmpty(loadParams.jsFilePath)) {
        coreBundleLoader = new HippyFileBundleLoader(loadParams.jsFilePath,
          !TextUtils.isEmpty(loadParams.codeCacheTag), loadParams.codeCacheTag);
      }
      this.mCoreBundleLoader = coreBundleLoader;
    }
    const rootView = this.mEngineContext!.createRootView(loadParams);
    this.rootView = rootView;
    this.loadJsModule(loadParams, rootView.getRootId(), loadCallback)
    if (this.params.debugMode && this.mDevSupportManager) {
      this.mDevSupportManager.attachToHost(this.params.context, rootView.getRootId());
    }
    return rootView;
  }

  destroyModule(rootId: number, destroyCallback: HippyModuleDestroyCallback): void {
    this.destroyCallback = destroyCallback;
    this.nativeRenderProvider!.getNativeRenderImpl().destroyRoot(rootId)
    if (this.mEngineContext != null && this.mEngineContext.getBridgeManager() != null) {
      this.mEngineContext.getBridgeManager().destroyInstance(rootId);
    }
    if (this.destroyCallback) {
      this.destroyCallback();
    }
  }

  destroyEngine(): void {
    if (this.mEngineContext != null) {
      this.mEngineContext.destroyBridge(false);
    }
  }

  onDestroyEngine(): void {
    if (this.mEngineContext != null) {
      this.mEngineContext.destroy(false);
    }
  }

  destroyRoot(rootId: number): void {
    this.mEngineContext!.destroyRoot(rootId)
  }

  getVfsManager(): VfsManager {
    return this.mEngineContext!.getVfsManager();
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
    this.mGlobalConfigs.getExceptionHandler()?.handleNativeException(exception, true);
  }

  updateDimension(width: number, height: number, shouldUseScreenDisplay: boolean, systemUiVisibilityChanged: boolean): void {
  }

  onSizeChanged(rootId: number, w: number, h: number, ow: number, oh: number): void {
  }

  loadJsModule(loadParams: ModuleLoadParams, rootId: number, loadCallback: HippyModuleLoadCallback) {
    if (!this.mEngineContext) {
      return;
    }
    if (!this.params.debugMode) {
      if (this.mCoreBundleLoader != null) {
        // TODO 多RootView
        this.mEngineContext?.getBridgeManager().runBundle(0, this.mCoreBundleLoader);
      } else {
        LogUtils.e('hippy', 'load module error. jsBundleLoader==null');
        // notifyModuleLoaded(ModuleLoadStatus.STATUS_VARIABLE_NULL,
        //   "load module error. jsBundleLoader==null");
        return;
      }
    }
    // TODO 多RootView
    this.mEngineContext.getBridgeManager().loadInstance(loadParams.componentName, rootId, loadParams.jsParams);

    // TODO: Debug
    if (this.params.debugMode) {
      // notifyModuleLoaded(ModuleLoadStatus.STATUS_OK, null);
    }
  }

  public getNativeRenderProvider(): NativeRenderProvider | null {
    return this.nativeRenderProvider
  }
}
