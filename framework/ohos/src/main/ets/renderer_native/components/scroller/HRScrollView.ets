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
import {
  Hippy2Map,
  HippyAny,
  HippyArray,
  HippyMap,
  HippyRecord,
  HippyRenderCallback
} from '../../../support/common/HippyTypes';
import { HRComponent } from '../../HippyRenderView';
import { NativeRenderContext } from '../../NativeRenderContext';
import HippyRenderBaseView, { HippyObservedArray } from '../base/HippyRenderBaseView';
import { HRScrollContentView } from './HRScrollContentView';
import { LogUtils } from '../../../support/utils/LogUtils';
import { HREventUtils } from '../../utils/HREventUtils';
import systemDateTime from '@ohos.systemDateTime';
import { HRValueUtils } from '../../utils/HRValueUtils';

@Observed
export class HRScrollView extends HippyRenderBaseView {
  private readonly TAG = "HRScrollView"
  private static readonly DEFAULT_FRICTION = 0.8
  axis: ScrollDirection = ScrollDirection.Vertical
  scrollBar: BarState = BarState.Off
  edgeEffect: EdgeEffect = EdgeEffect.Spring
  scrollEvent: HippyRenderCallback | null = null
  scrollEndEvent: HippyRenderCallback | null = null
  dragBeginEvent: HippyRenderCallback | null = null
  dragEndEvent: HippyRenderCallback | null = null
  willDragEndEvent: HippyRenderCallback | null = null
  friction: number = HRScrollView.DEFAULT_FRICTION // default value is 0.6 in ohos
  pageEnabled: boolean = false
  contentInset: number = 0
  contentInsetWhenDrag: number = 0
  contentOffset: number = 0
  enableScrollInteraction: boolean = true
  initialContentOffset: number = 0
  scrollEventThrottle: number = 30
  scrollMinOffset: number = 5
  private readonly DIRECTION_ROW: string = "directionRow"
  private readonly SHOW_SCROLLER_INDICATOR: string = "showScrollIndicator"
  private readonly VERTICAL_BOUNCES: string = "verticalbounces"
  private readonly HORIZONTAL_BOUNCES: string = "horizontalbounces"
  private readonly BOUNCES_ENABLE: string = "bouncesEnable"
  private readonly SCROLL: string = "scroll"
  private readonly SCROLL_END: string = "scrollEnd"
  private readonly DRAG_BEGIN: string = "dragBegin"
  private readonly DRAG_END: string = "dragEnd"
  private readonly WILL_DRAG_END: string = "willDragEnd"
  private readonly FLING_ENABLE: string = "flingEnable"
  private readonly METHOD_CONTENT_OFFSET: string = "contentOffset"
  private readonly METHOD_CONTENT_INSET_WHEN_END_DRAG: string = "contentInsetWhenEndDrag"
  private readonly METHOD_CONTENT_INSET: string = "contentInset"
  private readonly METHOD_SCROLLTO: string = "scrollTo"
  private readonly METHOD_SCROLLTO_OPTIONS: string = "scrollToWithOptions"
  private readonly BLANK_SEPARATOR: string = " "
  private readonly PAGING_ENABLED = "pagingEnabled"
  private readonly CONTENT_OFFSET = "contentOffset4Reuse"
  private readonly SCROLL_ENABLED = "scrollEnabled"
  private readonly HORIZONTAL = "horizontal"
  private readonly INITIAL_CONTENTOFFSET = "initialContentOffset"
  private readonly SCROLL_EVENT_THROTTLE = "scrollEventThrottle"
  private readonly SCROLL_MINOFFSET = "scrollMinOffset"
  scroller: Scroller = new Scroller()

  constructor(ctx: NativeRenderContext) {
    super(ctx)
    LogUtils.d(this.TAG, "constructor -------------------- ")
  }

