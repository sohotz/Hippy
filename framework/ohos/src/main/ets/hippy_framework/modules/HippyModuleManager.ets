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
import { HippyEngineContext } from '../HippyEngineContext'
import {
  HippyNativeModuleBase,
  ConsoleModule, StorageModule,
  TimerModule } from "./nativemodules"

export type HippyModuleCreator = (ctx: HippyEngineContext) => HippyNativeModuleBase

export function hippyModuleCreatorRegisterMap(): Map<string, HippyModuleCreator> {
  let registerMap: Map<string, HippyModuleCreator> = new Map()
  registerMap.set(ConsoleModule.NAME, (ctx): HippyNativeModuleBase => new ConsoleModule(ctx))
  registerMap.set(StorageModule.NAME, (ctx): HippyNativeModuleBase => new StorageModule(ctx))
  registerMap.set(TimerModule.NAME, (ctx): HippyNativeModuleBase => new TimerModule(ctx))
  return registerMap
}

export class HippyModuleManager {
  private ctx: HippyEngineContext
  private moduleCreatorRegisterMap: Map<string, HippyModuleCreator>
  private cachedModuleMap: Map<string, HippyNativeModuleBase> = new Map()

  constructor(ctx: HippyEngineContext) {
    this.ctx = ctx
    this.moduleCreatorRegisterMap = hippyModuleCreatorRegisterMap()
  }

  getModule(name: string): HippyNativeModuleBase | null {
    if (this.cachedModuleMap.has(name)) {
      return this.cachedModuleMap.get(name) as HippyNativeModuleBase
    } else {
      let creator = this.moduleCreatorRegisterMap.get(name)
      if (creator) {
        let module = creator(this.ctx)
        this.cachedModuleMap.set(name, module)
        return module
      } else {
        return null
      }
    }
  }

}
