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
import HashMap from '@ohos.util.HashMap';
import { HttpHeaderReuqest } from './HttpHeaderRequest';
import List from '@ohos.util.List';
import hilog from '@ohos.hilog';
import web_webview from '@ohos.web.webview';
import taskpool from '@ohos.taskpool';
import { TextUtils } from '../../../support/common/TextUtils';
import { HttpTaskCallbackImpl } from './HttpTaskCallbackImpl';
import Url from '@ohos.url'

const DOMAIN = 0x0002
const TAG = 'DefaultHttpAdapter'

export class DefaultHttpAdapter implements HippyHttpAdapter {
  private mExecutorService: taskpool.TaskGroup

  // protected execute(runnable: Runnable): void {
  //   if (this.mExecutorService == null) {
  //     this.mExecutorService = Executors.newFixedThreadPool(4);
  //   }
  //   this.mExecutorService.execute(runnable);
  // }

  public destroyIfNeed(): void {
    throw new Error('Method not implemented.');
  }

  public sendRequest(request: HippyHttpRequest, callback: HttpTaskCallback): void {
    throw new Error('Method not implemented.');
  }

  // public fetch(holder: ResourceDataHolder, nativeParams: HashMap<string, Object>, callback: ProcessorCallback): void {
  //   holder.processorTag = DefaultHttpAdapter.name
  //   let httpRequest: HippyHttpRequest = this.generateHttpRequest(holder, nativeParams);
  //   this.handleRequestCookie(httpRequest);
  //   this.sendRequest(httpRequest, new HttpTaskCallbackImpl(holder, callback));
  // }

  public handleRequestCookie(httpRequest: HippyHttpRequest) {
    let url: string = httpRequest.getUrl();
    if (url != null) {
      let cookies: string = httpRequest.getRequestCookies();
      this.saveCookieToManager(url, cookies, null);
      if (this.getCookieManager()) {
        let cookie: string = web_webview.WebCookieManager.getCookie(url);
        if (!TextUtils.isEmpty(cookie)) {
          httpRequest.addHeader(HttpHeaderReuqest.COOKIE, cookie);
        }
      }
    }
  }

  protected saveCookie2Manager(url: string, cookies: List<String>) {
    for (let i = 0; i < cookies.length; i++) {
      let cookie: String = cookies.get(i);
      this.saveCookieToManager(url, cookie, null);
    }
  }

  protected saveCookieToManager(url: string, cookies: String,
                                expires: string): void {
    if (this.getCookieManager() || cookies == null) {
      return;
    }
    cookies = cookies.replace("\\s+", "");
    let cookieItems: string[] = cookies.split(";");
    cookieItems.forEach((cookie) => {
      if (cookie != null && cookie.trim().length > 0) {
        let newCookie: string = this.resetCookieIfNeeded(cookie, expires);
        web_webview.WebCookieManager.setCookie(url, newCookie)
      }
    })
  }

  protected getCookieManager(): boolean {
    try {
      if (!web_webview.WebCookieManager.isCookieAllowed()) {
        web_webview.WebCookieManager.putAcceptCookieEnabled(true);
      }
      return true
    } catch (err) {
      hilog.info(DOMAIN, TAG, "getCookieManager: " + err.getMessage());
      // https://bugs.chromium.org/p/chromium/issues/detail?id=559720
      return false
    }
  }

  public getCookie(url: string, promise: Promise<void>): void {
    throw new Error('Method not implemented.');
  }

  public setCookie(url: string, keyValue: string, expires: string): void {
    throw new Error('Method not implemented.');
  }

  // protected generateHttpRequest(holder: ResourceDataHolder, nativeParams: HashMap<string, Object>): HippyHttpRequest {
  //   let httpRequest: HippyHttpRequest = new HippyHttpRequest(holder.requestHeaders,
  //     holder.requestParams, nativeParams);
  //   httpRequest.setUrl(holder.uri);
  //   httpRequest.setConnectTimeout(10 * 1000);
  //   httpRequest.setReadTimeout(16 * 1000);
  //   httpRequest.setUseCaches(false);
  //   return httpRequest;
  // }

  protected resetCookieIfNeeded(cookie: string, expires: string): string {
    let kv: String[] = cookie.split("=");
    if (kv.length == 1 || (kv.length >= 2 && kv[1].trim().length == 0)) {
      return kv[0] + "=;Max-Age=0";
    }
    if (!TextUtils.isEmpty(expires)) {
      return cookie + ";expires=" + expires;
    }
    return cookie;
  }

  protected clearCookie(url: string): void {
    if (!web_webview.WebCookieManager.existCookie()) {
      return;
    }
    let cookies: string = web_webview.WebCookieManager.getCookie(url);
    if (TextUtils.isEmpty(cookies)) {
      return;
    }
    let cookieItems: String[] = cookies.split(";");
    cookieItems.forEach((cookie) => {
      web_webview.WebCookieManager.setCookie(url, (cookie + ";Max-Age=0"))
    })
  }

  private isGzipRequest(request: HippyHttpRequest): boolean {
    let headers: Map<String, String> = request.getHeaders();
    //todo conten

    return false;
  }

  private toURL(url: string): Url.URL { //url验证和完善
    let _URL: Url.URL = Url.URL.parseURL(url);
    if (_URL == null || _URL.toString() == "") {
      if (_URL.pathname != "" && _URL.pathname.startsWith("?")) {
        // 补斜杠符号
      }

      // 分支走到这里，没有path也没有file，证明为一个没有/的host，例如:
      // http://m.cnbeta.com(注意：后面没有/)
      if (_URL == null || _URL.toString() == "") {
        let sb: string = url + "/";
        _URL = Url.URL.parseURL(sb);
      }
    }
    return _URL;
  }
}
