import { HippyLibrary } from '../hippy_library/HippyLibrary';
import { HRRenderViewCreator } from '../renderer_native/components/HippyRenderRegisterMap';
import { NativeRenderProvider } from '../renderer_native/NativeRenderProvider';
import { PixelUtil } from '../support/utils/PixelUtil';
import { TimeMonitor } from '../support/utils/TimeMonitor';
import { VfsManager } from '../vfs/VfsManager';
import { HippyBundleLoader } from './bridge/bundleloader/HippyBundleLoader';
import { HippyBridgeManager } from './bridge/HippyBridgeManager';
import { HippyBridgeManagerImpl } from './bridge/HippyBridgeManagerImpl';
import { DomManager } from './connector/DomManager';
import { JsDriver } from './connector/JsDriver';
import { NativeRenderer } from './connector/NativeRenderer';
import { EngineInitParams, ModuleLoadParams } from './HippyEngine';
import { HippyEngineContext } from './HippyEngineContext';
import { HippyGlobalConfigs } from './HippyGlobalConfigs';
import { HippyModuleManagerImpl, HippyNativeModuleCreator } from './modules/HippyModuleManagerImpl';
import { HippyResourceLoader } from './HippyResourceLoader';
import { DefaultProcessor } from '../vfs/DefaultProcessor';
import { HippyModuleManager } from './modules/HippyModuleManager';
import { HippyAPIProvider } from './bridge/HippyAPIProvider';
import { HippyJsException } from './common/HippyJsException';
import { DevSupportManager } from './devsupport/DevSupportManager';
import { DevtoolsManager } from './devsupport/DevtoolsManager';
import { LogUtils } from '../support/utils/LogUtils';
import { HippyRootView } from '../renderer_native/HippyRootView';
import { NativeRender } from '../renderer_native/NativeRender';
import { HippyRootViewManager } from '../renderer_native/HippyRootViewManager';

export class HippyEngineContextImpl implements HippyEngineContext {
  public vfsManager: VfsManager
  public nativeParams: Map<string, Object> = new Map<string, Object>()
  public mCoreBundleLoader: HippyBundleLoader | null = null;
  public nativeRenderProvider: NativeRenderProvider | null = null
  public nativeRenderer: NativeRenderer | null = null
  private mEngineInitParams: EngineInitParams;
  private mGlobalConfigs: HippyGlobalConfigs;
  private mDomManager: DomManager;
  private mMonitor: TimeMonitor;
  private mModuleManager: HippyModuleManager;
  private mBridgeManager: HippyBridgeManager | null = null;
  private providers: Array<HippyAPIProvider> | null = null
  private libHippy: HippyLibrary
  private mJsDriver: JsDriver | null = null
  private mDevSupportManager: DevSupportManager | null = null;
  private mDevtoolsManager: DevtoolsManager | null = null;
  private mDebugMode: boolean;
  private loadModuleCallback: Function | null = null;
  private component: string = '';

