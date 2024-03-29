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

import fs, { ReadOptions } from '@ohos.file.fs';
import HashMap from '@ohos.util.HashMap';
import { HippyJsException } from '../hippy_framework/common/HippyJsException';
import { FetchResourceCallback } from './FetchResourceCallback';
import { buffer } from '@kit.ArkTS';
import { LogUtils } from '../support/utils/LogUtils';

const RESOURCE_DATA_TAG = "ResourceDataHolder"

export class ResourceDataHolder {
  static RESOURCE_LOAD_SUCCESS_CODE: number = 0;
  errorMessage: string | null = null
  processorTag: string | null = null
  resultCode: number = -1
  public uri: string
  public responseHeaders: HashMap<string, string> | null = null
  public requestHeaders: HashMap<string, string> | null
  public requestParams: HashMap<string, string> | null
  public callback: FetchResourceCallback | null
  public requestFrom: RequestFrom
  public nativeRequestId: number
  public index: number = -1
  public transferType: TransferType = TransferType.NORMAL;
  public buffer: ArrayBuffer | null = null
  public bytes: Uint8Array | null = new Uint8Array()
  public loadStartTime = 0;
  // 文件存储路径
  public filePath = ""

  constructor(uri: string, requestHeaders: HashMap<string, string> | null,
              requestParams: HashMap<string, string> | null, callback: FetchResourceCallback | null,
              from: RequestFrom, nativeRequestId: number) {
    this.uri = uri
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
    let arrayBuffer: ArrayBuffer | null = new ArrayBuffer(4096);
    let readOption: ReadOptions = {
      offset: 5,
      length: 5
    };
    stream.read(arrayBuffer, readOption).then((readLen: number) => {
      LogUtils.d(RESOURCE_DATA_TAG, "read data succeed");
      this.buffer = buffer.from(arrayBuffer, 0, readLen).buffer;
      LogUtils.d(RESOURCE_DATA_TAG, `The content of file: ${this.buffer.toString()}`);
      // this.bytes = new Uint8Array(this.buffer.buffer);
      stream.close();
    }).catch((err: HippyJsException) => {
      LogUtils.e(RESOURCE_DATA_TAG, "read data failed with error message: " + err.message);
    });
  }

  public recycle() {
    this.buffer = null;
    this.bytes = null;
    this.loadStartTime = 0;
    this.transferType = TransferType.NORMAL;
    this.errorMessage = null;
    this.callback = null;
    this.processorTag = null;
    this.index = -1;
    this.resultCode = -1;
  }

  public getBytes(): ArrayBuffer | null { //todo
    // if (this.transferType == TransferType.NORMAL) {
    //   return this.bytes;
    // } else if (this.buffer != null) {
    //   let bytes = new Uint8Array(this.buffer.buffer);
    //   return bytes;
    // }
    return this.buffer;
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
