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
import { HippyHttpAdapter, HttpTaskCallback } from './HippyHttpAdapter';
import { HippyHttpRequest } from './HippyHttpRequest';
import { HippyHttpResponse } from './HippyHttpResponse';
import { BusinessError } from '@ohos.base';
import { ProcessorCallback } from '../../../vfs/ProcessorCallback';
import { ResourceDataHolder } from '../../../vfs/ResourceDataHolder';
import { FetchResultCode } from './HippyResourceLoader';
import { HippyException } from '../../../support/common/HippyException';
import { DefaultHttpAdapter } from './DefaultHttpAdapter';
import web_webview from '@ohos.web.webview'
import { HttpHeaderResponse } from './HttpHeaderResponse';
import buffer from '@ohos.buffer';
import { util } from '@kit.ArkTS';

export class HttpTaskCallbackImpl implements HttpTaskCallback {
  private mCallback: ProcessorCallback;
  private mDataHolder: ResourceDataHolder;

  constructor(holder: ResourceDataHolder, callback: ProcessorCallback) {
    this.mCallback = callback;
    this.mDataHolder = holder;
  }

  onTaskSuccess(request: HippyHttpRequest, response: HippyHttpResponse) {
    this.mDataHolder.resultCode = FetchResultCode.OK.valueOf();
    this.mDataHolder.addResponseHeaderProperty(HippyHttpResponse.HTTP_RESPONSE_STATUS_CODE,
      response.getStatusCode().toString());
    this.mDataHolder.addResponseHeaderProperty(HippyHttpResponse.HTTP_RESPONSE_RESPONSE_MESSAGE,
      response.getResponseMessage());
    let inputStream = response.getInputStream();
    if (response.getStatusCode() != 200 || inputStream == undefined) {
      let errorStream = response.getErrorStream()
      if (errorStream != null && errorStream != undefined) {
        let buf = new ArrayBuffer(4096);
        errorStream.read(buf, { offset: 5, length: 5 }, (err, readLen) => {
          if (err) {
            console.error("read stream failed with error message: " + err.message + ", error code: " + err.code);
          } else {
            console.info("read data succeed");
            let unit8Array = new Uint8Array(buf.slice(0, readLen))
            let decoder = util.TextDecoder.create('utf-8', { ignoreBOM: true })
            this.mDataHolder.errorMessage = decoder.decodeWithStream(unit8Array, { stream: false })
            console.log("response errorMessage: " + this.mDataHolder.errorMessage)
          }
        });
      }
      this.mCallback.onHandleCompleted()
      return
    }
    try { //todo 处理gzip类型的数据流
      if (DefaultHttpAdapter.isGzipRequest(request)) {
        // inputStream = new GZIPInputStream(inputStream);
      }
      this.mDataHolder.readResourceDataFromStream(inputStream);
    } catch (e) {
      this.mDataHolder.errorMessage = e.getMessage();
      this.mCallback.onHandleCompleted();
      return;
    }

    let headers = response.getRspHeaderMaps();
    if (headers != null) {
      for (let header of headers.entries()) {
        let key = header[0]
        let list = header[1]
        if (list != null && !list.isEmpty()) {
          if (HttpHeaderResponse.SET_COOKIE.toLowerCase() === key.toLowerCase()) {
            for (let item of list) {
              web_webview.WebCookieManager.setCookie(request.getUrl(), item)
            }
          }
        }
        if (list.length == 1) {
          this.mDataHolder.addResponseHeaderProperty(key, list.get(0));
        } else if (list.length > 1) { //todo  still can be improve
          let value = ''
          for (let i = 0; i < list.length; i++) {
            if (i != list.length - 1) {
              value += list[i] + HippyHttpRequest.HTTP_HEADERS_SEPARATOR
            }
          }
          this.mDataHolder.addResponseHeaderProperty(key, value);
        }
      }
    }
    this.mCallback.onHandleCompleted();
  }


  onTaskFailed(request: HippyHttpRequest
    , error: HippyException) {
    this.mDataHolder.resultCode = FetchResultCode.ERR_REMOTE_REQUEST_FAILED.valueOf();
    if (error != null) {
      this.mDataHolder.errorMessage = error.message;
    }

    this.mCallback.onHandleCompleted();
  }
}
