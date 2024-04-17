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
import { HippyNativeModule } from '../../../../support/annotation/HippyNativeModule'
import { HippyAny, HippyMap } from '../../../../support/common/HippyTypes'
import { HippyEngineContext } from '../../../HippyEngineContext'
import { HippyNativeModuleBase } from '../HippyNativeModuleBase'
import { webSocket } from '@kit.NetworkKit'
import { HashMap } from '@kit.ArkTS'
import { TextUtils } from '../../../../support/utils/TextUtils'
import { HippyJsException } from '../../../common/HippyJsException'
import { LogUtils } from '../../../../support/utils/LogUtils'
import { HippyModulePromise } from '../../HippyModulePromise'

const WEBSOCKET_MODULE_TAG = "WebSocketModule";

@HippyNativeModule({ name: "websocket" })
export class WebSocketModule extends HippyNativeModuleBase {
  public static readonly NAME = 'websocket'
  private static PARAM_KEY_SOCKET_URL = "url";
  private static PARAM_KEY_SOCKET_ID = "id";
  private static PARAM_KEY_CODE = "code";
  private static PARAM_KEY_REASON = "reason";
  private static PARAM_KEY_HEADERS = "headers";
  private static PARAM_KEY_TYPE = "type";
  private static PARAM_KEY_DATA = "data";
  private mWebSocketConnections: HashMap<Number, webSocket.WebSocket>
  private sWebSocketIds = 0

  constructor(ctx: HippyEngineContext) {
    super(ctx)
    this.mWebSocketConnections = new HashMap<number, webSocket.WebSocket>();
  }

  public destroy() {
    let size = this.mWebSocketConnections.length;
    if (size == 0) {
      return;
    }

    for (let key of this.mWebSocketConnections.keys()) {
      let value = this.mWebSocketConnections.get(key)
      if (value != null) {
        value.close();
      }
    }

    this.mWebSocketConnections.clear();
  }

  public call(method: string, params: Array<HippyAny>, callback: HippyModulePromise): HippyAny {
    switch (method) {
      case 'connect': {
        this.connect(params[0] as HippyMap, callback)
        break;
      }
      case 'send': {
        this.send(params[0] as HippyMap)
        break;
      }
      case 'close': {
        this.close(params[0] as HippyMap)
        break;
      }
      default:
        super.call(method, params, callback)
    }
    return null;
  }

  protected removeSocketConnection(socketId: number) {
    this.mWebSocketConnections.remove(socketId);
  }

  private connect(request: HippyMap, promise: HippyModulePromise) {
    let returnValue: HippyMap = new Map();
    if (request == null) {
      returnValue.set(WebSocketModule.PARAM_KEY_CODE, -1);
      returnValue.set(WebSocketModule.PARAM_KEY_REASON, "invalid connect param");
      promise.resolve(returnValue);
      return;
    }

    let url = request.get(WebSocketModule.PARAM_KEY_SOCKET_URL) + "";
    if (TextUtils.isEmpty(url)) {
      returnValue.set(WebSocketModule.PARAM_KEY_CODE, -1);
      returnValue.set(WebSocketModule.PARAM_KEY_REASON, "no valid url for websocket");
      promise.resolve(returnValue);
      return;
    }

    let webSocketId = this.addAndGet(this.sWebSocketIds);
    let ws = webSocket.createWebSocket();
    ws.connect(url).then((value) => {
      LogUtils.i(WEBSOCKET_MODULE_TAG, "connect success")
      this.mWebSocketConnections.set(webSocketId, ws);

      returnValue.set(WebSocketModule.PARAM_KEY_CODE, 0);
      returnValue.set(WebSocketModule.PARAM_KEY_REASON, "");
      returnValue.set(WebSocketModule.PARAM_KEY_SOCKET_ID, webSocketId);
      promise.resolve(returnValue);
    }).catch((err: HippyJsException) => {
      LogUtils.e(WEBSOCKET_MODULE_TAG, "connect fail, error:" + JSON.stringify(err))
    });
  }

  private send(param: HippyMap) {
    this.handleSocket(param, "open")
  }

  private close(param: HippyMap) {
    this.handleSocket(param, "close")
  }

  private handleSocket(param: HippyMap, status: string) {
    if (param == null) {
      LogUtils.d(WEBSOCKET_MODULE_TAG, "send: ERROR: request is null");
      return;
    }

    if (!param.has(WebSocketModule.PARAM_KEY_SOCKET_ID)) {
      LogUtils.d(WEBSOCKET_MODULE_TAG, "send: ERROR: no socket id specified");
      return;
    }
    let socketId = param.get(WebSocketModule.PARAM_KEY_SOCKET_ID);
    if (socketId instanceof Number) {
      let socketClient = this.mWebSocketConnections.get(socketId);
      if (socketClient == null) {
        LogUtils.d(WEBSOCKET_MODULE_TAG, "send: ERROR: specified socket not found, or not connected yet");
        return;
      }

      let textData = param.get(WebSocketModule.PARAM_KEY_DATA);
      if (textData == null) {
        LogUtils.d(WEBSOCKET_MODULE_TAG, "send: ERROR: no data specified to be sent");
        return;
      }
      if (status == "open") {
        // 当收到on('open')事件时，可以通过send()方法与服务器进行通信
        // todo textData数据类型支持string|ArrayBuffer
        socketClient.send(textData.toString(), (err, value) => {
          if (!err) {
            LogUtils.d(WEBSOCKET_MODULE_TAG, "send success");
          } else {
            LogUtils.d(WEBSOCKET_MODULE_TAG, "send fail, err:" + JSON.stringify(err));
          }
        });
      } else if (status == "close") {
        // 当收到服务器的`bye`消息时（此消息字段仅为示意，具体字段需要与服务器协商），主动断开连接
        //if (value === 'bye') {
        socketClient.close((err, value) => {
          if (!err) {
            LogUtils.d(WEBSOCKET_MODULE_TAG, "close success");
          } else {
            LogUtils.d(WEBSOCKET_MODULE_TAG, "close fail, err is " + JSON.stringify(err));
          }
        });
      }
    }
  }

  private addAndGet(num: number): number {
    return num + 1
  }
}
