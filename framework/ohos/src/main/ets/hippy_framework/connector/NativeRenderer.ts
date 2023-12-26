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

import { NativeRenderProvider } from '../../renderer_native/NativeRenderProvider'
import { PixelUtil } from '../../support/utils/PixelUtil'

export class NativeRenderer {
  public instanceId: number = 0

  constructor(private libHippy: any, private renderProvider: NativeRenderProvider) {
    this.instanceId = this.createNativeRenderManager(renderProvider)
  }

  createNativeRenderManager(renderProvider: NativeRenderProvider): number {
    return this.libHippy?.NativeRenderer_CreateNativeRenderManager(renderProvider, PixelUtil.getDensity())
  }

  destroyNativeRenderManager() {
    this.libHippy?.NativeRenderer_DestroyNativeRenderManager(this.instanceId)
  }

  getNativeRendererInstance(): any {
    return this.libHippy?.NativeRenderer_GetNativeRendererInstance(this.instanceId)
  }

  attachToDom(
    domManagerId: number
  ) {
    this.libHippy?.NativeRenderer_SetDomManager(this.instanceId, domManagerId)
  }
}
