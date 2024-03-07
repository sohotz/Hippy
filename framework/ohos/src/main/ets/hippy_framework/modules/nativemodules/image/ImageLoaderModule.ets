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
import { HippyNativeModule } from '../../../../support/annotation/HippyNativeModule'
import { HippyMap, HippyValue } from '../../../../support/common/HippyTypes'
import { VfsManager } from '../../../../vfs/VfsManager'
import { HippyEngineContext } from '../../../HippyEngineContext'
import { HippyNativeModuleBase } from '../HippyNativeModuleBase'
import { HashMap } from '@kit.ArkTS'

@HippyNativeModule({ name: "ImageLoaderModule" })
export class ImageLoaderModule extends HippyNativeModuleBase {
  public static readonly NAME = 'ImageLoaderModule'
  private mVfsManager: VfsManager;
  private static ERROR_KEY_MESSAGE = "message";

  constructor(ctx: HippyEngineContext) {
    super(ctx)
    this.mVfsManager = ctx.getVfsManager();
  }

  private decodeImageData(url: string, data: ArrayBuffer): Promise<void> {
    return new Promise<void>(() => {

    })
  }

  private generateRequestParams(): HashMap<string, string> {
    let requestParams = new HashMap<string, string>();
    requestParams.set("Content-Type", "image");
    return requestParams;
  }

  public getSize(url: string): Promise<void> {
    return new Promise<void>(() => {

    })
  }

  public prefetch(url: string) {

  }
}