  setProp(propKey: string, propValue: HippyAny | HippyRenderCallback): boolean {
    LogUtils.d(this.TAG, "setProp: index " + propKey + ", propValue: " + propValue)
    switch (propKey) {
      case this.DIRECTION_ROW:
        this.axis = HRValueUtils.getNumber(propValue as HippyAny) == 1 ? ScrollDirection.Horizontal : ScrollDirection.Vertical
        return true
      case this.SHOW_SCROLLER_INDICATOR:
        this.scrollBar = HRValueUtils.getNumber(propValue as HippyAny) == 1 ? BarState.On : BarState.Off
        return true
      case this.VERTICAL_BOUNCES:
      case this.HORIZONTAL_BOUNCES:
      case this.BOUNCES_ENABLE:
        this.edgeEffect = HRValueUtils.getNumber(propValue as HippyAny) == 1 ? EdgeEffect.Spring : EdgeEffect.None
        return true;
      case this.FLING_ENABLE:
        this.friction = (HRValueUtils.getNumber(propValue as HippyAny) == 1) ? 20 : 0.6 // no params in ohos
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
      case this.WILL_DRAG_END:
        this.willDragEndEvent = propValue as HippyRenderCallback
        return true
      case this.PAGING_ENABLED:
        this.pageEnabled = (HRValueUtils.getNumber(propValue as HippyAny) == 0) ? false : true
        return true
      case this.CONTENT_OFFSET:
        this.setContentOffset4Reuse(HRValueUtils.getMap(propValue as HippyAny) as HippyMap)
        return true
      case this.SCROLL_ENABLED:
        this.enableScrollInteraction = HRValueUtils.getBoolean(propValue as HippyAny,true)
        return true
      case this.HORIZONTAL:
        this.axis =HRValueUtils.getBoolean(propValue as HippyAny,true) ? ScrollDirection.Horizontal : ScrollDirection.Vertical
        return true
      case this.INITIAL_CONTENTOFFSET:
        this.initialContentOffset = HRValueUtils.getNumber(propValue as HippyAny)
        return true
      case this.SCROLL_EVENT_THROTTLE:
        this.scrollEventThrottle = HRValueUtils.getNumber(propValue as HippyAny)
        return true
      case this.SCROLL_MINOFFSET:
        this.scrollMinOffset = HRValueUtils.getNumber(propValue as HippyAny)
        return true
    }
    return super.setProp(propKey, propValue as HippyAny)
  }

  setContentOffset4Reuse(propValue: HippyMap) {
    if (this.axis == ScrollDirection.Vertical) {
      this.scroller.scrollTo({ xOffset: 0, yOffset: propValue.get('y') as number })
    } else {
      this.scroller.scrollTo({ xOffset: propValue.get('x') as number, yOffset: 0 })
    }
  }

