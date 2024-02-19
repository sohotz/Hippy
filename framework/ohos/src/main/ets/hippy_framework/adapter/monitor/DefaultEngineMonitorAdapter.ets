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
import { HippyEngineMonitorAdapter } from './HippyEngineMonitorAdapter';
import hilog from '@ohos.hilog';
import ArrayList from '@ohos.util.ArrayList';
import { MonitorGroup, MonitorPoint } from '../../../support/utils/TimeMonitor';
import { EngineInitStatus, ModuleLoadStatus } from '../..';
import { HippyCallNativeParams } from '../../bridge/HippyCallNativeParams';

const  DOMAIN = 0x0002
const TAG:string = "DefaultEngineMonitorAdapter"

export class DefaultEngineMonitorAdapter implements HippyEngineMonitorAdapter{

  protected printGroupTime( monitorGroup:MonitorGroup ):void {
    let monitorPoints:ArrayList<MonitorPoint> = monitorGroup.getMonitorPoints();
    if (monitorPoints != null) {
      monitorPoints.forEach((monitorPoint:MonitorPoint) =>{
        hilog.info(DOMAIN, TAG, monitorPoint.key + ": " + (monitorPoint.endTime - monitorPoint.startTime) + "ms");
      });
    }
    hilog.info(DOMAIN, TAG, "total time: " + monitorGroup.totalTime);
  }

  onEngineInitialized(statusCode: EngineInitStatus, monitorGroup: MonitorGroup): void {
    hilog.info(DOMAIN,TAG, "engine initialization completed with result: " + statusCode);
    this.printGroupTime(monitorGroup);
  }

  onLoadModuleCompleted(statusCode: ModuleLoadStatus, componentName: string, monitorGroup: MonitorGroup): void {
    hilog.info(DOMAIN,TAG,"load module completed with result: " + statusCode);
    this.printGroupTime(monitorGroup);
  }

  onLoadInstanceCompleted(componentName: string, monitorGroup: MonitorGroup): void {
    hilog.info(DOMAIN,TAG,
      componentName + " load instance completed with first view added");
    this.printGroupTime(monitorGroup);
  }

  onInterceptCallNative(componentName: string, params: HippyCallNativeParams): boolean {
    return false;
  }

  onCallNativeFinished(componentName: string, params: HippyCallNativeParams): void {
    throw new Error('Method not implemented.');
  }

  onInterceptPromiseCallback(componentName: string, moduleName: string, funcName: string, callbackId: string, callbackResult: Object): boolean {
    return false;
  }
}
