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
import systemDateTime from '@ohos.systemDateTime';
import { HippyAny, HippyMap, Hippy2Map, HippyRenderCallback } from '../../../support/common/HippyTypes';
import { HRComponent } from '../../HippyRenderView';
import { NativeRenderContext } from '../../NativeRenderContext';
import { HREventUtils } from '../../utils/HREventUtils'
import HippyRenderBaseView, {
  HippyObservedArray,
  HIPPY_COMPONENT_KEY_PREFIX } from '../base/HippyRenderBaseView';
import { LogUtils } from '../../../support/utils/LogUtils';
import { HRValueUtils } from '../../utils/HRValueUtils';
import { HRPullFooterView } from '../pullfooter/HRPullFooter';

@Observed
export class HRWaterfallView extends HippyRenderBaseView {
  private readonly TAG = "HRWaterfallView"
  public flex: number = 0
  public bannerViewMatch:boolean = false
  public containPullFooter: boolean = false
  public padding: Padding = {top:0, right:0, bottom:0, left:0};
  public containPullHeader: boolean = false
  public containBannerView: boolean = true
  public scrollEventThrottle: number = 200
  public preloadItemNumber: number = 0
  public interItemSpacing: number = 0
  public columnSpacing: number = 0
  public columnsTemplate: string = '1fr'
  public x: number = 0
  public y: number = 0
  public uiCtx: UIContext | null = null
  public childrenOffset: number[] = []
  public scroller: Scroller = new Scroller()
  onScrollEventEnable: boolean = true;
  constructor(ctx: NativeRenderContext) {
    super(ctx)
  }

  setProp(propKey: string, propValue: HippyAny | HippyRenderCallback): boolean {
    LogUtils.d(this.TAG, `setProp: propKey:${propKey} , value:${propValue}`)
    switch (propKey) {
      case 'flex':
        this.flex = HRValueUtils.getNumber(propValue as HippyAny);
        break;
      case 'bannerViewMatch':
        this.bannerViewMatch = HRValueUtils.getBoolean(propValue as boolean, false)
        break
      case 'containPullFooter':
        this.containPullFooter = HRValueUtils.getBoolean(propValue as HippyAny,true);
        break;
      case 'contentInset':
        let data:HippyMap = HRValueUtils.getMap(propValue as HippyAny) as HippyMap;
        this.padding.top = data.get('top') as number;
        this.padding.right = data.get('right') as number;
        this.padding.bottom = data.get('bottom') as number;
        this.padding.left = data.get('left') as number;
        break;
      case 'containPullHeader':
        this.containPullHeader = HRValueUtils.getBoolean(propValue as HippyAny,true);
        break;
      case 'containBannerView':
        this.containBannerView = HRValueUtils.getBoolean(propValue as HippyAny,true);
        break;
      case 'scrollEventThrottle':
        this.scrollEventThrottle = HRValueUtils.getNumber(propValue as HippyAny);
        break;
      case 'preloadItemNumber':
        this.preloadItemNumber = HRValueUtils.getNumber(propValue as HippyAny);
        break;
      case 'interItemSpacing':
        this.interItemSpacing = HRValueUtils.getNumber(propValue as HippyAny);
        break;
      case 'columnSpacing':
        this.columnSpacing = HRValueUtils.getNumber(propValue as HippyAny);
        break;
      case 'numberOfColumns':
        let i = 1
        for (i = 1; i < HRValueUtils.getNumber(propValue as HippyAny); i++) {
          this.columnsTemplate += ' 1fr'
        }
        break;
      default:
      break;
    }

    return super.setProp(propKey, propValue as HippyAny)
  }