  call(method: string, params: Array<HippyAny>, callback: HippyRenderCallback | null): void {
    switch (method) {
      case this.METHOD_CONTENT_OFFSET:
        if (params.length > 0) {
          this.setContentOffset(params[0])
        }
        break
      case this.METHOD_CONTENT_INSET_WHEN_END_DRAG:
        if (params.length > 0) {
          this.setContentInsetWhenEndDrag(params[0])
        }
        break
      case this.METHOD_CONTENT_INSET:
        if (params.length > 0) {
          this.setContentInset(params[0])
        }
        break
      case this.METHOD_SCROLLTO:
        this.hanldeScrollTo(params)
        break
      case this.METHOD_SCROLLTO_OPTIONS:
        this.handleScrollToWithOptions(params[0] as HippyMap)
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
    if (this.axis == ScrollDirection.Vertical) {
      return (this.children[0] as HRScrollContentView)?.viewHeight > yOffset
    } else {
      return (this.children[0] as HRScrollContentView)?.viewWidth > xOffset
    }
  }

  private setContentInsetWhenEndDrag(params: HippyAny) {
    let top: number = 0
    let left: number = 0
    let bottom: number = 0
    let right: number = 0
    let animation: boolean = false

    let arr: Array<string> = (params as string).split(this.BLANK_SEPARATOR)
    top = Number(arr[0])
    left = Number(arr[1])
    bottom = Number(arr[2])
    right = Number(arr[3])
    if (arr.length > 4) {
      animation = Number(arr[4]) == 1
    }

    if (this.axis == ScrollDirection.Vertical) {
      this.contentInsetWhenDrag = top - bottom
    } else {
      this.contentInsetWhenDrag = left - right
    }
    (this.children[0] as HRScrollContentView).startOffset = this.contentInsetWhenDrag
  }

  private setContentInset(params: HippyAny) {
    let top: number = 0
    let left: number = 0
    let bottom: number = 0
    let right: number = 0
    let animation: boolean = false

    let arr: Array<string> = (params as string).split(this.BLANK_SEPARATOR)
    top = Number(arr[0])
    left = Number(arr[1])
    bottom = Number(arr[2])
    right = Number(arr[3])
    if (arr.length > 4) {
      animation = Number(arr[4]) == 1
    }

    if (this.axis == ScrollDirection.Vertical) {
      this.contentInset = bottom - top
    } else {
      this.contentInset = right - left
    }
    (this.children[0] as HRScrollContentView).startOffset = this.contentInset
  }

  hanldeScrollTo(params: Array<HippyAny>) {
    let xOffset: number = params[0] as number
    let yOffset: number = params[1] as number
    let animated: boolean = params[2] as boolean
    this.scroller.scrollTo({ xOffset: xOffset, yOffset: yOffset, animation: animated })
  }

  handleScrollToWithOptions(options: HippyMap) {
    let xOffset: number = 0
    let yOffset: number = 0
    let duration: number = 0

    if (typeof options.get('x') === 'number') {
      xOffset = options.get('x') as number
    }
    if (typeof options.get('y') === 'number') {
      yOffset = options.get('y') as number
    }
    if (typeof options.get('duration') === 'number') {
      duration = options.get('duration') as number
    }

    if (duration > 0) {
      this.scroller.scrollTo({
        xOffset: xOffset,
        yOffset: yOffset,
        animation: { duration: duration }
      })
    } else {
      this.scroller.scrollTo({ xOffset: xOffset, yOffset: yOffset })
    }
  }
}

@Component
export struct HRScroll {
  @ObjectLink renderView: HRScrollView
  @ObjectLink children: HippyObservedArray<HippyRenderBaseView>
  @BuilderParam buildRenderView: ($$: HippyRenderBaseView, parent: HRComponent) => void
  @BuilderParam buildCustomRenderView: ($$: HippyRenderBaseView) => void | null
  @State xOffset: number = 0
  @State yOffset: number = 0
  viewWidth: number = 0
  viewHeight: number = 0
  private isDragging: boolean = false
  private needFireWillEndDragEvent: boolean = false
  private lastScrollOffset: number = 0
  private lastScrollTime: number = 0

  private fireBeginDragEvent() {
    this.needFireWillEndDragEvent = true
    if (this.renderView.dragBeginEvent) {
      this.renderView.dragBeginEvent(this.generateParam())
    }
  }

  private fireEndDragEvent() {
    if (this.needFireWillEndDragEvent) {
      this.fireWillDragEndEvent()
    }
    if (this.renderView.dragEndEvent) {
      this.renderView.dragEndEvent(this.generateParam())
    }
  }

  private fireWillDragEndEvent() {
    this.needFireWillEndDragEvent = false
  }

  private generateParam(): HippyRecord {
    return {
      "offsetX": this.renderView.scroller.currentOffset().xOffset,
      "offsetY": this.renderView.scroller.currentOffset().yOffset,
      "contentWidth": (this.renderView.children[0] as HRScrollContentView)?.viewWidth,
      "contentHeight": (this.renderView.children[0] as HRScrollContentView)?.viewHeight,
      "viewWidth": this.viewWidth,
      "viewHeight": this.viewHeight,
      "isDragging": this.isDragging ? 1 : 0
    } as HippyRecord
  }

  aboutToAppear() {

  }

