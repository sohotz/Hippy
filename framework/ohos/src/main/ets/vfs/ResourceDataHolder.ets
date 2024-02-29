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

import fs from '@ohos.file.fs';
import HashMap from '@ohos.util.HashMap';
import { HippyResource } from '../support/common/HippyTypes';

export interface FetchResourceCallback {
  /**
   * After the process chain traversal is completed, we need to call this method to
   * return the processing results to the original request initiator.
   *
   * @param dataHolder holder of resources fetch result
   */
  onFetchCompleted(holder: ResourceDataHolder)

  /**
   * Return the current progress when loading resources.
   *
   * @param total the total size of resources
   * @param loaded the current progress has loaded
   */
  onFetchProgress(total: number, loaded: number)
}

export class ResourceDataHolder {
  static RESOURCE_LOAD_SUCCESS_CODE: number = 0;
  errorMessage: string | undefined
  processorTag: string | undefined
  resultCode: number = -1
  public uri: string
  public resource: HippyResource
  responseHeaders: HashMap<string, string> | undefined
  public requestHeaders: HashMap<string, string> | undefined
  public requestParams: HashMap<string, string> | undefined
  public callback: FetchResourceCallback | null
  public requestFrom: RequestFrom
  public nativeRequestId: number
  public index: number = -1

  constructor(uri: string, requestHeaders: HashMap<string, string> | undefined,
              requestParams: HashMap<string, string> | undefined, callback: FetchResourceCallback | null,
              from: RequestFrom, nativeRequestId: number) {
    this.uri = uri
    this.resource = ""
    this.requestHeaders = requestHeaders
    this.requestParams = requestParams
    this.callback = callback
    this.requestFrom = from
    this.nativeRequestId = nativeRequestId
  }

  addResponseHeaderProperty(key: string, property: string) {
    if (this.responseHeaders == null) {
      this.responseHeaders = new HashMap <string, string>();
    }
    this.responseHeaders.set(key, property)
  }

  readResourceDataFromStream(stream: fs.Stream) {

  }
}

export enum RequestFrom {
  NATIVE,
  LOCAL,
}

export enum TransferType {
  NORMAL,
  NIO,
}
