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
import {
  HippyAny,
  HippyMap,
  HippyModuleCallback,
  HippyValue
} from '../../../../support/common/HippyTypes'
import { VfsManager } from '../../../../vfs/VfsManager'
import { FetchResourceCallback, ResourceDataHolder } from '../../../../vfs/ResourceDataHolder'
import { HippyEngineContext } from '../../../HippyEngineContext'
import { HippyNativeModuleBase } from '../HippyNativeModuleBase'
import { HashMap, HashSet } from '@kit.ArkTS'
import { TextUtils } from '../../../../support/utils/TextUtils'
import { HippyHttpRequest } from '../../../adapter/http/HippyHttpRequest'
import { HippyHttpResponse } from '../../../adapter/http/HippyHttpResponse'
import { LogUtils } from '../../../../support/utils/LogUtils'
import util from '@ohos.util';

const NETWORK_MODULE_TAG = "NetworkModule"

@HippyNativeModule({ name: "NetworkModule" })
export class NetworkModule extends HippyNativeModuleBase {
  public static readonly NAME = 'NetworkModule'

  constructor(ctx: HippyEngineContext) {
    super(ctx)
  }

  public fetch(request: HippyMap): Promise<void> {
    return new Promise<void>((resolve, reject) => {
      let vfsManager: VfsManager = this.ctx.getVfsManager()
      let requestHeaders = new HashMap<string, string>();
      let requestParams = new HashMap<string, string>();
      try {
        this.normalizeRequest(request, requestHeaders, requestParams);
      } catch (e) {
        reject(e.getMessage());
        return;
      }
      let uri: string = requestParams.get(HippyHttpRequest.HTTP_URL);
      if (TextUtils.isEmpty(uri)) {
        reject("Get url parameter failed!");
        return;
      }

      vfsManager.fetchResourceAsync(uri, requestHeaders, requestParams,
        new FetchResCallback(this.ctx));
    });
  }

  public handleFetchResult(dataHolder: ResourceDataHolder): Promise<HippyAny> {
    return new Promise((resolve, reject) => {
      try {
        if (dataHolder.resultCode == ResourceDataHolder.RESOURCE_LOAD_SUCCESS_CODE) {
          let responseObject = this.handleFetchResponse(dataHolder);
          resolve(responseObject);
        } else {
          let errorMessage =
            (dataHolder.errorMessage == null) ? "Load remote resource failed!" : dataHolder.errorMessage;
          reject(errorMessage);
        }
      } catch (e) {
        reject("Handle response failed: " + e.getMessage());
      }
    })
  }

  handleFetchResponse(dataHolder: ResourceDataHolder): HippyAny {
    let resHeader = dataHolder.responseHeaders
    let headerMap: HippyAny = new Map();
    let responseMap: HippyAny = new Map();
    let statusCode = -1;
    let responseMessage = "";
    if (resHeader != null) {
      try {
        statusCode = Number.parseInt(
          resHeader.get(HippyHttpResponse.HTTP_RESPONSE_STATUS_CODE));
      } catch (e) {
        LogUtils.e(NETWORK_MODULE_TAG, "parse status code error!")
      }
      responseMessage = resHeader.get(HippyHttpResponse.HTTP_RESPONSE_RESPONSE_MESSAGE);
      for (let header of resHeader.entries()) {
        let key = header[0]
        let value = header[1]
        if (key == null || value == null) {
          continue;
        }
        headerMap.set(key, value);
      }
    }
    if (responseMessage == "") {
      responseMessage = (dataHolder.errorMessage == null) ? "" : dataHolder.errorMessage;
    }
    responseMap.set(HippyHttpResponse.HTTP_RESPONSE_STATUS_CODE, statusCode);
    responseMap.set("statusLine", responseMessage);
    responseMap.set("respHeaders", headerMap.toString());
    let body = "";
    // let result = util.TextDecoder.create('utf-8', dataHolder)
    // let retStr = result.encoding
    try {
      let bytes = dataHolder.getBytes();
      if (bytes != null) {
        let textDecoder = util.TextDecoder.create("utf-8");
        // let body = textDecoder.decodeWithStream( bytes , {stream: false});
      }
    } catch (e) {
        LogUtils.e(NETWORK_MODULE_TAG,e)
    }
    responseMap.set("respBody", body);
    return responseMap
  }

  normalizeRequest(request: HippyMap, requestHeaders: HashMap<String, String>, requestParams: HashMap<String, String>) {
    let entry = request.entries();
    if (entry !== null) {
      for (let header of request.entries()) {
        let key = header[0]
        let value = header[1]
        if (key == null || value == null) {
          continue;
        }
        if (key == HippyHttpRequest.HTTP_HEADERS && value instanceof Map) {
          this.normalizeRequestHeaders(value, requestHeaders);
          continue;
        }
        requestParams.set(key, value.toString());
      }
    }
  }

  normalizeRequestHeaders(headers: HippyMap, requestHeaders: HashMap<String, String>) {
    if (headers.entries() == null) return
    for (let header of headers.entries()) {
      let key = header[0]
      let value = header[1]
      if (key == null || value == null) {
        continue;
      }
      if (value instanceof String) {
        requestHeaders.set(key, value.toString());
      }
    }
  }

  public getCookie(url: string): Promise<void> {
    return new Promise((resolve, reject) => {
      let adapter = this.ctx.getGlobalConfigs().getHttpAdapter();
      if (adapter != null) {
        adapter.getCookie(url);
      }
    })
  }

  public setCookie(url: string, keyValue: string, expires: string) {
    let adapter = this.ctx.getGlobalConfigs().getHttpAdapter();
    if (adapter != null) {
      adapter.setCookie(url, keyValue, expires);
    }
  }

  call(method: string, params: HippyAny, callback: HippyModuleCallback): HippyAny {
    return
  }
}

class FetchResCallback implements FetchResourceCallback {
  private netModule: NetworkModule

  constructor(ctx: HippyEngineContext) {
    this.netModule = new NetworkModule(ctx)
  }

  onFetchCompleted(holder: ResourceDataHolder): void {
    this.netModule.handleFetchResult(holder);
    holder.recycle();
  }

  onFetchProgress(total: number, loaded: number): void {
    // Nothing need to do here.
  }
}

