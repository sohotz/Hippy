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
import { Hippy1Map, HippyMap } from '../support/common/HippyTypes';
import HashMap from '@ohos.util.HashMap';
import fs from '@ohos.file.fs';

//todo  need to be changed to vfsFile
export class ResourceDataHolder {
  static RESOURCE_LOAD_SUCCESS_CODE: number = 0;
  errorMessage: string | undefined
  processorTag: string | undefined
  resultCode: number = -1
  responseHeaders: HashMap<string, string> | undefined
  requestHeaders: HashMap<string, string> | undefined
  requestParams: HashMap<string, string> | undefined
  uri: string = ""
  requestFrom:RequestFrom|undefined

  constructor(uri: string, requestHeaders: HashMap<string, string>, requestParams: HashMap<string, string>, from: RequestFrom) {
    this.init(uri, requestHeaders, requestParams, null, from, -1);
  }

  init(uri: string, requestHeaders: HashMap<string, string>, requestParams: HashMap<string, string>, arg3: null, from: RequestFrom, arg5: number) {
    this.uri = uri;
    this.requestHeaders = requestHeaders;
    this.requestParams = requestParams;
    // this.callback = callback;
    this.requestFrom = from;
    // this.nativeRequestId = nativeRequestId;
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
