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

export class HttpTaskCallbackImpl implements HttpTaskCallback {
  // private mCallback: ProcessorCallback;
  // private mDataHolder: ResourceDataHolder;
  //
  // constructor(holder: ResourceDataHolder, callback: ProcessorCallback) {
  //   this.mCallback = callback;
  //   this.mDataHolder = holder;
  // }
  //
  onTaskSuccess(request: HippyHttpRequest, response: HippyHttpResponse): void {
    // this.mDataHolder.resultCode = FetchResultCode.OK.ordinal();
    // this.mDataHolder.addResponseHeaderProperty(HTTP_RESPONSE_STATUS_CODE,
    //   response.getStatusCode().toString());
    // this.mDataHolder.addResponseHeaderProperty(HTTP_RESPONSE_RESPONSE_MESSAGE,
    //   response.getResponseMessage());
    //todo onTaskSuccess...
  }

  onTaskFailed(request: HippyHttpRequest, error: BusinessError<void>): void {
    throw new Error('Method not implemented.');
    //todo onTaskSuccess...
  }
}