  call(method: string, params: Array<HippyAny>, callback: HippyRenderCallback | null): void {
    this.uiCtx?.runScopedTask(() => {
      LogUtils.d(this.TAG, "call: method " + method + ", params: " + params)
      switch (method) {
        case "scrollToIndex":
          {
            let startOff = params[0] as number;
            let endOff = params[1] as number;
            let needAnimation = params[2] as boolean;
            this.scroller.scrollToIndex(endOff-4, needAnimation)
          }
          break;
        case "scrollToContentOffset":
          {
            let startOff = params[0] as number;
            let endOff = params[1] as number;
            let needAnimation = params[2] as boolean;
            this.scroller.scrollTo({
              xOffset: 0,
              yOffset: endOff,
              animation: needAnimation
            })
          }
          break;
        case "scrollToTop":
          {
            this.scroller.scrollToIndex(0);
          }
          break
        default:
          break;
      }
    })
  }
}

enum ScrollAction {
  None,
  PullFooter,
  ReleaseFooter
}

enum EdgeAction {
  None,
  ReachStart,
  ReachEnd
}

@Component
export struct HRWaterfall {
  private readonly TAG = "HRWaterfall"
  @ObjectLink renderView: HRWaterfallView
  @ObjectLink children: HippyObservedArray<HippyRenderBaseView>
  @BuilderParam buildRenderView: ($$: HippyRenderBaseView, parent: HRComponent) => void
  @BuilderParam buildCustomRenderView: ($$: HippyRenderBaseView) => void | null
  private PAGE_ITEM_POSITION: string = "position";
  private WATERFALL_INDEX_FIRST: string = "first";
  private WATERFALL_INDEX_LAST: string = "last";
  private footerView: HRPullFooterView | null = null;
  private footerYOff: number = 0;
  private childrenVisible:boolean[] = []
  private isLastTimeReachEnd: boolean = false;
  private lastScrollTime: number = 0;
  private currentOffset: number = 0;
  private pullAction: ScrollAction = ScrollAction.None;
  private CURRENT_OFFSET: string = "contentOffset";
  public edgeState: EdgeAction = EdgeAction.None
  @State index: number = 0

  handleChangeEvent(index: number) {
    LogUtils.d(this.TAG, "handleChangeEvent: index " + index)
    let params = new Map<string, number>() as HippyMap;
    params.set(this.PAGE_ITEM_POSITION, index);
    HREventUtils.sendComponentEvent(this.renderView.ctx, this.renderView.tag, HREventUtils.EVENT_PAGE_SELECTED, params);
  }

  handleGestureSwipeEvent(index: number, extraInfo: SwiperAnimationEvent) {
    LogUtils.d(this.TAG, "handleGestureSwipeEvent: index " + index)
    let params = new Map<string, Number>() as HippyMap;
    params.set(this.PAGE_ITEM_POSITION, index);
    HREventUtils.sendComponentEvent(this.renderView.ctx, this.renderView.tag, HREventUtils.EVENT_PAGE_SELECTED, params);
  }

  handleAnimationStartEvent(index: number, targetIndex: number, extraInfo: SwiperAnimationEvent) {
    LogUtils.d(this.TAG, "handleAnimationStartEvent: index " + targetIndex)
    let params = new Map<string, Number>() as HippyMap;
    params.set(this.PAGE_ITEM_POSITION, targetIndex);
    HREventUtils.sendComponentEvent(this.renderView.ctx, this.renderView.tag, HREventUtils.EVENT_PAGE_SELECTED, params);
    params.set(this.PAGE_ITEM_POSITION, targetIndex);
    HREventUtils.sendComponentEvent(this.renderView.ctx, this.renderView.tag, HREventUtils.EVENT_PAGE_ITEM_WILL_APPEAR, params);
    params.set(this.PAGE_ITEM_POSITION, index);
    HREventUtils.sendComponentEvent(this.renderView.ctx, this.renderView.tag, HREventUtils.EVENT_PAGE_ITEM_WILL_DISAPPEAR, params);
  }

  handleAnimationEnd(index: number, extraInfo: SwiperAnimationEvent) {
    LogUtils.d(this.TAG, "handleAnimationEnd: index " + index)
    let params = new Map<string, Number>() as HippyMap;
    params.set(this.PAGE_ITEM_POSITION, index);
    HREventUtils.sendComponentEvent(this.renderView.ctx, this.renderView.tag, HREventUtils.EVENT_PAGE_SELECTED, params);
  }

