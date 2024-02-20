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
import { HippyMap } from '../../../support/common/HippyTypes';
import { HttpHeaderReuqest } from './HttpHeaderRequest';
import buffer from '@ohos.buffer';
import { TextUtils } from '../../../support/common/TextUtils';

export class HippyHttpRequest {
  public static DEFAULT_TIMEOUT_MS: number = 3000;
  public static HTTP_HEADERS: string = "headers";
  public static HTTP_HEADERS_SEPARATOR: string = ",";
  public static HTTP_URL: string = "url";
  public static HTTP_METHOD: string = "method";
  public static HTTP_REDIRECT: string = "redirect";
  public static HTTP_BODY: string = "body";
  private static USER_AGENT: string = null;
  private mConnectTimeout: number = HippyHttpRequest.DEFAULT_TIMEOUT_MS;
  private mReadTimeout: number = HippyHttpRequest.DEFAULT_TIMEOUT_MS;
  private mUseCaches: boolean = true;
  // private mRedirectTimes: AtomicInteger
  private mUrl: string
  private mHeaders: Map<string, string>
  private mInitParams: Map<string, string>
  private mNativeParams: Map<string, Object>

  constructor(headers: Map<string, string>,
              initParams: Map<string, string>, nativeParams: Map<string, Object>) {
    this.mHeaders = (headers == null) ? new Map() : headers;
    this.mInitParams = initParams;
    this.mNativeParams = nativeParams;
    this.initUserAgent();
  }

  public getRequestCookies(): string {
    return this.mHeaders.get(HttpHeaderReuqest.COOKIE);
  }

  public getNativeParams(): Map<string, Object> {
    return this.mNativeParams;
  }

  public setUrl(url: string): void {
    this.mUrl = url;
  }

  public getUrl(): string {
    if (this.mUrl != null) {
      return this.mUrl;
    }
    return (this.mInitParams != null) ? this.mInitParams.get(HippyHttpRequest.HTTP_URL) : null;
  }

  public addHeader(name: string, value: string): void {
    this.mHeaders.set(name, value);
  }

  public getAndIncrementRedirectTimes(): number {
    // if (this.mRedirectTimes == null) {
      // this.mRedirectTimes = new AtomicInteger();
    // }
    // return this.mRedirectTimes.getAndIncrement();
    return 0
  }


  public getHeaders(): Map<String, String> {
    return this.mHeaders;
  }

  public getConnectTimeout(): number {
    return this.mConnectTimeout;
  }

  public setConnectTimeout(time: number): void {
    this.mConnectTimeout = time;
  }

  public getReadTimeout(): number {
    return this.mReadTimeout;
  }

  public setReadTimeout(time: number): void {
    this.mReadTimeout = time;
  }

  public isUseCaches(): boolean {
    return this.mUseCaches;
  }

  public setUseCaches(useCaches: boolean): void {
    this.mUseCaches = useCaches;
  }

  public getMethod(): string {
    if (this.mInitParams != null) {
      let method: string = this.mInitParams.get(HippyHttpRequest.HTTP_METHOD);
      if (!TextUtils.isEmpty(method)) {
        return method;
      }
    }
    return "GET";
  }

  public isInstanceFollowRedirects(): boolean {
    let redirect: string = (this.mInitParams != null) ? this.mInitParams.get(HippyHttpRequest.HTTP_REDIRECT) : null;
    return!TextUtils.isEmpty(redirect) && TextUtils.equals("follow", redirect);
  }

  public getBody(): string {
    return (this.mInitParams != null) ? this.mInitParams.get(HippyHttpRequest.HTTP_BODY) : null;
  }

  private initUserAgent(): void {
    if (HippyHttpRequest.USER_AGENT == null) {
      // Locale locale = Locale.getDefault();
      // let buff:buffer.Buffer= new buffer.Buffer();
      // // Add version
      // let version:string = build.VERSION.RELEASE;
      // if (version.length > 0) {
      //     // buffer.append(version);
      // } else {// default to "1.0"
      //     buffer.append("1.0");
      // }
      // buffer.append("; ");
      // final String language = locale.getLanguage();
      // buffer.append(language.toLowerCase());
      // final String country = locale.getCountry();
      // buffer.append("-");
      // buffer.append(country.toLowerCase());
      // final String base = "Mozilla/5.0 (Linux; U; Android %s) AppleWebKit/533.1 (KHTML, like Gecko) Mobile Safari/533.1";
      // HippyHttpRequest.USER_AGENT = String.format(base, buffer);
    }

    this.addHeader(HttpHeaderReuqest.USER_AGENT, HippyHttpRequest.USER_AGENT);
  }
}
