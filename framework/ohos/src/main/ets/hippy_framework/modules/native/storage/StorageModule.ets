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
import { HippyEngineContext } from '../../../HippyEngineContext';
import { HippyNativeModuleBase } from '../HippyNativeModuleBase';
import { HippyStorageAdapter } from '../../../adapter/storage/HippyStorageAdapter';
import { HippyNativeModule } from '../../../../support/annotation/HippyNativeModule';
import { LogUtils } from '../../../../support/utils/LogUtils';
import { HippyAny, HippyArray, HippyModuleCallback } from '../../../../support/common/HippyTypes';

const STORAGE_MODULE_TAG = "StorageModule"

@HippyNativeModule({ name: "StorageModule" })
export class StorageModule extends HippyNativeModuleBase {
  public static readonly NAME = 'StorageModule';
  private mStorageAdapter: HippyStorageAdapter | null;

  constructor(ctx: HippyEngineContext) {
    super(ctx)
    this.mStorageAdapter = this.ctx.getGlobalConfigs().getStorageAdapter();
  }

  public call(method: string, params: Array<HippyAny>, callback: HippyModuleCallback): HippyAny {
    switch (method) {
      case 'multiGet': {
        this.multiGet(params[0] as string[])
        break;
      }
      case 'multiSet': {
        this.multiSet(params[0] as HippyArray)
        break;
      }
      case 'multiRemove': {
        this.multiRemove(params[0] as string[])
        break;
      }
      case 'getAllKeys': {
        this.getAllKeys()
        break;
      }
      default:
        super.call(method, params, callback)
    }
    return null;
  }

  private multiGet(keys: string[]): Promise<[key: string, value: string][]> {
    return new Promise<[key: string, value: string][]>((resolve, reject) => {
      if (keys == null || keys.length <= 0) {
        reject("Invalid Key");
        return;
      }
      if (this.mStorageAdapter == null) {
        reject("Database Null");
        return;
      }
      return this.mStorageAdapter.multiGet(keys).then(result => {
        if (result == null || result.length <= 0) {
          reject("result Null");
          return;
        }
        let data = new Array<[key: string, value: string]>();
        result.forEach((value, index, result) => {
          data.push([value[0], value[1]])
        })
        LogUtils.i(STORAGE_MODULE_TAG, 'multiGet:' + data);
        resolve(data);
      })
    })
  }

  private multiSet(params: HippyArray): Promise<void> {
    return new Promise<void>((resolve, reject) => {
      if (params == null || params.length <= 0) {
        reject("Invalid Key");
        return;
      }
      if (this.mStorageAdapter == null) {
        reject("Database Null");
        return;
      }
      try {
        let data = new Array<[key: string, value: string]>();
        for (let i = 0; i < params.length; i++) {
          if (params[i] instanceof Array) {
            data.push([(params[i] as HippyArray)[0] as string, (params[i] as HippyArray)[1] as string]);
          }
        }
        LogUtils.i(STORAGE_MODULE_TAG, params[0] + '、' + data);

        this.mStorageAdapter.multiSet(data).then((err) => {
          if (err == undefined) {
            LogUtils.i(STORAGE_MODULE_TAG, "multiSet succ");
          } else {
            reject(err)
          }
        })
      } catch (e) {
        LogUtils.i(STORAGE_MODULE_TAG, "multiSet fail:" + e);
        reject(e.getMessage());
      }
    })
  }

  private multiRemove(keys: string[]): Promise<void> {
    return new Promise<void>((resolve, reject) => {
      if (keys == null || keys.length <= 0) {
        reject("Invalid Key");
        return;
      }
      if (this.mStorageAdapter == null) {
        reject("Database Null");
        return;
      }
      return this.mStorageAdapter.multiRemove(keys).then((err) => {
        if (err == undefined) {
          LogUtils.i(STORAGE_MODULE_TAG, "multiRemove succ");
        } else {
          LogUtils.i(STORAGE_MODULE_TAG, "multiRemove fail:" + err);
          reject(err)
        }
      })
    })
  }

  private getAllKeys(): Promise<string[]> {
    return new Promise<string[]>((resolve, reject) => {
      if (this.mStorageAdapter == null) {
        reject("Database Null");
        return;
      }
      return this.mStorageAdapter.getAllKeys().then((result) => {
        if (result == null || result.length <= 0) {
          reject([]);
          return;
        } else {
          resolve(result)
        }
      })
    })
  }
}