  handleScrollToIndex(first: number, last: number){
    LogUtils.d(this.TAG, "handleScrollToIndex: first: " + first + "last: " + last)
    let params = new Map<string, number>() as HippyMap;
    params.set(this.WATERFALL_INDEX_FIRST, first);
    params.set(this.WATERFALL_INDEX_LAST, last);
    this.checkSendReachEndEvent(last);
    HREventUtils.sendComponentEvent(this.renderView.ctx, this.renderView.tag, HREventUtils.EVENT_WATERFALL_SCROLL_REPORT, params);
  }

  aboutToAppear() {
    LogUtils.d(this.TAG, "aboutToAppear ")
    this.renderView.uiCtx = this.getUIContext()

    let offset = 0
    this.children.forEach((element, index) => {
      this.childrenVisible.push(false)

      this.renderView.childrenOffset.push(offset)
      if (element.cssVisibility != Visibility.None) {
        offset += element.cssHeight
      }

    if (element instanceof  HRPullFooterView) {
      this.footerView = element as HRPullFooterView;
      this.footerView.parentView = this.renderView;
      this.footerView.setProp("cssPositionX", 0)
      this.footerView.setProp("cssPositionY", this.footerView.cssHeight)
      }
    })
  }

  aboutToDisappear() {
  }

