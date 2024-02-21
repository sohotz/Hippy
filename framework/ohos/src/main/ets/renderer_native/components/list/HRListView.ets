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
import { HippyAny, HippyRenderCallback } from '../../../support/common/HippyTypes';
import { HRComponent } from '../../HippyRenderView';
import { NativeRenderContext } from '../../NativeRenderContext';
import HippyRenderBaseView, {
  HippyObservedArray } from '../base/HippyRenderBaseView';

@Observed
export class HRListView extends HippyRenderBaseView {
  constructor(ctx: NativeRenderContext) {
    super(ctx)
  }

  setProp(propKey: string, propValue: HippyAny | HippyRenderCallback): boolean {
    return super.setProp(propKey, propValue as HippyAny)
  }
}

@Component
export struct HRList {
  @ObjectLink renderView: HRListView
  @ObjectLink children: HippyObservedArray<HippyRenderBaseView>
  @BuilderParam buildRenderView:($$: HippyRenderBaseView, parent: HRComponent) => void

  build() {
    List({ space: 20, initialIndex: 0 }) {
      ForEach(this.children, (item: HippyRenderBaseView) => {
        ListItem() {
          this.buildRenderView(item, null)
        }
      }, (item: HippyRenderBaseView) => item.tag + '')
    }
    .applyRenderViewBaseAttr(this.renderView)
    .listDirection(Axis.Vertical)
    .scrollBar(BarState.Off)
    .friction(0.6)
    .divider({ strokeWidth: 2, color: 0xFFFFFF, startMargin: 0, endMargin: 0 })
    .edgeEffect(EdgeEffect.Spring)
    .onScrollIndex((firstIndex: number, lastIndex: number, centerIndex: number) => {

    })
    .onScroll((scrollOffset: number, scrollState: ScrollState) => {

    })
  }
}

// base props for all components
@Extend(List)
function applyRenderViewBaseAttr($$: HippyRenderBaseView) {
  .backgroundColor($$.cssBackgroundColor)
  .position({x: $$.cssPositionX, y: $$.cssPositionY})
  .size({width:$$.cssWidth, height: $$.cssHeight})
  .opacity($$.cssOpacity)
  .clip($$.cssOverflow)
  .visibility(($$ as HippyRenderBaseView).cssVisibility) // must add as, otherwise the compiler has error
  .touchable($$.cssTouchEnable) // TODO: use new api
  .zIndex($$.cssZIndex)
  .border($$.cssBorder)
  .rotate($$.cssRotate)
  .scale($$.cssScale)
  .translate($$.cssTranslate)
  .onClick($$.eventClick)
}
