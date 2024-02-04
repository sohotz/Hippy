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
import { HippyAny, HippyRecord, HippyRenderCallback } from '../../../support/common/HippyTypes';
import { HRComponent } from '../../HippyRenderView';
import HippyRenderBaseView, { HippyObservedArray } from '../base/HippyRenderBaseView';
import { HRScrollContentView } from './HRScrollContentView';

@Observed
export class HRScrollView extends HippyRenderBaseView {
  axis: Axis = Axis.Vertical
  scrollBar: BarState = BarState.Auto
  edgeEffect: EdgeEffect = EdgeEffect.Spring
  scrollEvent: HippyRenderCallback | null = null
  scrollEndEvent: HippyRenderCallback | null = null
  dragBeginEvent: HippyRenderCallback | null = null
  dragEndEvent: HippyRenderCallback | null = null
  friction: number = 0.6 // default value is 0.6 in ohos
  private readonly DIRECTION_ROW: string = "directionRow"
  private readonly SHOW_SCROLLER_INDICATOR: string = "showScrollerIndicator"
  private readonly VERTICAL_BOUNCES: string = "verticalbounces"
  private readonly HORIZONTAL_BOUNCES: string = "horizontalbounces"
  private readonly BOUNCES_ENABLE: string = "bouncesEnable"
  private readonly SCROLL: string = "scroll"
  private readonly SCROLL_END: string = "scrollEnd"
  private readonly DRAG_BEGIN: string = "dragBegin"
  private readonly DRAG_END: string = "dragEnd"
  private readonly FLING_ENABLE: string = "flingEnable"
  private readonly METHOD_CONTENT_OFFSET: string = "contentOffset"
  private readonly METHOD_CONTENT_INSET_WHEN_END_DRAG: string = "contentInsetWhenEndDrag"
  private readonly METHOD_CONTENT_INSET: string = "contentInset"
  private readonly BLANK_SEPARATOR: string = " "
  scroller: Scroller = new Scroller()

  setProp(propKey: string, propValue: HippyAny | HippyRenderCallback): boolean {
    switch (propKey) {
      case this.DIRECTION_ROW:
        this.axis = propValue as number == 1 ? Axis.Horizontal : Axis.Vertical
        return true
      case this.SHOW_SCROLLER_INDICATOR:
        this.scrollBar = propValue as number == 1 ? BarState.On : BarState.Off
        return true
      case this.VERTICAL_BOUNCES:
      case this.HORIZONTAL_BOUNCES:
      case this.BOUNCES_ENABLE:
        this.edgeEffect = propValue as number == 1 ? EdgeEffect.Spring : EdgeEffect.None
        return true;
      case this.FLING_ENABLE:
        this.friction = (propValue as number == 1) ? 20 : 0.6 // no params in ohos
      case this.SCROLL:
        this.scrollEvent = propValue as HippyRenderCallback
        return true
      case this.SCROLL_END:
        this.scrollEndEvent = propValue as HippyRenderCallback
        return true
      case this.DRAG_BEGIN:
        this.dragBeginEvent = propValue as HippyRenderCallback
        return true
      case this.DRAG_END:
        this.dragEndEvent = propValue as HippyRenderCallback
        return true
    }
    return super.setProp(propKey, propValue)
  }

  call(method: string, params: HippyAny, callback: HippyRenderCallback | null): void {
    switch (method) {
      case this.METHOD_CONTENT_OFFSET:
        this.setContentOffset(params)
        break
      case this.METHOD_CONTENT_INSET_WHEN_END_DRAG:
        break
      case this.METHOD_CONTENT_INSET:
        break
      default:
        super.call(method, params, callback)
    }
  }

  private setContentOffset(params: HippyAny) {
    let arr: Array<string> = (params as string).split(this.BLANK_SEPARATOR)
    let xOffset: number = Number(arr[0])
    let yOffset: number = Number(arr[1])
    let animation: boolean = Number(arr[2]) == 1

    if (this.canScrollImmediately(xOffset, yOffset)) {
      this.scroller.scrollTo({ xOffset: xOffset, yOffset: yOffset, animation: animation })
    }
  }

