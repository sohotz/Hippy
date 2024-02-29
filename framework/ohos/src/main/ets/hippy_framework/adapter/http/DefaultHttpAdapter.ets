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
import web_webview from '@ohos.web.webview';
import taskpool from '@ohos.taskpool';
import { HttpTaskCallbackImpl } from './HttpTaskCallbackImpl';
import Url from '@ohos.url'
import { TextUtils } from '../../../support/utils/TextUtils';
import { ResourceDataHolder } from 'ets/vfs/ResourceDataHolder';
import { ProcessorCallback } from '../../../vfs/ProcessorCallback';
import { HippyExecutor, HippyTask } from '../../../support/common/HippyExecutor';
import connection from '@ohos.net.connection';
import { HippyException } from '../../../support/common/HippyException';
import { LogUtils } from '../../../support/utils/LogUtils';
import { BusinessError } from '@kit.BasicServicesKit';
import { HippyHttpResponse } from './HippyHttpResponse';
import { http } from '@kit.NetworkKit';
import { call } from '@kit.TelephonyKit';

const TAG = 'DefaultHttpAdapter'

export class DefaultHttpAdapter implements HippyHttpAdapter {
  static isGzipRequest(request: HippyHttpRequest): boolean {
    let headers = request.getHeaders();
    for (let header of headers.entries()) {
      if (header[0] != null && header[0].toLowerCase() == HttpHeaderReuqest.ACCEPT_ENCODING.toLowerCase()) {
        if (TextUtils.isEmpty(header[1])) {
          return false;
        }
        let encodings: string[] = header[1].split(HippyHttpRequest.HTTP_HEADERS_SEPARATOR);
        for (let i = 0; i < encodings.length; i++) {
          if ("gzip" === encodings[i].toLowerCase() || encodings[i].toLowerCase() === "deflate") {
            return true;
          }
        }
      }
    }
    return false
  }

  private mExecutorService: HippyExecutor | null = null

  protected execute(runnable: HippyTask) {
    if (this.mExecutorService == null) {
      this.mExecutorService = new HippyExecutor()
    }
    this.mExecutorService.execute(runnable);
  }

  protected createResponse(urlConnection: connection.NetConnection): HippyHttpResponse {
    let response = new HippyHttpResponse();
    // response.setStatusCode(urlConnection.getResponseCode());
    // response.setRspHeaderMap(urlConnection.getHeaderFields());
    let isException = false;
    let inputStream = null;
    let errorStream = null;
    // try {
    //   inputStream = urlConnection.getInputStream();
    // } catch (e) {
    //   e.printStackTrace();
    //   isException = true;
    // }
    // if (isException || urlConnection.getResponseCode() >= 400) {
    //   errorStream = urlConnection.getErrorStream();
    // }
    // response.setInputStream(inputStream);
    // response.setErrorStream(errorStream);
    // try {
    //   response.setResponseMessage(urlConnection.getResponseMessage());
    // } catch ( e) {
    //   e.printStackTrace();
    // }
    return response;
  }

  protected createConnection(request: HippyHttpRequest): connection.NetConnection {
    let requrl = this.toURL(request.getUrl());
    let netConnection = connection.createNetConnection()
    let method = request.getMethod();
    // connection.setRequestMethod(method);
    // connection.setUseCaches(request.isUseCaches());
    // connection.setInstanceFollowRedirects(request.isInstanceFollowRedirects());
    // connection.setConnectTimeout(request.getConnectTimeout());
    // connection.setReadTimeout(request.getReadTimeout());
    if (method != undefined && (method.toUpperCase() == "POST"
      || method.toUpperCase() == "PUT"
      || method.toUpperCase() == "PATCH")) {
      // netConnection.setDoOutput(true);
    }
    return netConnection
  }

  protected fillHeader(urlConnection: connection.NetConnection, request: HippyHttpRequest) {
    let headerMap = request.getHeaders();
    if (headerMap.isEmpty()) {
      return;
    }
    headerMap.forEach((value, key) => {
      if (key == null || TextUtils.isEmpty(value)) {
        // continue;
      }
      // urlConnection.setRequestProperty(key, value);
    })
  }

  protected fillPostBody(connection: connection.HttpRequest, request: HippyHttpRequest) {
    if (TextUtils.isEmpty(request.getBody())) {
      return;
    }
    // connection.setRequestProperty("Content-Length", request.getBody().getBytes().length + "");
    // out: DataOutputStream = new DataOutputStream(connection.getOutputStream());
    // //TODO big stream will cause OOM; Progress callback is meaningless
    // out.write(request.getBody().getBytes());
    // out.flush();
    // out.close();
  }

  public destroyIfNeed() {
    if (this.mExecutorService != null && !this.mExecutorService.isShutdown()) {
      this.mExecutorService.shutdown();
      this.mExecutorService = null;
    }
  }

  private handleRedirectRequest(request: HippyHttpRequest,
                                callback: HttpTaskCallback, connection: connection.NetConnection) {
    //获取重定向地址
    // let location = connection.getHeaderField("Location");
    // if (TextUtils.isEmpty(location)) {
    //   callback.onTaskFailed(request, new HippyException(
    //     "Redirect location field is empty!"));
    // } else if (request.getAndIncrementRedirectTimes() > 2) {
    //   callback.onTaskFailed(request, new HippyException(
    //     "Redirect more than 3 times!"));
    // } else {
    //   request.setUrl(location);
    //   this.sendRequest(request, callback);
    // }
  }

