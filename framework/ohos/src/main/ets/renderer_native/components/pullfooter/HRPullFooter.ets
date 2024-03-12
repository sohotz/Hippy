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
import HippyRenderBaseView, {
  HippyObservedArray } from '../base/HippyRenderBaseView';
import { LogUtils } from '../../../support/utils/LogUtils';
import { HRRect } from '../../utils/HRTypes';
import { HRValueUtils } from '../../utils/HRValueUtils';

@Observed
export class HRPullFooterView extends HippyRenderBaseView {
  private readonly TAG = "HRPullFooterView"
  //public collapseTime: number = 1000
  constructor(ctx: NativeRenderContext) {
    super(ctx)
    this.cssVisibility = Visibility.None
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
    return super.setProp(propKey, propValue as HippyAny)
  }

  call(method: string, params: Array<HippyAny>, callback: HippyRenderCallback | null): void {
    LogUtils.d(this.TAG, "call: method " + method + ", params: " + params)
    switch (method) {
      case "collapsePullFooter":
        {
          //setTimeout(() => {
          //  LogUtils.d(this.TAG, "delay : " + this.collapseTime)
            this.cssVisibility = Visibility.None
            if (this.parentView) {
              this.parentView.setProp("cssPositionX", 0);
              this.parentView.setProp("cssPositionY", 0);
              this.parentView.setProp("cssHeight", this.parentView.cssHeight+this.cssHeight)
            }
          //}, this.collapseTime);
        }
        break;
      default:
        break;
    }
  }

  setRenderViewFrame(frame: HRRect): void {
    frame.x = this.cssPositionX;
    frame.y = this.cssPositionY;
    super.setRenderViewFrame(frame)
  }
}

@Component
export struct HRPullFooter {
  private readonly TAG = "HRPullFooter"
  @ObjectLink renderView: HRPullFooterView
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

  setPage(index: number) {
    this.index = index
  }

  build() {
    Stack() {
      ForEach(this.children, (item: HippyRenderBaseView) => {
        this.buildRenderView(item, null)
      }, (item: HippyRenderBaseView) => item.tag + '')
    }
    .applyRenderViewBaseAttr(this.renderView)
    //.visibility(this.renderView.cssVisibility)
    // .zIndex(2)
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