  public constructor(
    params: EngineInitParams,
    domManager: DomManager,
    globalConfigs: HippyGlobalConfigs,
    monitor: TimeMonitor,
    coreBundleLoader: HippyBundleLoader | null,
    devSupportManager: DevSupportManager| null,
  ) {
    this.mEngineInitParams = params;
    this.libHippy = this.mEngineInitParams.libHippy
    this.mGlobalConfigs = globalConfigs;
    const resourceLoader = new HippyResourceLoader(this)
    this.vfsManager = new VfsManager(this.libHippy)
    this.vfsManager.addProcessorAtLast(new DefaultProcessor(resourceLoader))
    this.mDomManager = domManager;
    this.mMonitor = monitor;
    this.mCoreBundleLoader = coreBundleLoader;
    this.mDebugMode = params.debugMode;


    this.providers = this.mEngineInitParams.providers;
    // provider挂载
    let customRenderViewCreatorMap: Map<string, HRRenderViewCreator> | null = null;
    let customRenderViews: Array<string> | null = null;
    let customMeasureRenderViews: Array<string> | null = null;
    let mappingRenderViews: Array<string> | null = null;
    let customNativeModules: Map<string, HippyNativeModuleCreator> | null = null;
    let customFontPathMap: Map<string, string> | null = null;
    if (this.providers) {
      customRenderViewCreatorMap = new Map();
      customRenderViews = new Array();
      customMeasureRenderViews = new Array();
      mappingRenderViews = new Array();
      customNativeModules = new Map();
      customFontPathMap = new Map();
      for (let provider of this.providers) {
        let apiProviderRenderMap = provider.getCustomRenderViewCreatorMap()
        if (apiProviderRenderMap) {
          apiProviderRenderMap.forEach((value, key) => {
            customRenderViewCreatorMap?.set(key, value)
            customRenderViews?.push(key)
          })
        }
        let apiProviderMeasureArray = provider.getCustomMeasureRenderViews()
        if (apiProviderMeasureArray) {
          apiProviderMeasureArray.forEach((value) => {
            customMeasureRenderViews?.push(value)
          })
        }
        let apiProviderRenderMappingMap = provider.getCustomMappingRenderViewMap()
        if (apiProviderRenderMappingMap) {
          apiProviderRenderMappingMap.forEach((value, key) => {
            mappingRenderViews?.push(key)
            mappingRenderViews?.push(value)
          })
        }
        let apiProviderNativeModules = provider.getCustomNativeModuleCreatorMap();
        if (apiProviderNativeModules) {
          apiProviderNativeModules.forEach((value, key) => {
            customNativeModules?.set(key, value)
          })
        }
        let apiProviderFontPaths = provider.getCustomFontPathMap();
        if (apiProviderFontPaths) {
          apiProviderFontPaths.forEach((value, key) => {
            customFontPathMap?.set(key, value)
          })
        }
      }
    }

    let bundlePath = ""
    if (coreBundleLoader) {
      bundlePath = coreBundleLoader.getPath()
    }

    // Render注册
    let enableArkCApi = globalConfigs.enableArkCApi()
    this.nativeRenderProvider = new NativeRenderProvider(this.libHippy, enableArkCApi, customRenderViewCreatorMap, customMeasureRenderViews)
    this.nativeRenderer = new NativeRenderer(this.libHippy, enableArkCApi, this.nativeRenderProvider,
      customRenderViews, customMeasureRenderViews, mappingRenderViews, bundlePath)
    this.nativeRenderer.registerCustomFontPaths(customFontPathMap)
    this.nativeRenderer.attachToDom(this.mDomManager.instanceId)
    // DomManager挂载
    this.mDomManager.attachToRenderer(this.nativeRenderer.instanceId)
    // ModuleManager注册
    this.mModuleManager = new HippyModuleManagerImpl(this, params.enableV8Serialization);
    this.mModuleManager.addModules(customNativeModules);

    if (params.integrateJSEngine) {
      // JsDriver注册
      this.mJsDriver = new JsDriver(this.libHippy, this.mModuleManager)
      // BridgeManager注册
      let bridgeManagerImpl = new HippyBridgeManagerImpl(
        this,
        coreBundleLoader,
        params.enableV8Serialization,
        params.debugMode,
        params.debugServerHost,
        params.groupId,
        params.thirdPartyAdapter,
        params.v8InitParams,
        this.mJsDriver);
      this.mBridgeManager = bridgeManagerImpl;
      this.mJsDriver.setBridgeCallback(bridgeManagerImpl);
      this.mDevSupportManager = devSupportManager;

      // DevtoolsManager创建
      this.mDevtoolsManager = new DevtoolsManager(params.libHippy, params.debugMode);
      if (params.debugMode) {
        let localCachePath = this.getGlobalConfigs().getContext()?.cacheDir;
        let debugUrl = this.getDevSupportManager()?.createDebugUrl(params.debugServerHost);
        this.mDevtoolsManager?.create(localCachePath as string, debugUrl || '');
      }
    }
  }

  onBridgeDestroyed(isReload: boolean): void {
    // TODO devtools relaod
    // if (isReload) {
    //   this.restartEngineInBackground(true);
    // } else {
    //   this.destroy(false);
    // }
  }

  setLoadModuleCallback(loadModuleCallback: Function | null) {
    this.loadModuleCallback = loadModuleCallback;
  }

