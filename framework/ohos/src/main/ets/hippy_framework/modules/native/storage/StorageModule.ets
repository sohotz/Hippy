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

const STORAGE_MODULE_TAG = "StorageModule"

@HippyNativeModule({ name: "StorageModule" })
export class StorageModule extends HippyNativeModuleBase {
  public static readonly NAME = 'StorageModule';
  private mStorageAdapter: HippyStorageAdapter|null;

  constructor(ctx: HippyEngineContext) {
    super(ctx)
    this.mStorageAdapter = this.ctx.getGlobalConfigs().getStorageAdapter();
  }

  multiGet(keys: string[]): Promise<[key: string, value: string][]> {
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
        data.forEach((value, index, result) => {
          data.push([value[0], value[1]])
        })
        resolve(data);
      })
    })
  }

  multiSet(params: [key: string, value: string][]): Promise<void> {
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
        for (let idx: number = 0; idx < params.length; idx++) {
          let array = params[idx]
          if (array == null) {
            reject("Invalid Value");
            return;
          }
          if (array.length != 2) {
            reject("Invalid Value");
            return;
          }
          if (array[0] == null) {
            reject("Invalid key");
            return;
          }
          if (array[1] == null) {
            reject("Invalid Value");
            return;
          }
          data.push([array[0], array[1]]);
        }
        this.mStorageAdapter.multiSet(data).then((err) => {
          if (err != null) {
            LogUtils.i(STORAGE_MODULE_TAG, "multiSet succ");
          } else {
            reject(err)
          }
        })
      } catch (e) {
        reject(e.getMessage());
      }
    })
  }

  multiRemove(keys: string[]): Promise<void> {
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
        if (err != null) {
          LogUtils.i(STORAGE_MODULE_TAG, "multiRemove succ");
        } else {
          reject(err)
        }
      })
    })
  }

  getAllKeys(): Promise<string[]> {
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
