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
import { EngineInitParams } from './HippyEngine';
import { HippyEngineContext } from './HippyEngineContext';
import { HippyGlobalConfigs } from './HippyGlobalConfigs';
import { HippyModuleManagerImpl } from './modules/HippyModuleManagerImpl';
import { HippyResourceLoader } from './adapter/http/HippyResourceLoader';
import { DefaultProcessor } from '../vfs/DefaultProcessor';
import { HippyModuleManager } from './modules/HippyModuleManager';
import { HippyAPIProvider } from './bridge/HippyAPIProvider';
import { HippyJsException } from './common/HippyJsException';
import { DevSupportManager } from './devsupport/DevSupportManager';

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
  private mBridgeManager: HippyBridgeManager;
  private providers: Array<HippyAPIProvider> | null = null
  private libHippy: HippyLibrary
  private mJsDriver: JsDriver
  // TODO
  private mDevSupportManager: DevSupportManager;

  // private mDevtoolsManager: DevtoolsManager;


  public constructor(
    params: EngineInitParams,
    domManager: DomManager,
    globalConfigs: HippyGlobalConfigs,
    monitor: TimeMonitor,
    coreBundleLoader: HippyBundleLoader | null
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


    this.providers = this.mEngineInitParams.providers;
    // provider挂载
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
    // Render注册
    this.nativeRenderProvider = new NativeRenderProvider(this.libHippy, customRenderViewCreatorMap)
    this.nativeRenderer = new NativeRenderer(this.libHippy, this.nativeRenderProvider)
    this.nativeRenderer.attachToDom(this.mDomManager.instanceId)
    // DomManager挂载
    this.mDomManager.attachToRenderer(this.nativeRenderer.instanceId)

    // ModuleManager注册
    this.mModuleManager = new HippyModuleManagerImpl(this);
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
    this.mBridgeManager = bridgeManagerImpl
    this.mJsDriver.setBridgeCallback(bridgeManagerImpl)
    this.mDevSupportManager = new DevSupportManager(globalConfigs, params.debugMode, params.debugServerHost,
      params.debugBundleName, params.remoteServerUrl);
    // this.mDevSupportManager.setDevCallback(this);
  }

  getDevSupportManager(): DevSupportManager {
    return this.mDevSupportManager;
  }

  onRuntimeInitialized(): void {
    this.mJsDriver.attachToDom(this.mDomManager.instanceId)
  }

  attachDomToRootView(rootId: number): void {
    this.mDomManager.createRootNode(rootId, PixelUtil.getDensity())
    this.mDomManager.attachToRoot(rootId)
  }

  attachJsToRootView(rootId: number): void {
    this.mJsDriver.attachToRoot(rootId)
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

  getComponentName(): string {
    throw new Error('Method not implemented.');
  }

  getVfsManager(): VfsManager {
    return this.vfsManager
  }

  getJsDriver(): JsDriver {
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

  getBridgeManager(): HippyBridgeManager {
    return this.mBridgeManager;
  }

  getNativeParams(): Map<string, Object> {
    return this.nativeParams
  }

  handleException(error: HippyJsException): void {

  }
}
