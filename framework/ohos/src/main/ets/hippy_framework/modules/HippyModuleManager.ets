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
import { HippySerValue } from '../../serialization/SharedSerialization'
import { HippyException } from '../../support/common/HippyException'
import { HippyAny } from '../../support/common/HippyTypes'
import { ArgumentUtils } from '../../support/utils/ArgumentUtils'
import { HippyEngineContext } from '../HippyEngineContext'
import {
  HippyNativeModuleBase,
  AudioPlayerModule,
  ConsoleModule,
  StorageModule,
  DevMenuModule,
  DeviceEventModule,
  ExceptionModule,
  ImageLoaderModule,
  NetInfoModule,
  NetworkModule,
  WebSocketModule,
  UtilsModule
} from "./nativemodules"
import util from '@ohos.util'

export type HippyModuleCreator = (ctx: HippyEngineContext) => HippyNativeModuleBase

export function hippyModuleCreatorRegisterMap(): Map<string, HippyModuleCreator> {
  let registerMap: Map<string, HippyModuleCreator> = new Map()
  registerMap.set(AudioPlayerModule.NAME, (ctx): HippyNativeModuleBase => new AudioPlayerModule(ctx))
  registerMap.set(ConsoleModule.NAME, (ctx): HippyNativeModuleBase => new ConsoleModule(ctx))
  registerMap.set(StorageModule.NAME, (ctx): HippyNativeModuleBase => new StorageModule(ctx))
  registerMap.set(DevMenuModule.NAME, (ctx): HippyNativeModuleBase => new DevMenuModule(ctx))
  registerMap.set(DeviceEventModule.NAME, (ctx): HippyNativeModuleBase => new DeviceEventModule(ctx))
  registerMap.set(ExceptionModule.NAME, (ctx): HippyNativeModuleBase => new ExceptionModule(ctx))
  registerMap.set(ImageLoaderModule.NAME, (ctx): HippyNativeModuleBase => new DevMenuModule(ctx))
  registerMap.set(NetInfoModule.NAME, (ctx): HippyNativeModuleBase => new NetInfoModule(ctx))
  registerMap.set(NetworkModule.NAME, (ctx): HippyNativeModuleBase => new NetworkModule(ctx))
  registerMap.set(WebSocketModule.NAME, (ctx): HippyNativeModuleBase => new WebSocketModule(ctx))
  registerMap.set(UtilsModule.NAME, (ctx): HippyNativeModuleBase => new UtilsModule(ctx))
  return registerMap
}

export class HippyModuleManager {
  private ctx: HippyEngineContext
  private moduleCreatorRegisterMap: Map<string, HippyModuleCreator>
  private cachedModuleMap: Map<string, HippyNativeModuleBase> = new Map()
  private mEnableV8Serialization: boolean;

  constructor(ctx: HippyEngineContext, enableV8Serialization = false) {
    this.ctx = ctx
    this.moduleCreatorRegisterMap = hippyModuleCreatorRegisterMap()
    this.mEnableV8Serialization = enableV8Serialization;
  }

  public getModule(name: string): HippyNativeModuleBase | null {
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

  public bytesToArgument(buffer: ArrayBuffer, useJSValueType: boolean): string {
    let result = '';
    if (buffer) {
      try {
        if (this.mEnableV8Serialization) {
          // result = this.parseV8SerializeData(buffer, useJSValueType);
        } else {
          result = this.parseJsonData(buffer);
        }
      } catch (e) {
        throw new HippyException("parse bytesToArgument error");
      }
    }
    return result;
  }

  // public parseV8SerializeData(buffer: ArrayBuffer, useJSValueType: boolean) {
  //
  // }

  public parseJsonData(buffer: ArrayBuffer): string {
    const bytes = new Uint8Array(buffer);
    let textDecoder = util.TextDecoder.create('utf-8')
    let str = textDecoder.decodeWithStream(bytes)
    return str;
  }

}
