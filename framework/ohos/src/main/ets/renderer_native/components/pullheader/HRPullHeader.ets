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
import { HRRect } from '../../utils/HRTypes';
import HippyRenderBaseView, {
  HippyObservedArray } from '../base/HippyRenderBaseView';
import { LogUtils } from '../../../support/utils/LogUtils';
import { HRValueUtils } from '../../utils/HRValueUtils';

@Observed
export class HRPullHeaderView extends HippyRenderBaseView {
  private readonly TAG = "HRPullHeaderView"
  private CURRENT_OFFSET: string = "contentOffset";

  constructor(ctx: NativeRenderContext) {
    super(ctx)
    this.cssVisibility = Visibility.Hidden
  }

  setProp(propKey: string, propValue: HippyAny | HippyRenderCallback): boolean {
    // TODO: xxxxx
    LogUtils.d(this.TAG, "setProp: propKey " + propKey + ", propValue: " + propValue)
    switch (propKey) {
      case "cssPositionX":
        this.cssPositionX = HRValueUtils.getNumber(propValue as HippyAny);
        break;
      case "cssPositionY":
        this.cssPositionY = HRValueUtils.getNumber(propValue as HippyAny);
        break;
      default:
        break;
    }
    let ret : boolean = super.setProp(propKey, propValue as HippyAny)
    if (propKey === "visibility") {
      LogUtils.d(this.TAG, "setProp: visibility " + this.cssVisibility)
    }
    return ret;
  }

  call(method: string, params: Array<HippyAny>, callback: HippyRenderCallback | null): void {
    LogUtils.d(this.TAG, "call: method " + method + ", params: " + params)
    switch (method) {
      case "collapsePullHeader":
        this.onHeadRefreshFinish()
        break
      case "collapsePullHeaderWithOptions":
        {
          let map = params[0] as HippyMap
          let collapseTime = map.get('time') as number;
          setTimeout(() => {
            LogUtils.d(this.TAG, "delay : " + collapseTime)
            // let params = new Map<string, Number>() as HippyMap;
            // params.set(this.CURRENT_OFFSET, this.cssHeight);
            //HREventUtils.sendComponentEvent(this.ctx, this.tag, HREventUtils.EVENT_LIST_HEADER_PULLING, params);
            this.onHeadRefreshFinish()
          }, collapseTime);
        }
        break;
      default:
        break;
    }
  }

  onHeadRefreshFinish() {
    this.cssVisibility = Visibility.None
    if (this.parentView) {
      this.parentView.setProp("cssPositionX", 0);
      this.parentView.setProp("cssPositionY", 0);
    }
  }

  setRenderViewFrame(frame: HRRect): void {
    frame.x = this.cssPositionX;
    frame.y = this.cssPositionY;
    super.setRenderViewFrame(frame)
  }
}

@Component
export struct HRPullHeader {
  private readonly TAG = "HRPullHeader"
  @ObjectLink renderView: HRPullHeaderView
  @ObjectLink children: HippyObservedArray<HippyRenderBaseView>
  @BuilderParam buildRenderView: ($$: HippyRenderBaseView, parent: HRComponent) => void
  @BuilderParam buildCustomRenderView: ($$: HippyRenderBaseView) => void | null
  private controller: SwiperController = new SwiperController()
  private PAGE_ITEM_POSITION: string = "position";
  @State index: number = 0

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
    .zIndex(2)
    // .visibility(this.renderView.cssVisibility)
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
  // .zIndex($$.cssZIndex)
  .border($$.cssBorder)
  .rotate($$.cssRotate)
  .scale($$.cssScale)
  .translate($$.cssTranslate)
  .onClick($$.eventClick)
}
