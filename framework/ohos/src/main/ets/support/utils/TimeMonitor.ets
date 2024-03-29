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
import ArrayList from '@ohos.util.ArrayList';
import systemDateTime from '@ohos.systemDateTime';
import HashMap from '@ohos.util.HashMap';
import { LogUtils } from './LogUtils';
import { HippyJsException } from '../../hippy_framework/common/HippyJsException';

export enum MonitorGroupType {
  ENGINE_INITIALIZE,
  RUN_JS_BUNDLE,
  LOAD_INSTANCE,
  VFS_RESOURCE_LOAD
}

const TIME_MONITOR_TAG = "TimeMonitor"

export class TimeMonitor {
  public static MONITOR_POINT_INIT_NATIVE_ENGINE: string = "initNativeEngine";
  public static MONITOR_POINT_INIT_JS_ENGINE: string = "initJSEngine";
  public static MONITOR_POINT_LOAD_COMMON_JS: string = "loadCommonJs";
  public static MONITOR_POINT_NOTIFY_ENGINE_INITIALIZED: string = "notifyEngineInitialized";
  public static MONITOR_POINT_LOAD_BUSINESS_JS: string = "loadBusinessJs";
  public static MONITOR_POINT_LOAD_INSTANCE: string = "loadInstance";
  public static MONITOR_POINT_FIRST_FRAME: string = "firstFrame";
  mMonitorGroups: HashMap<MonitorGroupType, MonitorGroup> | null = null;

  public startPoint(groupType: MonitorGroupType, point: string): void {
    if (this.mMonitorGroups == null) {
      this.mMonitorGroups = new HashMap();
    }
    let monitorGroup: MonitorGroup = this.mMonitorGroups.get(groupType);
    if (monitorGroup == null) {
      monitorGroup = new MonitorGroup(groupType);
      this.mMonitorGroups.set(groupType, monitorGroup);
    }
    monitorGroup.startPoint(point);
  }

  public endGroup(groupType: MonitorGroupType): MonitorGroup | null {
    if (this.mMonitorGroups == null) {
      return null;
    }
    let monitorGroup: MonitorGroup = this.mMonitorGroups.get(groupType);
    if (monitorGroup != null) {
      monitorGroup.end();
    }
    return monitorGroup;
  }

  public getMonitorGroup(groupType: MonitorGroupType): MonitorGroup | null {
    if (this.mMonitorGroups == null) {
      return null;
    }
    return (this.mMonitorGroups == null) ? null : this.mMonitorGroups.get(groupType);
  }
}

export class MonitorGroup {
  public type: MonitorGroupType
  public beginTime: number = -1
  public totalTime: number = -1
  public isActive: boolean;
  private mMonitorPoints: ArrayList<MonitorPoint> | null = null;
  private mLastPoint: MonitorPoint | null = null;

  constructor(type: MonitorGroupType) {
    this.type = type;
    this.isActive = true;
  }

  startPoint(pointKey: string): void {
    if (!this.isActive) {
      return;
    }
    if (this.mMonitorPoints == null) {
      this.mMonitorPoints = new ArrayList();
    }
    let monitorPoint: MonitorPoint | null = this.checkMonitorPoint(pointKey);
    if (monitorPoint == null) {
      monitorPoint = new MonitorPoint(pointKey);
      this.mMonitorPoints.add(monitorPoint);
    }
    let currentTime: number = this.getCurrentTime()
    monitorPoint.startTime = currentTime;
    if (this.mLastPoint != null) {
      this.mLastPoint.endTime = currentTime;
    }
    this.mLastPoint = monitorPoint;
    if (this.beginTime == -1) {
      this.beginTime = currentTime;
    }
  }

  end(): void {
    if (this.isActive) {
      this.isActive = false;
      if (this.mLastPoint != null) {
        this.mLastPoint.endTime = this.getCurrentTime()
      }
      if (this.beginTime != -1) {
        this.totalTime = (Number)(this.getCurrentTime() - this.beginTime);
      }
    }
  }

  public getMonitorPoints(): ArrayList<MonitorPoint> | null {
    return this.mMonitorPoints;
  }

  getCurrentTime(): number {
    try {
      systemDateTime.getCurrentTime().then((time) => {
        LogUtils.d(TIME_MONITOR_TAG, `Succeeded in getting currentTime : ${time}`);
        return time
      }).catch((err: HippyJsException) => {
        LogUtils.e(TIME_MONITOR_TAG, `Failed to get currentTime. message:${err.message}`);
      });
    } catch (e) {
      LogUtils.e(TIME_MONITOR_TAG, `Failed to get currentTime. message:${e.message}, code:${e.code}`);
    }
    return -1
  }

  private checkMonitorPoint(pointKey: string): MonitorPoint | null {
    if (this.mMonitorPoints && this.mMonitorPoints.length > 0) {
      this.mMonitorPoints.forEach((monitorPoint) => {
        if (monitorPoint.key == pointKey) {
          return monitorPoint
        }
        return null;
      });
    }
    return null;
  }
}

export class MonitorPoint {
  public key: string
  public startTime: number = 0
  public endTime: number = 0

  constructor(key: string) {
    this.key = key;
  }
}
