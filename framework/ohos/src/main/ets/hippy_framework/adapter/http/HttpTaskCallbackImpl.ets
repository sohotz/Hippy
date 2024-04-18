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
import { HttpTaskCallback } from './HippyHttpAdapter';
import { HippyHttpRequest } from './HippyHttpRequest';
import { HippyHttpResponse } from './HippyHttpResponse';
import { ProcessorCallback } from '../../../vfs/ProcessorCallback';
import { ResourceDataHolder } from '../../../vfs/ResourceDataHolder';
import { FetchResultCode } from './HippyResourceLoader';
import { HippyException } from '../../../support/common/HippyException';
import web_webview from '@ohos.web.webview';
import { HttpHeaderResponse } from './HttpHeaderResponse';
import { LogUtils } from '../../../support/utils/LogUtils';
import { ReadOptions } from '@ohos.file.fs';
import { buffer } from '@kit.ArkTS';
import { HippyJsException } from '../../common/HippyJsException';

const HTTP_TASK_CALLBACK_IMPL_TAG = "HttpTaskCallbackImpl"

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
    if (response.getStatusCode() != 200) {
      if (response.getErrorStream() != null) {
        let arrayBuffer = new ArrayBuffer(4096);
        let readOption: ReadOptions = {
          offset: 5,
          length: 5
        };
        response.getErrorStream()?.read(arrayBuffer, readOption).then((readLen: number) => {
          let buf = buffer.from(arrayBuffer, 0, readLen);
          LogUtils.d(HTTP_TASK_CALLBACK_IMPL_TAG, `read data succeed, content of file: ${buf.toString()}`);
          this.mDataHolder.errorMessage = buf.toString();
          this.mCallback.onHandleCompleted();
        }).catch((err: HippyJsException) => {
          LogUtils.e(HTTP_TASK_CALLBACK_IMPL_TAG, "read data failed with error message: " + err.message);
        });
      }
      return;
    }
    // this.mDataHolder.readResourceDataFromStream(inputStream);
    let headers = response.getRspHeaderMaps();
    if (headers != null && headers.size > 0) {
      for (let header of headers.entries()) {
        let key = header[0]
        let list = header[1]
        if (list != null && !list.isEmpty()) {
          if (HttpHeaderResponse.SET_COOKIE.toLowerCase() === key.toLowerCase()) {
            for (let item of list) {
              web_webview.WebCookieManager.configCookieSync(request.getUrl(), item)
            }
          }
        }
        if (list.length == 1) {
          this.mDataHolder.addResponseHeaderProperty(key, list.get(0));
        } else if (list.length > 1) { //todo  still can be improved
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
      LogUtils.d(HTTP_TASK_CALLBACK_IMPL_TAG, "error : " + this.mDataHolder.errorMessage)
    }
    this.mCallback.onHandleCompleted();
  }
}
