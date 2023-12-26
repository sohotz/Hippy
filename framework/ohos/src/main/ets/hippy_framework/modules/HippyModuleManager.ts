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

import { HippyEngineContext } from '../HippyEngineContext';
import {
  HippyNativeModuleBase,
  ConsoleModule
} from "./nativemodules";

const MODULE_CLASS_TYPE_MAP: Record<string, typeof HippyNativeModuleBase> = {
  [ConsoleModule.NAME]: ConsoleModule,
}

export class HippyModuleManager {
  private cachedModuleMap: Record<string, HippyNativeModuleBase> = {};

  constructor(protected ctx: HippyEngineContext) {
  }

  getModule<T extends HippyNativeModuleBase>(name: string): T {
    if (!(name in this.cachedModuleMap)) {
      if (name in MODULE_CLASS_TYPE_MAP) {
        this.cachedModuleMap[name] = new MODULE_CLASS_TYPE_MAP[name](this.ctx);
        if (this.cachedModuleMap[name] === null) {
          throw new Error(`Couldn't create "${name}" Module`);
        }
      }
    }
    return this.cachedModuleMap[name] as T;
  }
}