  emitScrollEvent(eventName: string) {
    let contentInset = new Map<string, number>() as HippyMap;
    contentInset.set("top", 0);
    contentInset.set("bottom", 0);
    contentInset.set("left", 0);
    contentInset.set("right", 0);

    //todo: should using PixelUtil.px2dp?
    let contentOffset = new Map<string, number>() as HippyMap;
    contentOffset.set("x", this.renderView.scroller.currentOffset().xOffset);
    contentOffset.set("y", this.renderView.scroller.currentOffset().yOffset);

    let contentSize = new Map<string, number>() as HippyMap;
    contentSize.set("width", this.renderView.children.length > 0 ? this.renderView.children[0].cssWidth : this.renderView.cssWidth);
    contentSize.set("height", this.renderView.children.length > 0 ? this.renderView.children[0].cssHeight : this.renderView.cssHeight);

    let layoutMeasurement = new Map<string, number>() as HippyMap;
    layoutMeasurement.set("width", this.renderView.cssWidth);
    layoutMeasurement.set("height", this.renderView.cssHeight);

    let params = new Map<string, HippyMap>() as Hippy2Map;
    params.set("contentInset", contentInset);
    params.set("contentOffset", contentOffset);
    params.set("contentSize", contentSize);
    params.set("layoutMeasurement", layoutMeasurement);

    HREventUtils.sendComponentEvent(this.renderView.ctx, this.renderView.tag, eventName, params as HippyMap);
  }

  build() {
    Scroll(this.renderView.scroller) {
      Stack() {
        if (this.children.length > 0) {
          this.buildRenderView(this.children[0], this)
        }
      }
    }
    .applyRenderViewBaseAttr(this.renderView)
    .edgeEffect(this.renderView.edgeEffect)
    .scrollable(this.renderView.axis)
    .friction(this.renderView.friction)
    .enablePaging(this.renderView.pageEnabled)
    .enableScrollInteraction(this.renderView.enableScrollInteraction)
    .onScroll((xOffset: number, yOffset: number) => {
      this.xOffset = xOffset
      this.yOffset = yOffset

      let now = systemDateTime.getUptime(systemDateTime.TimeType.STARTUP)
      if (this.renderView.axis == ScrollDirection.Vertical) {
        if (this.renderView.scrollMinOffset != 0 && Math.abs(yOffset - this.lastScrollOffset) > this.renderView.scrollMinOffset) {
          this.lastScrollOffset = yOffset
          this.emitScrollEvent(HREventUtils.EVENT_SCROLLER_ON_SCROLL)
          return
        }
      } else {
        if (this.renderView.scrollMinOffset != 0 && Math.abs(xOffset - this.lastScrollOffset) > this.renderView.scrollMinOffset) {
          this.lastScrollOffset = xOffset
          this.emitScrollEvent(HREventUtils.EVENT_SCROLLER_ON_SCROLL)
          return
        }
      }

      if (this.renderView.scrollMinOffset == 0 && (now - this.lastScrollTime) > this.renderView.scrollEventThrottle) {
        this.lastScrollTime = now
        this.emitScrollEvent(HREventUtils.EVENT_SCROLLER_ON_SCROLL)
      }
    })
    .onScrollStop(() => {
      //consider for change between vertical and Horizontal
      this.lastScrollOffset = 0
      this.lastScrollTime = 0

      if (this.renderView.scrollEndEvent) {
        this.renderView.scrollEndEvent(this.generateParam())
      }
    })
    .onTouch((e: TouchEvent) => {
      switch (e.type) {
        case TouchType.Down:
        case TouchType.Move:
          this.isDragging = true
          this.fireBeginDragEvent()
          break
        case TouchType.Up:
        case TouchType.Cancel:
          this.isDragging = false
          this.fireEndDragEvent()
          break
      }
    })
    .onAreaChange((oldArea: Area, newArea: Area) => {
      this.viewWidth = newArea.width as number
      this.viewHeight = newArea.height as number
    })
    .onAppear(() => {
      if (this.renderView.axis == ScrollDirection.Vertical) {
        this.renderView.scroller.scrollTo({
          xOffset: 0,
          yOffset: this.renderView.initialContentOffset
        })
      } else {
        this.renderView.scroller.scrollTo({
          xOffset: this.renderView.initialContentOffset,
          yOffset: 0
        })
      }
    })
  }
}

// base props for all components
@Extend(Scroll)
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
  .scrollBar(($$ as HRScrollView).scrollBar)
}
