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
import { DefaultStorageAdapter } from '../../../adapter/storage/DefaultStorageAdapter';
import { HippyStorageAdapter } from '../../../adapter/storage/HippyStorageAdapter';

export class StorageModule extends HippyNativeModuleBase {
  public static readonly NAME = 'StorageModule';
  // private adapter: HippyStorageAdapter;

  constructor(ctx: HippyEngineContext) {
    super(ctx)
    // todo Need Harmony Context
    // this.adapter = new DefaultStorageAdapter(context);
  }
  /*
  multiGet(keys: string[]): Promise<[key: string, value: string][]> {
    return this.adapter.multiGet(keys);
  }

  multiSet(pairs: [key: string, value: string][]): Promise<void> {
    return this.adapter.multiSet(pairs);
  }

  multiRemove(keys: string[]): Promise<void> {
    return this.adapter.multiRemove(keys);
  }

  getAllKeys(): Promise<string[]> {
    return this.adapter.getAllKeys();
  }
  */
}
