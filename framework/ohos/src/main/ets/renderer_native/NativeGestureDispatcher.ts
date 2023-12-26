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

import HashMap from '@ohos.util.HashMap'
import { NodeProps } from './dom_node/NodeProps'
import { NativeRenderContext } from './NativeRenderContext'
import { EventUtils } from './utils/EventUtils'

export class NativeGestureDispatcher {
  static readonly TAG = "NativeGestureDispatcher"
  static readonly KEY_PAGE_X = "page_x"
  static readonly KEY_PAGE_Y = "page_y"

  static handleAttachedToWindow(ctx: NativeRenderContext, nodeId: number) {
    EventUtils.sendComponentEvent(ctx, nodeId, NodeProps.ON_ATTACHED_TO_WINDOW, null)
  }

  static handleDetachedFromWindow(ctx: NativeRenderContext, nodeId: number) {
    EventUtils.sendComponentEvent(ctx, nodeId, NodeProps.ON_DETACHED_FROM_WINDOW, null)
  }

  static handleClickEvent(ctx: NativeRenderContext, nodeId: number, eventName: string) {
    EventUtils.sendGestureEvent(ctx, nodeId, eventName, null)
  }

  static handleTouchEvent(ctx: NativeRenderContext, nodeId: number, windowX: number, windowY: number, eventName: string) {
    let params = new HashMap<string, any>()
    params.set(NativeGestureDispatcher.KEY_PAGE_X, windowX)
    params.set(NativeGestureDispatcher.KEY_PAGE_Y, windowY)
    EventUtils.sendGestureEvent(ctx, nodeId, eventName, params)
  }
}
