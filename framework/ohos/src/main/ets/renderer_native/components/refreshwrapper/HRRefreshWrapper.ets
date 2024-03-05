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

import { HippyAny, HippyMap, HippyRenderCallback } from '../../../support/common/HippyTypes';
import { HRComponent } from '../../HippyRenderView';
import { NativeRenderContext } from '../../NativeRenderContext';
import { HREventUtils } from '../../utils/HREventUtils'
import HippyRenderBaseView, { HippyObservedArray } from '../base/HippyRenderBaseView';
import { LogUtils } from '../../../support/utils/LogUtils';

@Observed
export class HRRefreshWrapperView extends HippyRenderBaseView {
  private readonly TAG = "HRRefreshWrapperView"
  public bounceTime:number = 300

  constructor(ctx: NativeRenderContext) {
    super(ctx)
  }

  setProp(propKey: string, propValue: HippyAny | HippyRenderCallback): boolean {
    LogUtils.d(this.TAG, "setProp: propKey " + propKey + ", propValue: " + propValue)
    if (propKey === 'bounceTime') {
      this.bounceTime = propValue as number
    } else if (propKey === 'cssPositionY') {
      this.cssPositionY = propValue as number
    }
    return super.setProp(propKey, propValue as HippyAny)
  }

  call(method: string, params: Array<HippyAny>, callback: HippyRenderCallback | null): void {
    LogUtils.d(this.TAG, "call: method " + method + ", params: " + params)
    if (method === 'refreshComplected') {
      this.refreshComplected()
    }
  }

  bounceToHead(posY:number) {
    setTimeout(() => {
      LogUtils.d(this.TAG, `timeout:${this.bounceTime}, PosY:${posY}`)
      this.children[0].setProp('cssPositionY', -posY)
      this.setProp('cssPositionY', posY)
    },
      this.bounceTime)
  }

  startRefresh() {
    this.bounceToHead(this.children[0].cssHeight)
    HREventUtils.sendComponentEvent(this.ctx, this.tag, HREventUtils.EVENT_REFRESH_WRAPPER_REFRESH, null);
  }

  refreshComplected() {
    this.bounceToHead(0)
  }
}

@Component
export struct HRRefreshWrapper {
  private readonly TAG = "HRRefreshWrapper"
  @ObjectLink renderView: HRRefreshWrapperView
  @ObjectLink children: HippyObservedArray<HippyRenderBaseView>
  @BuilderParam buildRenderView: ($$: HippyRenderBaseView, parent: HRComponent) => void
  private controller: SwiperController = new SwiperController()
  private PAGE_ITEM_POSITION: string = "position";
  initY: number = 0

  aboutToAppear() {
  }

  aboutToDisappear() {
  }

  build() {
    Stack() {
      ForEach(this.children, (item: HippyRenderBaseView) => {
        this.buildRenderView(item, null)
      }, (item: HippyRenderBaseView) => item.tag + '')
    }
    .applyRenderViewBaseAttr(this.renderView)
    .onTouch((event: TouchEvent) => {
      let delta = 0
      delta = event.touches[0].windowY - this.initY
      //LogUtils.d(this.TAG, `type:${event.type}, touch:x, ${event.touches[0].x},y, ${event.touches[0].y}, WindowsY, ${event.touches[0].windowY}, DisplayY, ${event.touches[0].displayY}, cssPos:${this.renderView.cssPositionY}`)
      switch (event.type) {
        case TouchType.Down:
          this.initY = event.touches[0].windowY
          break
        case TouchType.Up:
          if (delta > this.children[0].cssHeight) {
            this.renderView.startRefresh()
          } else {
            this.renderView.bounceToHead(0)
          }
          break
        case TouchType.Move:
        case TouchType.Cancel:
          //LogUtils.d(this.TAG, `initY:${this.initY}, WindowsY:${event.touches[0].windowY}, detla:${delta}, delta:${delta}`)
          if (delta > 0) {
            this.renderView.setProp('cssPositionY', delta)

            //this.children[0].cssPositionY = this.itemInitY + delta
            if (delta > this.children[0].cssHeight) {
              delta = this.children[0].cssHeight
            }
            this.children[0].setProp('cssPositionY', -delta)

            //this.children[1].setProp('cssPositionY',  delta)

            //this.renderView.cssPositionY = event.touches[0].windowY
            //this.children[0].cssHeight = this.heightx
          }
          break
      }
    })
  }
}

// base props for all components
@Extend(Stack)
function applyRenderViewBaseAttr($$: HippyRenderBaseView) {
  .backgroundColor($$.cssBackgroundColor)
  .position({ x: $$.cssPositionX, y: $$.cssPositionY })
  .size({ width: $$.cssWidth, height: $$.cssHeight })
  .opacity($$.cssOpacity)
  .clip($$.cssOverflow)
  .visibility(($$ as HippyRenderBaseView).cssVisibility) // must add as, otherwise the compiler has error
  .zIndex($$.cssZIndex)
  .border($$.cssBorder)
  .rotate($$.cssRotate)
  .scale($$.cssScale)
  .translate($$.cssTranslate)
  .onClick($$.eventClick)
}
