import { HippyMap } from '../support/common/HippyTypes';
import { TimeMonitor } from '../support/utils/TimeMonitor';
import { VfsManager } from '../vfs/VfsManager';
import { HippyBridgeManager } from './bridge/HippyBridgeManager';
import { DomManager } from './connector/DomManager';
import { JsDriver } from './connector/JsDriver';
import { HippyEngineContext } from './HippyEngineContext';
import { HippyGlobalConfigs } from './HippyGlobalConfigs';
import { HippyModuleManager } from './modules/HippyModuleManager';
import { HashMap } from '@kit.ArkTS';
import { HippyLibrary } from '../hippy_library/HippyLibrary';
import { HippyResourceLoader } from './adapter/http/HippyResourceLoader';
import { DefaultProcessor } from '../vfs/DefaultProcessor';

export class HippyEngineContextImpl implements HippyEngineContext {
  private mGlobalConfigs: HippyGlobalConfigs;
  public vfsManager: VfsManager
  public nativeParams: HashMap<string, Object> = new HashMap()

  // TODO
  // private mDevtoolsManager: DevtoolsManager;


  public constructor(
    hippyLibrary: HippyLibrary,
    domManager: DomManager,
    globalConfigs: HippyGlobalConfigs) {
    this.mGlobalConfigs = globalConfigs;
    this.vfsManager = new VfsManager(hippyLibrary)
    const resourceLoader = new HippyResourceLoader(this)
    this.vfsManager.addProcessorAtFirst(new DefaultProcessor(resourceLoader))
  }

  getComponentName(): string {
    throw new Error('Method not implemented.');
  }

  getVfsManager(): VfsManager {
    return this.vfsManager
  }

  getJsDriver(): JsDriver {
    throw new Error('Method not implemented.');
  }

  getMonitor(): TimeMonitor {
    throw new Error('Method not implemented.');
  }

  getGlobalConfigs(): HippyGlobalConfigs {
    return this.mGlobalConfigs;
  }

  getModuleManager(): HippyModuleManager {
    throw new Error('Method not implemented.');
  }

  getBridgeManager(): HippyBridgeManager {
    throw new Error('Method not implemented.');
  }

  getNativeParams() : HashMap<string, Object> {
    return this.nativeParams
  }
}
