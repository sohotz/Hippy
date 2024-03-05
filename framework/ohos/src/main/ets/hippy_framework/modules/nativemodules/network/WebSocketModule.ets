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
import { HippyObservedArray } from '../../../../renderer_native'
import { HippyNativeModule } from '../../../../support/annotation/HippyNativeModule'
import { HippyMap } from '../../../../support/common/HippyTypes'
import { HippyEngineContext } from '../../../HippyEngineContext'
import { HippyNativeModuleBase } from '../HippyNativeModuleBase'
import { webSocket } from '@kit.NetworkKit'
import { HashMap } from '@kit.ArkTS'
import { TextUtils } from '../../../../support/utils/TextUtils'
import { HippyJsException } from '../../../common/HippyJsException'

const WEBSOCKET_MODULE_TAG = "WebSocketModule";

@HippyNativeModule({ name: "WebSocketModule" })
export class WebSocketModule extends HippyNativeModuleBase {
  public static readonly NAME = 'WebSocketModule'
  private mWebSocketConnections: Array<webSocket.WebSocket>
  private static PARAM_KEY_SOCKET_URL = "url";

  constructor(ctx: HippyEngineContext) {
    super(ctx)
    this.mWebSocketConnections = new Array<webSocket.WebSocket>();
  }

  public connect(request: HippyMap): Promise<void> {
    return new Promise((resolve, reject) => {
      if (request == null) {

      }
      let url = request.get(WebSocketModule.PARAM_KEY_SOCKET_URL)+"";
      if (TextUtils.isEmpty(url)) {
        return
      }

      let ws = webSocket.createWebSocket();
      ws.connect(url).then((value) => {
        console.log("connect success")
      }).catch((err:HippyJsException) => {
        console.log("connect fail, error:" + JSON.stringify(err))
      });
    })
  }

  public send(request: HippyMap) {

  }

  public close(request: HippyMap) {

  }

  public destroy() {

  }

  protected removeSocketConnection(socketId: number) {

  }
}