  public sendRequest(request: HippyHttpRequest, callback: HttpTaskCallback) {
    let httpRequest = http.createHttp();
    httpRequest.request(request.getUrl(),
      {
        method: http.RequestMethod.GET,
        header: request.getHeaders(),
        readTimeout: request.getReadTimeout(),
        connectTimeout: request.getConnectTimeout()
      }, (err, data) => {
        if (!err) {
          LogUtils.i(TAG,'Result:' + data.result);
          LogUtils.i(TAG,'code:' + data.responseCode);
          LogUtils.i(TAG,'header:' + JSON.stringify(data.header));
          LogUtils.i(TAG,'cookies:' + data.cookies); // 8+
          LogUtils.i(TAG,'header.Content-Type:' + data.header['Content-Type']);
          LogUtils.i(TAG,'header.Status-Line:' + data.header['Status-Line']);
        } else {
          LogUtils.i(TAG,'error:' + JSON.stringify(err));
        }
      });
    // this.execute(new )
  }

  public fetch(holder: ResourceDataHolder, nativeParams: HashMap<string, Object>, callback: ProcessorCallback) {
    holder.processorTag = DefaultHttpAdapter.name
    let httpRequest = this.generateHttpRequest(holder, nativeParams);
    this.handleRequestCookie(httpRequest);
    this.sendRequest(httpRequest, new HttpTaskCallbackImpl(holder, callback))
  }

  public async handleRequestCookie(httpRequest: HippyHttpRequest) {
    let url = httpRequest.getUrl();
    if (url != null) {
      let cookies = httpRequest.getRequestCookies();
      if (cookies == undefined) {
        return
      }
      this.saveCookieToManager(url, cookies, "");
      if (this.getCookieManager()) {
        let cookie = web_webview.WebCookieManager.fetchCookie(url);
        if (!TextUtils.isEmpty(await cookie)) {
          httpRequest.addHeader(HttpHeaderReuqest.COOKIE, await cookie);
        }
      }
    }
  }

  protected saveCookie2Manager(url: string, cookies: List<String>) {
    for (let i = 0; i < cookies.length; i++) {
      let cookie: String = cookies.get(i);
      this.saveCookieToManager(url, cookie, "");
    }
  }

  protected saveCookieToManager(url: string, cookies: String, expires: string) {
    if (this.getCookieManager() || cookies == null) {
      return;
    }
    cookies = cookies.replace("\\s+", "");
    let cookieItems: string[] = cookies.split(";");
    cookieItems.forEach((cookie) => {
      if (cookie != null && cookie.trim().length > 0) {
        let newCookie: string = this.resetCookieIfNeeded(cookie, expires);
        web_webview.WebCookieManager.configCookie(url, newCookie)
      }
    })
  }

  protected getCookieManager(): web_webview.WebCookieManager {
    let cookieManager: web_webview.WebCookieManager | undefined
    if (cookieManager == undefined) {
      cookieManager = new web_webview.WebCookieManager()
    }
    try {
      if (!web_webview.WebCookieManager.isCookieAllowed()) {
        web_webview.WebCookieManager.putAcceptCookieEnabled(true);
      }
      return web_webview.WebCookieManager
    } catch (err) {
      LogUtils.e(TAG, "getCookieManager: " + err.getMessage());
      // https://bugs.chromium.org/p/chromium/issues/detail?id=559720
      return false
    }
  }

  public getCookie(url: string): Promise<void> {
    return new Promise((resolve, reject) => {
      try {
        web_webview.WebCookieManager.fetchCookie(url)
          .then((res) => {
            console.log("saveCookieAsyncCallback success!");
            return res
          })
          .then(() => resolve())
          .catch((err: BusinessError) => {
            console.error("error: " + err.message);
            reject(err)
          })
      } catch (error) {
        console.error(`ErrorCode: ${error.code},  Message: ${error.message}`);
      }
    });
  }

  public setCookie(url: string, keyValue: string, expires: string) {
    if (!TextUtils.isEmpty(url) && keyValue != null) {
      if (keyValue.trim().length == 0) {
        this.clearCookie(url);
      } else {
        this.saveCookieToManager(url, keyValue, expires);
      }
    }
  }

  protected generateHttpRequest(holder: ResourceDataHolder, nativeParams: HashMap<string, Object>): HippyHttpRequest {
    if (holder.responseHeaders == null || holder.requestHeaders == undefined)
      holder.requestHeaders = new HashMap<string, string>()
    if (holder.requestParams == null || holder.requestParams == undefined)
      holder.requestParams = new HashMap<string, string>()
    let httpRequest: HippyHttpRequest = new HippyHttpRequest(holder.requestHeaders,
      holder.requestParams, nativeParams);
    httpRequest.setUrl(holder.uri);
    httpRequest.setConnectTimeout(10 * 1000);
    httpRequest.setReadTimeout(16 * 1000);
    httpRequest.setUseCaches(false);
    return httpRequest;
  }

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

  protected clearCookie(url: string) {
    if (!web_webview.WebCookieManager.existCookie()) {
      return;
    }
    let cookies: string = web_webview.WebCookieManager.fetchCookieSync(url);
    if (TextUtils.isEmpty(cookies)) {
      return;
    }
    let cookieItems: String[] = cookies.split(";");
    cookieItems.forEach((cookie) => {
      web_webview.WebCookieManager.setCookie(url, (cookie + ";Max-Age=0"))
    })
  }

  private toURL(url: string | null | undefined): Url.URL { //url验证和完善
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

