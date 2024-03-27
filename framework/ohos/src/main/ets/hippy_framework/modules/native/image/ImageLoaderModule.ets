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
import { HippyAny, HippyMap,
  HippyModuleCallback,
  HippyValue } from '../../../../support/common/HippyTypes'

import { VfsManager } from '../../../../vfs/VfsManager'
import { HippyEngineContext } from '../../../HippyEngineContext'
import { HippyNativeModuleBase } from '../HippyNativeModuleBase'
import { HashMap } from '@kit.ArkTS'
import { TextUtils } from '../../../../support/utils/TextUtils'
import { ResourceDataHolder } from '../../../../vfs/ResourceDataHolder'
import { image } from '@kit.ImageKit'
import { HippyException } from '../../../../support/common/HippyException'
import { LogUtils } from '../../../../support/utils/LogUtils'
import { FetchResourceCallback } from '../../../../vfs/FetchResourceCallback'


const IMAGE_MODULE_TAG = "ImageLoaderModule"

@HippyNativeModule({ name: "ImageLoaderModule" })
export class ImageLoaderModule extends HippyNativeModuleBase {
  public static readonly NAME = 'ImageLoaderModule'
  private mVfsManager: VfsManager;
  private static ERROR_KEY_MESSAGE = "message";

  constructor(ctx: HippyEngineContext) {
    super(ctx)
    this.mVfsManager = ctx.getVfsManager();
  }

  private decodeImageData(url: string, data: ArrayBuffer): Promise<Map<string, string>> {
    return new Promise<Map<string, string>>((resolve, reject) => {
      if (data && data.byteLength > 0) {
        const imageSourceApi = image.createImageSource(data);
        imageSourceApi.getImageInfo(0)
          .then(imageInfo => {
            LogUtils.d(IMAGE_MODULE_TAG, 'Succeeded in obtaining the image information.');
            let map = new Map<string, string>()
            map.set("width", imageInfo.size.width + "")
            map.set("height", imageInfo.size.height + "")
            resolve(map)
          }).catch((e: HippyException) => {
          LogUtils.e(IMAGE_MODULE_TAG, 'Failed to obtain the image information.');
          let map = new Map<string, string>()
          map.set("width", "Fetch image failed, url=" + url + ", msg=" + e.message)
          reject(map)
        })
      }
    })
  }

  private generateRequestParams(): HashMap<string, string> {
    let requestParams = new HashMap<string, string>();
    requestParams.set("Content-Type", "image");
    return requestParams;
  }

  private getSize(url: string): Promise<Map<string, string>> {
    return new Promise<Map<string, string>>((resolve, reject) => {
      if (TextUtils.isEmpty(url)) {
        let map = new Map<string, string>()
        map.set("width", "Url parameter is empty!")
        reject(map);
        return;
      }
      this.mVfsManager.fetchResourceAsync(url, null, this.generateRequestParams(), new ResourceCallback(this.ctx, url))
    })
  }

  private prefetch(url: string) {
    this.mVfsManager.fetchResourceAsync(url, null, this.generateRequestParams(), new ResourceCallback(this.ctx, "prefetch"))
  }

  public fetchCompleted(dataHolder: ResourceDataHolder, url: string) {
    return new Promise<Map<string, string>>((resolve, reject) => {
      let bytes = dataHolder.getBytes();
      if (dataHolder.resultCode
        != ResourceDataHolder.RESOURCE_LOAD_SUCCESS_CODE || bytes == null
        || bytes == null) {
        let message =
          dataHolder.errorMessage != null ? dataHolder.errorMessage : "";
        let errorMsg = "Fetch image failed, url=" + url + ", msg=" + message;
        let map = new Map<string, string>()
        map.set(ImageLoaderModule.ERROR_KEY_MESSAGE, errorMsg)
        reject(map);
      } else {
        this.decodeImageData(url, bytes);
      }
      dataHolder.recycle();
    })
  }

  public call(method: string, params: Array<HippyAny>, callback: HippyModuleCallback): HippyAny {
    switch (method) {
      case 'getSize': {
        this.getSize(params[0] as string)
        break;
      }
      case 'prefetch': {
        this.prefetch(params[0] as string)
        break;
      }
      default:
        super.call(method, params, callback)
    }
    return null;
  }
}

class ResourceCallback implements FetchResourceCallback {
  private imageModule: ImageLoaderModule
  private url: string

  constructor(ctx: HippyEngineContext, str: string) {
    this.imageModule = new ImageLoaderModule(ctx)
    this.url = str
  }

  onFetchCompleted(dataHolder: ResourceDataHolder): void {
    if ("prefetch" === this.url) {
      dataHolder.recycle()
    } else {
      this.imageModule.fetchCompleted(dataHolder, this.url)
    }
  }

  onFetchProgress(total: number, loaded: number): void {
    // Nothing need to do here.
  }
}