  emitScrollEvent(eventName: string) {
    let contentInset = new Map<string, number>() as HippyMap;
    contentInset.set("top", 0);
    contentInset.set("bottom", 0);
    contentInset.set("left", 0);
    contentInset.set("right", 0);

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

  checkSendOnScrollEvent() {
    if (this.renderView.onScrollEventEnable) {
      let currentTime = systemDateTime.getTime(true);
      this.currentOffset = this.renderView.scroller.currentOffset().yOffset;
      if (currentTime - this.lastScrollTime >= this.renderView.scrollEventThrottle) {
        this.lastScrollTime = currentTime;
        this.emitScrollEvent(HREventUtils.EVENT_REFRESH_WRAPPER_SCROLL);
      }
    }
  }
  checkSendReachEndEvent(lastIndex: number) {
    let isThisTimeReachEnd: boolean = false;
    isThisTimeReachEnd = this.isReachEnd(lastIndex);

    if (!this.isLastTimeReachEnd && isThisTimeReachEnd) {
      this.sendOnReachedEvent();
    }
    this.isLastTimeReachEnd = isThisTimeReachEnd;
  }

  isReachEnd(lastIndex: number): boolean {
    if (this.renderView.preloadItemNumber > 0 && (this.children.length - lastIndex) < this.renderView.preloadItemNumber) {
      return true;
    } else {
      return false;
    }
  }

  handleScrollEvent(lastIndex: number) {
    this.checkSendOnScrollEvent();
  }

  sendOnReachedEvent() {
    HREventUtils.sendComponentEvent(this.renderView.ctx, this.renderView.tag, HREventUtils.EVENT_RECYCLER_END_REACHED, null);
    HREventUtils.sendComponentEvent(this.renderView.ctx, this.renderView.tag, HREventUtils.EVENT_RECYCLER_LOAD_MORE, null);
  }


  setPage(index: number) {
    this.index = index
  }

  build() {
    Column({ space: this.renderView.interItemSpacing }) {
        if (this.renderView.containBannerView) {
          Stack() {
            this.buildRenderView(this.children[0], null)
          }
          .padding({
            left: this.renderView.bannerViewMatch ? 0 : this.renderView.columnSpacing / 2,
            right: this.renderView.bannerViewMatch ? 0 : this.renderView.columnSpacing / 2,
            bottom: 0,
            top: 0
          })
        }
      Stack() {
        WaterFlow({
          footer: () => {
            null
          }, scroller: this.renderView.scroller
        }) {
          ForEach(this.children, (item: HippyRenderBaseView) => {
            if (item.viewType === 'WaterfallItem') {
              FlowItem() {
                this.buildRenderView(item, null)
              }
            }
          }, (item: HippyRenderBaseView) => item.tag + '')
        }
        .enableScrollInteraction(this.renderView.onScrollEventEnable)
        .scrollBar(BarState.Off)
        .edgeEffect(EdgeEffect.Spring)
        .nestedScroll({
          scrollForward: NestedScrollMode.PARENT_FIRST,
          scrollBackward: NestedScrollMode.SELF_FIRST
        })
        .columnsTemplate("1fr 1fr")
        .columnsGap(this.renderView.columnSpacing)
        .rowsGap(this.renderView.interItemSpacing)
        .onScrollIndex((first: number, last: number) => this.handleScrollToIndex(first, last))
        .onScroll(
          (scrollOffset: number, scrollState: ScrollState) => {
            this.handleScrollEvent(scrollOffset);
            let xOff: number = this.renderView.scroller.currentOffset().xOffset;
            let yOff: number = this.renderView.scroller.currentOffset().yOffset;
            LogUtils.d(this.TAG, "onScroll: xOff=" + xOff + " yOff= " + yOff + " scrollOffset=" + scrollOffset + "  scrollState=" + scrollState)
            if (this.pullAction === ScrollAction.None && scrollOffset > 0 &&
              scrollState === ScrollState.Scroll && this.edgeState === EdgeAction.ReachEnd) {
              //scroll to end,
              let params = new Map<string, Number>() as HippyMap;
              yOff = yOff - this.footerYOff
              params.set(this.CURRENT_OFFSET, yOff);
              LogUtils.d(this.TAG, "onScroll: yOff=" + yOff)
              LogUtils.d(this.TAG, "onScroll: footerYOff=" + this.footerYOff)
              if (this.footerView) {
                this.footerView.setProp("visibility", 1);
                this.footerView.setProp("cssPositionX", xOff);
                this.footerView.setProp("cssPositionY", -yOff);
                this.renderView.setProp("cssHeight", this.renderView.cssHeight - this.footerView.cssHeight)
                LogUtils.d(this.TAG, "footerView1: cssPositionX=" + xOff + "  cssPositionY=" + (-yOff) + "  cssHeight=" + (this.renderView.cssHeight - this.footerView.cssHeight))
              }
              this.pullAction = ScrollAction.PullFooter;
              LogUtils.d(this.TAG, "ScrollAction: PullFooter=")
            } else if (this.pullAction === ScrollAction.PullFooter && scrollOffset < 0 &&
              scrollState === ScrollState.Fling && this.edgeState === EdgeAction.ReachEnd) {
              //scroll to end then release
              let params = new Map<string, Number>() as HippyMap;
              yOff = yOff - this.footerYOff
              params.set(this.CURRENT_OFFSET, yOff);
              if (this.footerView) {
                LogUtils.d(this.TAG, "onScroll: footerYOff=" + yOff)
                HREventUtils.sendComponentEvent(this.footerView.ctx, this.footerView.tag,
                  HREventUtils.EVENT_LIST_FOOTER_RELEASED, params);
                this.footerView.setProp("cssPositionX", xOff);
                this.footerView.setProp("cssPositionY", -yOff);
                LogUtils.d(this.TAG, "footerView2: cssPositionX=" + xOff + "  cssPositionY=" + (-yOff) + "  cssHeight=" + (this.renderView.cssHeight - this.footerView.cssHeight))
              }
              this.pullAction = ScrollAction.ReleaseFooter;
              LogUtils.d(this.TAG, "ScrollAction: ReleaseFooter=")
            } else if ((this.pullAction === ScrollAction.PullFooter ||
              this.pullAction === ScrollAction.ReleaseFooter) &&
              this.edgeState === EdgeAction.ReachEnd) {
              yOff = yOff - this.footerYOff
              if (this.footerView) {
                if (scrollState === ScrollState.Scroll) {
                  let params = new Map<string, Number>() as HippyMap;
                  params.set(this.CURRENT_OFFSET, -yOff);
                  HREventUtils.sendComponentEvent(this.footerView.ctx, this.footerView.tag,
                    HREventUtils.EVENT_LIST_FOOTER_PULLING, params);
                  LogUtils.d(this.TAG, "footerView3: cssPositionX=" + xOff + "  cssPositionY=" + (-yOff) + "  cssHeight=" + (this.renderView.cssHeight - this.footerView.cssHeight))
                }
                LogUtils.d(this.TAG, "onScroll: footerYOff=" + yOff)
                this.footerView.setProp("cssPositionX", xOff);
                this.footerView.setProp("cssPositionY", -yOff);
              }
            }
          })
        .onDragStart(() => this.emitScrollEvent(HREventUtils.EVENT_SCROLLER_BEGIN_DRAG))
        .onDragEnd(() => this.emitScrollEvent(HREventUtils.EVENT_SCROLLER_END_DRAG))
        .onScrollStart(() => {
          LogUtils.d(this.TAG, "onScroll: onScrollStart=");
          this.emitScrollEvent(HREventUtils.EVENT_SCROLLER_MOMENTUM_BEGIN);
        })
        .onScrollStop(() => {
          LogUtils.d(this.TAG, "onScroll: onScrollStop=");
          this.emitScrollEvent(HREventUtils.EVENT_SCROLLER_MOMENTUM_END);
          let xOff: number = this.renderView.scroller.currentOffset().xOffset;
          let yOff: number = this.renderView.scroller.currentOffset().yOffset;
          let curOff = new Map<string, Number>() as HippyMap;
          curOff.set("x", xOff);
          curOff.set("y", yOff);
          let params = new Map<string, HippyMap>() as Hippy2Map;
          params.set(this.CURRENT_OFFSET, curOff);
          //todo: should call emitScrollEvent?
          HREventUtils.sendComponentEvent(this.renderView.ctx, this.renderView.tag,
            HREventUtils.EVENT_SCROLLER_ON_SCROLL, params as HippyMap);
        })
        .onReachStart(() => {
          LogUtils.d(this.TAG, 'onReachStart')
          this.edgeState = EdgeAction.ReachStart;
        })
        .onReachEnd(() => {
          LogUtils.d(this.TAG, 'onReachEnd')
          this.edgeState = EdgeAction.ReachEnd;
          if (this.pullAction === ScrollAction.ReleaseFooter) {
            LogUtils.d(this.TAG, 'onReachEnd............')
            this.pullAction = ScrollAction.None;
            this.edgeState = EdgeAction.None;
          }
          this.footerYOff = this.renderView.scroller.currentOffset().yOffset;
        })
        .width('100%')
        .height('100%')
      }.padding(this.renderView.padding)

      Stack() {
        ForEach(this.children, (item: HippyRenderBaseView) => {
          if (item.viewType === 'PullFooterView') {
            this.buildRenderView(item, null)
          }
        }, (item: HippyRenderBaseView) => item.tag + '')
      }
    }
    .applyRenderViewBaseAttr(this.renderView)
  }
}

// base props for all components
@Extend(Column)
function applyRenderViewBaseAttr($$: HippyRenderBaseView) {
  .key(HIPPY_COMPONENT_KEY_PREFIX + $$.tag)
  .backgroundColor($$.cssBackgroundColor)
   .position({x: $$.cssPositionX, y: $$.cssPositionY})
  .size({width:$$.cssWidth, height: $$.cssHeight})
  .opacity($$.cssOpacity)
  .clip($$.cssOverflow)
  .visibility(($$ as HippyRenderBaseView).cssVisibility) // must add as, otherwise the compiler has error
  .zIndex($$.cssZIndex)
  .accessibilityText($$.cssAccessibilityLabel)
  .focusable($$.cssFocusable)
  .border($$.cssBorder)
  .shadow($$.cssShadow)
  .linearGradient($$.cssLinearGradient)
  .backgroundImage($$.cssBackgroundImage)
  .backgroundImageSize($$.cssBackgroundImageSize)
  .backgroundImagePosition($$.cssBackgroundImagePosition)
  .transform($$.cssMatrix)
  .rotate($$.cssRotate)
  .scale($$.cssScale)
  .translate($$.cssTranslate)
  .hitTestBehavior($$.hitTestBehavior)
  .onClick($$.eventClick)
}
