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
import { LogUtils } from '../support/utils/LogUtils';
import { FetchResourceCallback } from './FetchResourceCallback';
import { buffer } from '@kit.ArkTS';
import { HippyJsException } from '../hippy_framework/common/HippyJsException';

const RESOURCE_DATA_TAG = "ResourceDataHolder"

export enum RequestFrom {
  NATIVE,
  LOCAL,
}

export enum TransferType {
  NORMAL,
  NIO,
}

export class ResourceDataHolder {
  static RESOURCE_LOAD_SUCCESS_CODE: number = 0;
  public uri: string
  public buffer: ArrayBuffer | null = null
  public bytes: Uint8Array | null = new Uint8Array()
  public requestHeaders: HashMap<string, string> | null
  public requestParams: HashMap<string, string> | null
  public responseHeaders: HashMap<string, string> | null = null
  public callback: FetchResourceCallback | null
  public transferType: TransferType = TransferType.NORMAL
  public requestFrom: RequestFrom
  public nativeRequestId: number
  public index: number = -1
  public loadStartTime: number = 0
  public resultCode: number = -1
  public requestId: string | null = null
  public errorMessage: string | null = null
  public processorTag: string | null = null
  // 文件存储路径
  public filePath = ""

  constructor(uri: string,
              requestHeaders: HashMap<string, string> | null,
              requestParams: HashMap<string, string> | null,
              callback: FetchResourceCallback | null,
              from: RequestFrom,
              nativeRequestId: number) {
    this.uri = uri
    this.requestHeaders = requestHeaders
    this.requestParams = requestParams
    this.callback = callback
    this.requestFrom = from
    this.nativeRequestId = nativeRequestId
  }

  private hashMap2Array(map: HashMap<string, string> | null): Array<string> | null {
    if (!map) {
      return null
    }

    let array = new Array<string>()
    map.forEach((value: string, key: string) => {
      array.push(key)
      array.push(value)
    })
    return array
  }

  private array2HashMap(array: Array<string> | null): HashMap<string, string> | null {
    if (!array || array.length <= 0) {
      return null
    }

    let map = new HashMap<string, string>()
    for (let i = 0; i < array.length; i += 2) {
      let key = array[i]
      if (i + 1 < array.length) {
        let value = array[i + 1]
        map.set(key, value)
      }
    }
    return map
  }

  // call from c++
  public setNativeRequestId(nativeRequestId: number) {
    this.nativeRequestId = nativeRequestId;
  }

  // call from c++
  public getNativeRequestId(): number {
    return this.nativeRequestId;
  }

  // call from c++
  public setUri(uri: string) {
    this.uri = uri;
  }

  // call from c++
  public getUri(): string {
    return this.uri;
  }

  // call from c++
  public getRequestHeadersArray(): Array<string> | null {
    return this.hashMap2Array(this.requestHeaders)
  }

  // call from c++
  public setResponseHeadersArray(responseHeadersArray: Array<string> | null) {
    this.responseHeaders = this.array2HashMap(responseHeadersArray)
  }

  // call from c++
  public getResponseHeadersArray(): Array<string> | null {
    return this.hashMap2Array(this.responseHeaders)
  }

  // call from c++
  public getResultCode(): number {
    return this.resultCode;
  }

  // call from c++
  public setResultCode(resultCode: number) {
     this.resultCode = resultCode;
  }

  // call from c++
  public setBuffer(buffer: ArrayBuffer) {
    this.buffer = buffer;
  }

  // call from c++
  public getBuffer(): ArrayBuffer | null {
    let resultBuffer: ArrayBuffer | null = null
    let filePath = this.filePath;
    let stat = fs.statSync(filePath);
    if (stat && stat.size > 0) {
      let file = fs.openSync(filePath, fs.OpenMode.READ_WRITE);
      if (file) {
        resultBuffer = new ArrayBuffer(stat.size);
        fs.readSync(file.fd, resultBuffer);
        fs.closeSync(file);
      }
    }
    return resultBuffer;
  }

  public recycle() {
    this.buffer = null;
    this.bytes = null;
    this.callback = null;
    this.errorMessage = null;
    this.processorTag = null;
    this.loadStartTime = 0;
    this.index = -1;
    this.resultCode = -1;
    this.transferType = TransferType.NORMAL;
  }

  addResponseHeaderProperty(key: string, property: string) {
    if (this.responseHeaders == null) {
      this.responseHeaders = new HashMap<string, string>()
    }
    this.responseHeaders.set(key, property)
  }

  readResourceDataFromStream(stream: fs.Stream) {
    let arrayBuffer: ArrayBuffer = new ArrayBuffer(4096);
    try {
      stream.read(arrayBuffer).then((readLen: number) => {
        LogUtils.d(RESOURCE_DATA_TAG, "read data succeed");
        let buf = buffer.from(arrayBuffer, 0, readLen)
        LogUtils.d(RESOURCE_DATA_TAG, `The content of buf: ${JSON.stringify(buf)}`);
        this.buffer = buf.buffer;
        this.bytes = new Uint8Array(this.buffer);
        LogUtils.d(RESOURCE_DATA_TAG, `The content of file: ${JSON.stringify(this.buffer)}`);
      }).catch((err: HippyJsException) => {
        LogUtils.e(RESOURCE_DATA_TAG, "read data failed with error message: " + err.message);
      });
    } catch (err) {
      LogUtils.e(RESOURCE_DATA_TAG, "read data failed : " + err.message);
    }
  }

  public getBytes(): ArrayBuffer | null {
    return this.bytes
  }
}
