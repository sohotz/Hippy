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
import { HippyAny, HippyMap } from '../../../../support/common/HippyTypes'
import { VfsManager } from '../../../../vfs/VfsManager'
import { ResourceDataHolder } from '../../../../vfs/ResourceDataHolder'
import { HippyEngineContext } from '../../../HippyEngineContext'
import { HippyNativeModuleBase } from '../HippyNativeModuleBase'
import { HashMap } from '@kit.ArkTS'
import { TextUtils } from '../../../../support/utils/TextUtils'
import { HippyHttpRequest } from '../../../adapter/http/HippyHttpRequest'
import { HippyHttpResponse } from '../../../adapter/http/HippyHttpResponse'
import { LogUtils } from '../../../../support/utils/LogUtils'
import { FetchResourceCallback } from '../../../../vfs/FetchResourceCallback'
import { HippyModulePromise } from '../../HippyModulePromise'

const NETWORK_MODULE_TAG = "NetworkModule"

@HippyNativeModule({ name: "network" })
export class NetworkModule extends HippyNativeModuleBase {
  public static readonly NAME = 'network'

  constructor(ctx: HippyEngineContext) {
    super(ctx)
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

  public call(method: string, params: Array<HippyAny>, promise: HippyModulePromise): HippyAny {
    switch (method) {
      case 'fetch': {
        this.fetch(params[0] as HippyMap, promise)
        break;
      }
      case 'setCookie': {
        if (params != null && params.length > 2) {
          let url = ""
          let keyValue = ""
          let expires = ""
          if (params[0] !== undefined) {
            url = params[0] as string
          }
          if (params[1] !== undefined) {
            keyValue = params[1] as string
          }
          if (params[2] !== undefined) {
            expires = params[2] as string
          }
          this.setCookie(url, keyValue, expires)
        }
        break;
      }
      case 'addListener': {
        this.addListener()
        break;
      }
      case 'removeListener': {
        this.removeListener()
        break;
      }
      case 'getCookie': {
        this.getCookie(params[0] as string)
        break;
      }
      default:
        super.call(method, params, promise)
    }
    return null;
  }

  private fetch(request: HippyMap, promise: HippyModulePromise) {
    let map = new Map<string, string>()
    let vfsManager: VfsManager = this.ctx.getVfsManager()
    let requestHeaders = new HashMap<string, string>();
    let requestParams = new HashMap<string, string>();
    try {
      this.normalizeRequest(request, requestHeaders, requestParams);
    } catch (e) {
      map.set('request', e.getmessage)
      promise.resolve(map);
      return;
    }
    let uri: string = requestParams.get(HippyHttpRequest.HTTP_URL);
    if (TextUtils.isEmpty(uri)) {
      map.set('request', 'Get url parameter failed!')
      promise.resolve(map);
      return;
    }

    vfsManager.fetchResourceAsync(uri, requestHeaders, requestParams,
      new FetchResCallback(this.ctx));
  }

  private handleFetchResponse(dataHolder: ResourceDataHolder): HippyAny {
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
      let bytes = dataHolder.getBuffer();
      if (bytes != null) {
        let textDecoder = TextUtils.getUtf8Decoder();
        // let body = textDecoder.decodeWithStream( bytes , {stream: false});
      }
    } catch (e) {
      LogUtils.e(NETWORK_MODULE_TAG, e)
    }
    responseMap.set("respBody", body);
    return responseMap
  }

  private normalizeRequest(request: HippyMap, requestHeaders: HashMap<string, string>, requestParams: HashMap<String, String>) {
    if (request instanceof Object) {
      let entry: string[][] = Object.entries(request)
      if (entry !== null && entry.length > 0) {
        for (let i = 0; i < entry.length; i++) {
          LogUtils.i(NETWORK_MODULE_TAG, entry[i].toString());
          let key = entry[i][0]
          let value = entry[i][1]
          if (key == null || value == null) {
            continue;
          }
          if (key == HippyHttpRequest.HTTP_HEADERS) {

            this.normalizeRequestHeaders(value, requestHeaders);
            continue;
          }
          requestParams.set(key, value)
        }
      }
    }
  }

  private normalizeRequestHeaders(headers: string, requestHeaders: HashMap<string, string>) {
    if (Object.entries(headers) == null) return
    let valueEntry: string[][] = Object.entries(headers)
    if (valueEntry == null || valueEntry.length < 1) return

    for (let j = 0; j < valueEntry.length; j++) {
      LogUtils.i(NETWORK_MODULE_TAG, valueEntry[j].toString());
      if (valueEntry[j][0] == null || valueEntry[j][1] == null) {
        continue;
      }
      requestHeaders.set(valueEntry[j][0], valueEntry[j][1])
    }
  }

  private getCookie(url: string): Promise<string> {
    return new Promise(() => {
      let adapter = this.ctx.getGlobalConfigs().getHttpAdapter();
      if (adapter != null) {
        adapter.getCookie(url);
      }
    })
  }

  private setCookie(url: string, keyValue: string, expires: string) {
    let adapter = this.ctx.getGlobalConfigs().getHttpAdapter();
    if (adapter != null) {
      adapter.setCookie(url, keyValue, expires);
    }
  }

  private addListener() {

  }

  private removeListener() {

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
