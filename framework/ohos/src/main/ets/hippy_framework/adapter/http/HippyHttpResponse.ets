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
import { TextUtils } from '../../../support/common/TextUtils';

export class HippyHttpResponse {
  public static UNKNOWN_STATUS: number = -1;
  public static HTTP_RESPONSE_STATUS_CODE: string = "statusCode";
  public static HTTP_RESPONSE_RESPONSE_MESSAGE: string = "responseMessage";
  private mStatusCode: number = HippyHttpResponse.UNKNOWN_STATUS;
  private mResponseMessage: string;
  private mRspHeaderMap: Map<String, List<String>> = null;
  private mInputStream: fs.Stream;
  private mErrorStream: fs.Stream;

  public getStatusCode(): number {
    return this.mStatusCode != null ? this.mStatusCode : HippyHttpResponse.UNKNOWN_STATUS;
  }

  public setStatusCode(statusCode: number): void {
    this.mStatusCode = statusCode;
  }

  public setRspHeaderMap(headerMap: Map<String, List<String>>): void {
    this.mRspHeaderMap = headerMap;
  }

  public getRspHeaderMaps(): Map<String, List<String>> {
    return this.mRspHeaderMap;
  }

  public getHeaderFields(name: string): List<String> {
    if (TextUtils.isEmpty(name) || this.mRspHeaderMap == null) {
      return null;
    }

    return this.mRspHeaderMap.get(name);
  }

  public getHeaderField(name: string): String {
    if (TextUtils.isEmpty(name) || this.mRspHeaderMap == null) {
      return null;
    }

    let fields: List<String> = this.mRspHeaderMap.get(name);
    return fields != null && fields.length > 0 ? fields.get(0) : null;
  }

  public getInputStream(): fs.Stream {
    return this.mInputStream;
  }

  public setInputStream(inputStream: fs.Stream): void {
    this.mInputStream = inputStream;
  }

  public getErrorStream(): fs.Stream {
    return this.mErrorStream;
  }

  public setErrorStream(errorStream: fs.Stream): void {
    this.mErrorStream = errorStream;
  }

  public setResponseMessage(message: string): void {
    this.mResponseMessage = message;
  }

  public getResponseMessage(): string {
    return this.mResponseMessage;
  }

  public close(): void {
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
