import { TimeMonitor } from '../support/utils/TimeMonitor';
import { VfsManager } from '../vfs/VfsManager';
import { HippyBridgeManager } from './bridge/HippyBridgeManager';
import { DomManager } from './connector/DomManager';
import { JsDriver } from './connector/JsDriver';
import { HippyEngineContext } from './HippyEngineContext';
import { HippyGlobalConfigs } from './HippyGlobalConfigs';
import { HippyModuleManager } from './modules/HippyModuleManager';


export class HippyEngineContextImpl implements HippyEngineContext {
  private mGlobalConfigs: HippyGlobalConfigs;

  // TODO
  // private mDevtoolsManager: DevtoolsManager;


  public constructor(
    domManager: DomManager,
    globalConfigs: HippyGlobalConfigs) {
    this.mGlobalConfigs = globalConfigs;
  }

  getComponentName(): string {
    throw new Error('Method not implemented.');
  }

  getVfsManager(): VfsManager {
    throw new Error('Method not implemented.');
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
}
