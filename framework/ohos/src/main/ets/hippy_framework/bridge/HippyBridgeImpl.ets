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
import { HippyBridge } from './HippyBridge';
import { JsDriver } from '../connector/JsDriver';
import { HippyAny } from '../../support/common/HippyTypes';
import { NativeCallback } from '../connector/NativeCallback';

export class HippyBridgeImpl implements HippyBridge {
  private URI_SCHEME_ASSETS: string = '';
  private URI_SCHEME_FILE: string = '';
  private jsDriver: JsDriver

  public constructor(jsDriver: JsDriver) {
    this.jsDriver = jsDriver;
  }
  initJSBridge(gobalConfig: string, callback: NativeCallback, groupId: number): void {
    throw new Error('Method not implemented.');
  }

  runScriptFromUri(uri: string, canUseCodeCache: boolean, codeCacheTag: string, callback: NativeCallback): boolean {
    throw new Error('Method not implemented.');
  }

  onDestroy(): void {
    throw new Error('Method not implemented.');
  }

  destroy(callback: NativeCallback, isReload: boolean): void {
    throw new Error('Method not implemented.');
  }

  getV8RuntimeId(): number {
    throw new Error('Method not implemented.');
  }

  callFunction(functionId: number, callback: NativeCallback, buffer: Uint8Array) {
  }

}
