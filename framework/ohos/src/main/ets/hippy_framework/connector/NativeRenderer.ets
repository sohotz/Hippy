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
import { HippyLibrary } from '../../hippy_library/HippyLibrary'
import { NativeRenderProvider } from '../../renderer_native/NativeRenderProvider'
import { HippyAny } from '../../support/common/HippyTypes'
import { PixelUtil } from '../../support/utils/PixelUtil'

export class NativeRenderer {
  private libHippy: HippyLibrary
  private renderProvider: NativeRenderProvider
  public instanceId: number = 0

  constructor(libHippy: HippyLibrary, enableArkCApi: boolean, renderProvider: NativeRenderProvider,
              customRenderViews: Array<string> | null, customMeasureRenderViews: Array<string> | null,
              mappingRenderViews: Array<string> | null) {
    this.libHippy = libHippy
    this.renderProvider = renderProvider
    this.instanceId = this.createNativeRenderManager(enableArkCApi, renderProvider, customRenderViews, customMeasureRenderViews, mappingRenderViews)
  }

  createNativeRenderManager(enableArkCApi: boolean, renderProvider: NativeRenderProvider,
                            customRenderViews: Array<string> | null, customMeasureRenderViews: Array<string> | null,
                            mappingRenderViews: Array<string> | null): number {
    return this.libHippy?.NativeRenderer_CreateNativeRenderManager(enableArkCApi, renderProvider,
      customRenderViews, customMeasureRenderViews, mappingRenderViews, PixelUtil.getDensity())
  }

  destroyNativeRenderManager() {
    this.libHippy?.NativeRenderer_DestroyNativeRenderManager(this.instanceId)
  }

  attachToDom(
    domManagerId: number
  ) {
    this.libHippy?.NativeRenderer_SetDomManager(this.instanceId, domManagerId)
  }
}
