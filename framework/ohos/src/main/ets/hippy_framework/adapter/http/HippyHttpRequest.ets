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
import { TextUtils } from '../../../support/utils/TextUtils';
import geoLocationManager from '@ohos.geoLocationManager';
import I18n from '@ohos.i18n';
import { HippyEngineContext } from '../../HippyEngineContext';
import { featureAbility } from '@kit.AbilityKit';
import { HashMap, util } from '@kit.ArkTS';
import { http } from '@kit.NetworkKit';
import { LogUtils } from '../../../support/utils/LogUtils';

const HIPPY_HTTP_REQUEST_TAG = "HippyHttpRequest"

export class HippyHttpRequest {
  private context: HippyEngineContext
  public static DEFAULT_TIMEOUT_MS: number = 3000;
  public static HTTP_HEADERS: string = "headers";
  public static HTTP_HEADERS_SEPARATOR: string = ",";
  public static HTTP_URL: string = "url";
  public static HTTP_METHOD: string = "method";
  public static HTTP_REDIRECT: string = "redirect";
  public static HTTP_BODY: string = "body";
  private static USER_AGENT = "";
  private mConnectTimeout: number = HippyHttpRequest.DEFAULT_TIMEOUT_MS;
  private mReadTimeout: number = HippyHttpRequest.DEFAULT_TIMEOUT_MS;
  private mUseCaches: boolean = true;
  private mRedirectTimes: number = 0
  private mUrl: string = ""
  private mHeaders: HashMap<string, string>
  private mMethod: http.RequestMethod = http.RequestMethod.GET
  private mInitParams: HashMap<string, string>
  private mNativeParams: HashMap<string, Object>

  constructor(context: HippyEngineContext, headers: HashMap<string, string>, method: http.RequestMethod,
              initParams: HashMap<string, string>, nativeParams: HashMap<string, Object>) {
    this.context = context
    this.mHeaders = (headers == null) ? new HashMap() : headers;
    this.mMethod = method;
    this.mInitParams = initParams;
    this.mNativeParams = nativeParams;
    this.initUserAgent();
  }

  public getRequestCookies(): string | undefined {
    if (this.mHeaders != null) {
      return this.mHeaders.get(HttpHeaderReuqest.COOKIE);
    }
    return undefined
  }

  public getNativeParams(): HashMap<string, Object> {
    return this.mNativeParams;
  }

  public setUrl(url: string): void {
    this.mUrl = url;
  }

  public getUrl(): string | null | undefined {
    if (this.mUrl != null) {
      return this.mUrl;
    }
    return (this.mInitParams != null) ? this.mInitParams.get(HippyHttpRequest.HTTP_URL) : null;
  }

  public addHeader(name: string, value: string) {
    this.mHeaders.set(name, value);
  }

  public getAndIncrementRedirectTimes(): number {
    return this.mRedirectTimes++;
  }


  public getHeaders(): HashMap<string, string> {
    return this.mHeaders;
  }

  public getConnectTimeout(): number {
    return this.mConnectTimeout;
  }

  public setConnectTimeout(time: number) {
    this.mConnectTimeout = time;
  }

  public getReadTimeout(): number {
    return this.mReadTimeout;
  }

  public setReadTimeout(time: number) {
    this.mReadTimeout = time;
  }

  public isUseCaches(): boolean {
    return this.mUseCaches;
  }

  public setUseCaches(useCaches: boolean) {
    this.mUseCaches = useCaches;
  }

  public getMethod(): http.RequestMethod {
    return this.mMethod;
  }

  public isInstanceFollowRedirects(): boolean {
    let redirect = (this.mInitParams != null) ? this.mInitParams.get(HippyHttpRequest.HTTP_REDIRECT) : null;
    return!TextUtils.isEmpty(redirect) && TextUtils.equals("follow", redirect);
  }

  public getBody(): string | Object | ArrayBuffer {
    if (this.mInitParams == null) return ""
    let jsonObject: Record<string, Object> = {};
    this.mInitParams.forEach((value, key) => {
      if (key != undefined && value != undefined) {
        jsonObject[key] = value;
      }
    })
    LogUtils.d(HIPPY_HTTP_REQUEST_TAG, JSON.stringify(jsonObject));
    return JSON.stringify(jsonObject)
  }

  private initUserAgent(): void {
    if (HippyHttpRequest.USER_AGENT == null) {
      try {
        let language = I18n.System.getSystemLanguage();
        let country = I18n.System.getSystemRegion(); // 获取系统当前地区设置

        // Add version
        let buffer = "" //todo context
        // context.getAppVersionInfo((err, data) => {
        //     console.info('getAppVersionInfo err: ' + JSON.stringify(err) + 'data: ' + JSON.stringify(data));
        //     version =
        // });
        let version: string = ""
        if (version.length > 0) {
          buffer += version + "; "
        } else { // default to "1.0"
          buffer += "1.0; "
        }
        buffer += language.toLowerCase()
        buffer += "-"
        buffer += country.toLowerCase()
        let base = "Mozilla/5.0 (Linux; U; Android %s) AppleWebKit/533.1 (KHTML, like Gecko) Mobile Safari/533.1";
        HippyHttpRequest.USER_AGENT = util.format(base, buffer);
      } catch (error) {
        console.error(`call System.isSuggested failed, error code: ${error.code}, message: ${error.message}.`);
      }
    }

    this.addHeader(HttpHeaderReuqest.USER_AGENT, HippyHttpRequest.USER_AGENT);
  }
}
