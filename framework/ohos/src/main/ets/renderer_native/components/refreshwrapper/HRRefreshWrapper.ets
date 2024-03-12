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
import { ComposeListItem } from '@ohos.arkui.advanced.ComposeListItem';
import { HRValueUtils } from '../../utils/HRValueUtils';

@Observed
export class HRRefreshWrapperView extends HippyRenderBaseView {
  private readonly TAG = "HRRefreshWrapperView"
  public bounceTime:number = 300
  public scroller: Scroller = new Scroller()
  public needRefresh:boolean = false
  public refreshBarVisible = false
  public uiCtx:UIContext|null = null

  constructor(ctx: NativeRenderContext) {
    super(ctx)
  }

  setProp(propKey: string, propValue: HippyAny | HippyRenderCallback): boolean {
    LogUtils.d(this.TAG, "setProp: propKey " + propKey + ", propValue: " + propValue)
    if (propKey === 'bounceTime') {
      this.bounceTime = HRValueUtils.getNumber(propValue as HippyAny)
    } else if (propKey === 'cssPositionY') {
      this.cssPositionY = HRValueUtils.getNumber(propValue as HippyAny)
    }
    return super.setProp(propKey, propValue as HippyAny)
  }

  call(method: string, params: Array<HippyAny>, callback: HippyRenderCallback | null): void {
    LogUtils.d(this.TAG, "call: method " + method + ", params: " + params)
    if (method === 'refreshComplected') {
      this.refreshComplected()
    }
  }

  bounceToHead() {
    setTimeout(() => {
      LogUtils.d(this.TAG, `timeout:${this.bounceTime}`)
      this.scroller.scrollToIndex(1, true)
      this.refreshBarVisible = false
    },
      this.bounceTime)
  }

  startRefresh() {
    HREventUtils.sendComponentEvent(this.ctx, this.tag, HREventUtils.EVENT_REFRESH_WRAPPER_REFRESH, null);
  }

  refreshComplected() {
    this.uiCtx?.runScopedTask(()=>{
      this.bounceToHead()
    })

    this.needRefresh = false
  }
}

@Component
export struct HRRefreshWrapper {
  private readonly TAG = "HRRefreshWrapper"
  @ObjectLink renderView: HRRefreshWrapperView
  @ObjectLink children: HippyObservedArray<HippyRenderBaseView>
  @BuilderParam buildRenderView: ($$: HippyRenderBaseView, parent: HRComponent) => void
  @BuilderParam buildCustomRenderView: ($$: HippyRenderBaseView) => void | null

  aboutToAppear() {
    this.renderView.uiCtx = this.getUIContext()
  }

  aboutToDisappear() {
  }

  build() {
    List({ initialIndex: 1, scroller: this.renderView.scroller }) {
      ForEach(this.children, (item: HippyRenderBaseView) => {
        ListItem() {
          this.buildRenderView(item, null)
        }
      }, (item: HippyRenderBaseView) => item.tag + '')
    }
    .applyRenderViewBaseAttr(this.renderView)
    .scrollBar(BarState.Off)
    .onScrollIndex((start)=>{
      LogUtils.d(this.TAG, `onScrollIndex, start:${start}`)
      if (start === 0) {
        this.renderView.refreshBarVisible = true
      }
    })
    .onReachStart(() => {
      LogUtils.d(this.TAG, 'onReachStart')
      this.renderView.needRefresh = true
    })
    .onScrollStop(() => {
      LogUtils.d(this.TAG, `onReachStop, needfresh:${this.renderView.needRefresh}, visible:${this.renderView.refreshBarVisible}`)
      if (this.renderView.needRefresh) {
        this.renderView.startRefresh()
      } else if (this.renderView.refreshBarVisible){
        this.renderView.bounceToHead()
      }
    })
  }
}

// base props for all components
@Extend(List)
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