  private canScrollImmediately(xOffset: number, yOffset: number): boolean {
    if (this.axis == Axis.Vertical) {
      return (this.children[0] as HRScrollContentView)?.viewHeight > yOffset
    } else {
      return (this.children[0] as HRScrollContentView)?.viewWidth > xOffset
    }
  }
}

@Component
export struct HRScroll {
  @ObjectLink renderView: HRScrollView
  @ObjectLink children: HippyObservedArray<HippyRenderBaseView>
  @BuilderParam buildRenderView: ($$: HippyRenderBaseView, parent: HRComponent) => void
  @State xOffset: number = 0
  @State yOffset: number = 0
  viewWidth: number = 0
  viewHeight: number = 0

  build() {
    List({ scroller: this.renderView.scroller }) {
      if (this.children.length > 0) {
        this.buildRenderView(this.children[0], this)
      }
    }
    .applyRenderViewBaseAttr(this.renderView)
    .edgeEffect(this.renderView.edgeEffect)
    .listDirection(this.renderView.axis)
    .friction(this.renderView.friction)
    .onScroll((xOffset: number, yOffset: number) => {
      this.xOffset = xOffset
      this.yOffset = yOffset
      if (this.renderView.scrollEvent) {
        this.renderView.scrollEvent({
          "offsetX": this.renderView.scroller.currentOffset().xOffset,
          "offsetY": this.renderView.scroller.currentOffset().yOffset,
          "contentWidth": (this.renderView.children[0] as HRScrollContentView).viewWidth,
          "contentHeight": (this.renderView.children[0] as HRScrollContentView).viewHeight,
          "viewWidth": this.viewWidth,
          "viewHeight": this.viewHeight,
          "isDragging": false
        } as HippyRecord);
      }
    })
    .onScrollStop(() => {
      if (this.renderView.scrollEndEvent) {
        this.renderView.scrollEndEvent({
          "offsetX": this.renderView.scroller.currentOffset().xOffset,
          "offsetY": this.renderView.scroller.currentOffset().yOffset,
          "contentWidth": (this.renderView.children[0] as HRScrollContentView).viewWidth,
          "contentHeight": (this.renderView.children[0] as HRScrollContentView).viewHeight,
          "viewWidth": this.viewWidth,
          "viewHeight": this.viewHeight,
          "isDragging": false
        } as HippyRecord);
      }
    })
    .onDragStart(() => {
      if (this.renderView.dragBeginEvent) {
        this.renderView.dragBeginEvent({
          "offsetX": this.renderView.scroller.currentOffset().xOffset,
          "offsetY": this.renderView.scroller.currentOffset().yOffset,
          "contentWidth": (this.renderView.children[0] as HRScrollContentView).viewWidth,
          "contentHeight": (this.renderView.children[0] as HRScrollContentView).viewHeight,
          "viewWidth": this.viewWidth,
          "viewHeight": this.viewHeight,
          "isDragging": true
        } as HippyRecord);
      }
    })
    .onDragEnd(() => {
      if (this.renderView.dragEndEvent) {
        this.renderView.dragEndEvent({
          "offsetX": this.renderView.scroller.currentOffset().xOffset,
          "offsetY": this.renderView.scroller.currentOffset().yOffset,
          "contentWidth": (this.renderView.children[0] as HRScrollContentView).viewWidth,
          "contentHeight": (this.renderView.children[0] as HRScrollContentView).viewHeight,
          "viewWidth": this.viewWidth,
          "viewHeight": this.viewHeight,
          "isDragging": true
        } as HippyRecord);
      }
    })
    .onAreaChange((oldArea: Area, newArea: Area) => {
      this.viewWidth = newArea.width as number
      this.viewHeight = newArea.height as number
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
  .touchable($$.cssTouchEnable) // TODO: use new api
  .zIndex($$.cssZIndex)
  .border($$.cssBorder)
  .rotate($$.cssRotate)
  .scale($$.cssScale)
  .translate($$.cssTranslate)
  .scrollBar(($$ as HRScrollView).scrollBar)
}
