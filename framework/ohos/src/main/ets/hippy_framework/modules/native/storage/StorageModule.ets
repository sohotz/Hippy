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
import { HippyAny, HippyArray } from '../../../../support/common/HippyTypes';
import { HippyModulePromise } from '../../HippyModulePromise';

const STORAGE_MODULE_TAG = "StorageModule"

@HippyNativeModule({ name: "StorageModule" })
export class StorageModule extends HippyNativeModuleBase {
  public static readonly NAME = 'StorageModule';
  private mStorageAdapter: HippyStorageAdapter | null;

  constructor(ctx: HippyEngineContext) {
    super(ctx)
    this.mStorageAdapter = this.ctx.getGlobalConfigs().getStorageAdapter();
  }

  public call(method: string, params: Array<HippyAny>, callback: HippyModulePromise): HippyAny {
    switch (method) {
      case 'multiGet': {
        this.multiGet(params[0] as string[], callback)
        break;
      }
      case 'multiSet': {
        this.multiSet(params[0] as HippyArray, callback)
        break;
      }
      case 'multiRemove': {
        this.multiRemove(params[0] as string[], callback)
        break;
      }
      case 'getAllKeys': {
        this.getAllKeys(callback)
        break;
      }
      default:
        super.call(method, params, callback)
    }
    return null;
  }

  private multiGet(keys: string[], promise: HippyModulePromise) {
    let map = new Map<string, string>()
    if (keys == null || keys.length <= 0) {
      map.set("multiGet", "Invalid Key")
      promise.resolve(map);
    }
    if (this.mStorageAdapter == null) {
      map.set("multiGet", "Database Null")
      promise.resolve(map);
    }
    this.mStorageAdapter?.multiGet(keys).then(result => {
      if (result == null || result.length <= 0) {
        map.set("multiGet", "result Null")
        promise.resolve(map);
      }
      let data = new Array<[key: string, value: string]>();
      result.forEach((value, index, result) => {
        data.push([value[0], value[1]])
      })
      LogUtils.i(STORAGE_MODULE_TAG, 'multiGet:' + data);
      map.set("multiGet", JSON.stringify(data))
      promise.resolve(map);
    })
  }

  private multiSet(params: HippyArray, promise: HippyModulePromise) {
    let map = new Map<string, string>()
    if (params == null || params.length <= 0) {
      map.set("multiSet", "Invalid Key")
      promise.resolve(map);
    }
    if (this.mStorageAdapter == null) {
      map.set("multiSet", "Database Null")
      promise.resolve(map);
    }
    try {
      let data = new Array<[key: string, value: string]>();
      for (let i = 0; i < params.length; i++) {
        if (params[i] instanceof Array) {
          data.push([(params[i] as HippyArray)[0] as string, (params[i] as HippyArray)[1] as string]);
        }
      }
      LogUtils.i(STORAGE_MODULE_TAG, params[0] + '、' + data);

      this.mStorageAdapter?.multiSet(data).then((err) => {
        if (err == undefined) {
          LogUtils.i(STORAGE_MODULE_TAG, "multiSet succ");
        } else {
          map.set("multiSet", JSON.stringify(err))
          promise.resolve(map);
        }
      })
    } catch (e) {
      LogUtils.i(STORAGE_MODULE_TAG, "multiSet fail:" + e);
      map.set("multiSet", e.getMessage())
      promise.resolve(map);
    }
  }

  private multiRemove(keys: string[], promise: HippyModulePromise) {
    let map = new Map<string, string>()
    if (keys == null || keys.length <= 0) {
      map.set("multiRemove", "Invalid Key")
      promise.resolve(map);
    }
    if (this.mStorageAdapter == null) {
      map.set("multiRemove", "Database Null")
      promise.resolve(map);
    }
    this.mStorageAdapter?.multiRemove(keys).then((err) => {
      if (err == undefined) {
        LogUtils.i(STORAGE_MODULE_TAG, "multiRemove succ");
      } else {
        LogUtils.i(STORAGE_MODULE_TAG, "multiRemove fail:" + err);
        map.set("multiRemove", JSON.stringify(err))
        promise.resolve(map);
      }
    })
  }

  private getAllKeys(promise: HippyModulePromise) {
    let map = new Map<string, string>()
    if (this.mStorageAdapter == null) {
      map.set("getAllKeys", "Database Null")
      promise.resolve(map);
    }
    this.mStorageAdapter?.getAllKeys().then((result) => {
      if (result == null || result.length <= 0) {
        map.set("getAllKeys", "")
        promise.resolve(map);
      } else {
        map.set("getAllKeys", JSON.stringify(result))
        promise.resolve(map);
      }
    })
  }
}
