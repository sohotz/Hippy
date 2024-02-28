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
import { HippyLibrary } from '../hippy_library/HippyLibrary';
import { NativeRenderContext } from '../renderer_native/NativeRenderContext';
import { HippyEngineImpl } from './HippyEngineImpl';

export interface HippyEngine {
  initEngine(initCallbak: HippyEngineInitCallback)

  loadModule(loadParams: ModuleLoadParams, loadCallback: HippyModuleLoadCallback)

  getNativeRenderContext(): NativeRenderContext

}

export function createHippyEngine(params: EngineInitParams): HippyEngine {
  return new HippyEngineImpl(params)
}

export type HippyEngineInitCallback = (statusCode: EngineInitStatus, msg: string) => void
export type HippyModuleLoadCallback = (statusCode: ModuleLoadStatus, msg: string) => void

export class EngineInitParams {
  libHippy: HippyLibrary
  context: Context // UIAbilityContext

  constructor(
    libHippy: HippyLibrary,
    context: Context
  ) {
    this.libHippy = libHippy
    this.context = context
  }
}

export class ModuleLoadParams {

}

export enum EngineInitStatus {
  STATUS_OK = 0
}

export enum ModuleLoadStatus {
  STATUS_OK = 0
}

