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
import List from '@ohos.util.List';
import fs from '@ohos.file.fs';
import { TextUtils } from '../../../support/utils/TextUtils';

export class HippyHttpResponse {
  public static UNKNOWN_STATUS: number = -1;
  public static HTTP_RESPONSE_STATUS_CODE: string = "statusCode";
  public static HTTP_RESPONSE_RESPONSE_MESSAGE: string = "responseMessage";
  public buffer: ArrayBuffer | null = null
  private mStatusCode: number = HippyHttpResponse.UNKNOWN_STATUS;
  private mResponseMessage: string = ""
  private mRspHeaderMap: Map<string, List<string>> | null = null
  private mInputStream: fs.Stream | null = null;
  private mErrorStream: fs.Stream | null = null;

  public getStatusCode(): number {
    return this.mStatusCode != null ? this.mStatusCode : HippyHttpResponse.UNKNOWN_STATUS;
  }

  public setStatusCode(statusCode: number) {
    this.mStatusCode = statusCode;
  }

  public setRspHeaderMap(headerMap: Map<string, List<string>>) {
    this.mRspHeaderMap = headerMap;
  }

  public getRspHeaderMaps(): Map<string, List<string>> | null {
    return this.mRspHeaderMap;
  }

  public getHeaderFields(name: string): List<string> | undefined {
    if (TextUtils.isEmpty(name) || this.mRspHeaderMap == undefined) {
      return undefined;
    }
    return this.mRspHeaderMap.get(name);
  }

  public getHeaderField(name: string): string {
    if (TextUtils.isEmpty(name) || this.mRspHeaderMap == null) {
      return "";
    }

    let fields = this.mRspHeaderMap.get(name);
    return fields != null && fields.length > 0 ? fields.get(0) : "";
  }

  public setResponseMessage(message: string) {
    this.mResponseMessage = message;
  }

  public getResponseMessage(): string {
    return this.mResponseMessage;
  }

  public getInputStream(): fs.Stream | null {
    return this.mInputStream;
  }

  public setInputStream(inputStream: fs.Stream) {
    this.mInputStream = inputStream
  }

  public getErrorStream(): fs.Stream | null {
    return this.mErrorStream;
  }

  public setErrorStream(errStream: fs.Stream) {
    this.mInputStream = errStream
  }

  public close() {
    if (this.mInputStream != null) {
      try {
        this.mInputStream.close();
      } catch (e) {
        e.printStackTrace();
      }

      if (this.mErrorStream != null) {
        try {
          this.mErrorStream.close();
        } catch (e) {
          e.printStackTrace();
        }
      }
    }

  }
}