  onLoadModuleCompleted(statusCode: number, msg: string): void {
    if (this.loadModuleCallback) {
        this.loadModuleCallback(statusCode, msg);
    }
  }

  public setComponentName(component: string) {
    return this.component = component;
  }

  public getComponentName(): string {
    return this.component;
  }

  getLibHippy(): HippyLibrary {
    return this.mEngineInitParams.libHippy
  }

  getDevSupportManager(): DevSupportManager | null {
    return this.mDevSupportManager;
  }

  onRuntimeInitialized(): void {
    if (!this.mJsDriver) {
      return
    }
    this.mJsDriver.attachToDom(this.mDomManager.instanceId)
    if (this.mDevtoolsManager != null && this.nativeRenderer != null) {
      this.mDevtoolsManager.bind(this.mJsDriver, this.mDomManager, this.nativeRenderer);
    }
  }

  createRootView(loadParams: ModuleLoadParams) {
    const rootView = this.nativeRenderProvider!.getNativeRenderImpl().createRootView(loadParams.componentName, {}, loadParams.wrappedCustomRenderViewBuilder)
    if (rootView != null) {
      this.attachDomToRootView(rootView.getRootId())
      this.attachJsToRootView(rootView.getRootId())
      if (this.mDevtoolsManager != null) {
        this.mDevtoolsManager.attachToRoot(rootView.getRootId());
      }
    }
    return rootView;
  }

  attachDomToRootView(rootId: number): void {
    this.mDomManager.createRootNode(rootId, PixelUtil.getDensity())
    this.mDomManager.attachToRoot(rootId)
  }

  attachJsToRootView(rootId: number): void {
    this.mJsDriver?.attachToRoot(rootId)
  }

  destroyRoot(rootId: number): void {
    this.mDomManager.destroyRootNode(rootId)
  }

  destroyBridge(isReload: boolean): void {
    this.mBridgeManager?.destroyBridge(isReload);
  }

  destroy(isReload: boolean) {
    if (this.mDevtoolsManager != null) {
      this.mDevtoolsManager.destroy(isReload);
    }
    if (!isReload) {
      this.mDomManager.destroy();
    }
    this.mModuleManager.destroy();
    this.vfsManager.destroy();
    // this.onDestroyVfs(this.vfsManager.getVfsId());
}

  getDomManagerId(): number {
    return this.mDomManager.getDomManagerId();
  }

  getVfsId(): number {
    return this.vfsManager.getVfsId();
  }

  getNativeRenderProvider(): NativeRenderProvider | null {
    return this.nativeRenderProvider;
  }

  getVfsManager(): VfsManager {
    return this.vfsManager
  }

  getJsDriver(): JsDriver | null {
    return this.mJsDriver;
  }

  getMonitor(): TimeMonitor {
    return this.mMonitor;
  }

  getGlobalConfigs(): HippyGlobalConfigs {
    return this.mGlobalConfigs;
  }

  getModuleManager(): HippyModuleManager {
    return this.mModuleManager;
  }

  getBridgeManager(): HippyBridgeManager | null {
    return this.mBridgeManager;
  }

  getNativeParams(): Map<string, Object> {
    return this.nativeParams
  }

  getNativeRender(): NativeRender | null {
    return this.nativeRenderProvider ? this.nativeRenderProvider.getNativeRenderImpl() : null
  }

  getDevtoolsId(): number {
    return this.mDevtoolsManager != null ? this.mDevtoolsManager.getId() : -1;
  }

  handleException(error: HippyJsException): void {
    if (this.mDebugMode && this.mDevSupportManager != null) {
      this.mDevSupportManager.handleException(error);
      this.mGlobalConfigs.getExceptionHandler()?.handleJsException(error);
    } else {
      if (error instanceof HippyJsException) {
        this.mGlobalConfigs.getExceptionHandler()?.handleJsException(error);
      } else {
        // TODO Native Exception
        this.mGlobalConfigs.getExceptionHandler()?.handleNativeException(new HippyJsException('', ''), true);
      }
    }
  }

  getRootView(rootId: number): HippyRootView | null {
    let view = HippyRootViewManager.getRootView(rootId)
    return view ? view : null
  }
}
