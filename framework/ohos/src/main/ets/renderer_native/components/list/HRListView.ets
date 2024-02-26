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
import HippyRenderBaseView, {
  HippyObservedArray } from '../base/HippyRenderBaseView';
import { HREventUtils } from '../../utils/HREventUtils'
import { LogUtils } from '../../../support/utils/LogUtils';
import { HRPullHeaderView } from '../pullheader/HRPullHeader';
import { HRPullFooterView } from '../pullfooter/HRPullFooter';
import { HRRect } from '../../utils/HRTypes';
import { PixelUtil } from '../../../support/utils/PixelUtil';

@Observed
export class HRListView extends HippyRenderBaseView {
  private readonly TAG = "HRListView"
  public flex: number = 0
  public scrollForward: number = NestedScrollMode.PARENT_FIRST
  public scrollBackward: number = NestedScrollMode.PARENT_FIRST
  public readonly EXPOSURE_STATE_FULL_VISIBLE: number = 1;
  public readonly EXPOSURE_STATE_INVISIBLE: number = 2;
  public readonly EXPOSURE_STATE_PART_VISIBLE: number = 3;
  public scroller: Scroller = new Scroller()

  listDirection: Axis = Axis.Vertical;
  scrollEnabled: boolean = true;
  initialOffset: number = 0;
  initialIndex: number = 0;
  cachedCount: number = 0;
  scrollEventThrottle: number = 30;
  preloadItemNumber: number = 0;
  onScrollEventEnable: boolean = true;
  exposureEventEnabled: Boolean = true;
  exposureState: number = this.EXPOSURE_STATE_INVISIBLE;

  constructor(ctx: NativeRenderContext) {
    super(ctx)
  }

  setProp(propKey: string, propValue: HippyAny | HippyRenderCallback): boolean {
    LogUtils.d(this.TAG, "setProp: propKey " + propKey + ", value: " + propValue)
    switch (propKey) {
      case "cssPositionX":
        this.cssPositionX = propValue as number;
        break;
      case "cssPositionY":
        this.cssPositionY = propValue as number;
        break;
      case 'nestedScrollTopPriority':
        {
          let pvalue = propValue as string
          if (pvalue === 'parent') {
            this.scrollForward = NestedScrollMode.PARENT_FIRST
            this.scrollBackward = NestedScrollMode.PARENT_FIRST
          } else if (pvalue === 'self') {
            this.scrollForward = NestedScrollMode.SELF_FIRST
            this.scrollBackward = NestedScrollMode.SELF_FIRST
          }
        }
        break;
      case "cssHeight":
        this.cssHeight = propValue as number;
        break;
      case "horizontal":
        this.listDirection = propValue as boolean == true ? Axis.Horizontal : Axis.Vertical;
        break;
      case "scrollEnabled":
        this.scrollEnabled = propValue as boolean;
        break;
      case "initialContentOffset":
        this.initialOffset = propValue as number;
        break;
      case "itemViewCacheSize":
        this.cachedCount = propValue as number;
        break;
      case "scrollEventThrottle":
        this.scrollEventThrottle = propValue as number;
        break;
      case "preloadItemNumber":
        this.preloadItemNumber = propValue as number;
        break;
      case "exposureEventEnabled":
        this.exposureEventEnabled = propValue as boolean;
        break;
      default:
        break;
    }

    return super.setProp(propKey, propValue as HippyAny)
  }

  call(method: string, params: Array<HippyAny>, callback: HippyRenderCallback | null): void {
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
  }

  setRenderViewFrame(frame: HRRect): void {
    frame.x = this.cssPositionX;
    frame.y = this.cssPositionY;
    if (this.cssHeight > 0) {
      frame.height = this.cssHeight;
    }

    super.setRenderViewFrame(frame)
  }
}

enum ScrollAction {
  None,
  PullHeader,
  ReleaseHeader,
  PullFooter,
  ReleaseFooter
}

enum EdgeAction {
  None,
  ReachStart,
  ReachEnd
}

@Component
export struct HRList {
  private readonly TAG = "HRList"
  @ObjectLink renderView: HRListView
  @ObjectLink children: HippyObservedArray<HippyRenderBaseView>

  @BuilderParam buildRenderView:($$: HippyRenderBaseView, parent: HRComponent) => void
  private CURRENT_OFFSET: string = "contentOffset";
  private pullAction: ScrollAction = ScrollAction.None;
  private edgeState: EdgeAction = EdgeAction.None;
  private headerView: HRPullHeaderView | null = null;
  private footerView: HRPullFooterView | null = null;
  private footerYOff: number = 0;
  private lastScrollTime: number = 0;
  private currentOffset: number = 0;
  private isLastTimeReachEnd: boolean = false;

  aboutToAppear(): void {
    LogUtils.d(this.TAG, "aboutToAppear: " )
    this.renderView.setProp("cssPositionX", 0);
    this.renderView.setProp("cssPositionY", 0);
    this.children.forEach(element => {
      if (element instanceof HRPullHeaderView) {
        this.headerView = element as HRPullHeaderView;
        this.headerView.parentView = this.renderView;
        this.headerView.setProp("cssPositionX", 0)
        this.headerView.setProp("cssPositionY", this.headerView.cssHeight)
      } else if (element instanceof  HRPullFooterView) {
        this.footerView = element as HRPullFooterView;
        this.footerView.parentView = this.renderView;
        this.footerView.setProp("cssPositionX", 0)
        this.footerView.setProp("cssPositionY", this.footerView.cssHeight)
      }
    });
  }

  aboutToDisappear(): void {
    LogUtils.d(this.TAG, "aboutToDisappear: ")
  }

  emitScrollEvent(eventName : string){
    let contentInset = new Map<string, number>() as HippyMap;
    contentInset.set("top", 0);
    contentInset.set("bottom", 0);
    contentInset.set("left", 0);
    contentInset.set("right", 0);

    let contentOffset = new Map<string, number>() as HippyMap;
    contentOffset.set("x",PixelUtil.px2dp(this.renderView.scroller.currentOffset().xOffset));
    contentOffset.set("y",PixelUtil.px2dp(this.renderView.scroller.currentOffset().yOffset));

    let contentSize = new Map<string, number>() as HippyMap;
    contentSize.set("width",PixelUtil
      .px2dp(this.renderView.children.length > 0 ? this.renderView.children[0].cssWidth : this.renderView.cssWidth));
    contentSize.set("height",PixelUtil
      .px2dp(this.renderView.children.length > 0 ? this.renderView.children[0].cssHeight : this.renderView.cssHeight));

    let layoutMeasurement = new Map<string, number>() as HippyMap;
    layoutMeasurement.set("width",PixelUtil.px2dp(this.renderView.cssWidth));
    layoutMeasurement.set("height",PixelUtil.px2dp(this.renderView.cssHeight));

    let params = new Map<string, HippyMap>() as Hippy2Map;
    params.set("contentInset",contentInset);
    params.set("contentOffset",contentOffset);
    params.set("contentSize",contentSize);
    params.set("layoutMeasurement",layoutMeasurement);

    HREventUtils.sendComponentEvent(this.renderView.ctx, this.renderView.tag, eventName, params as HippyMap);
  }

  checkSendOnScrollEvent(){
    if(this.renderView.onScrollEventEnable) {
      let currentTime = systemDateTime.getTime(true);
      this.currentOffset = this.renderView.scroller.currentOffset().yOffset;
      if (currentTime - this.lastScrollTime >= this.renderView.scrollEventThrottle) {
        this.lastScrollTime = currentTime;
        this.emitScrollEvent(HREventUtils.EVENT_REFRESH_WRAPPER_SCROLL);
      }
    }
  }

  //可视面积小于10%，任务view当前已经不在可视区域
  calculateExposureState(): number {
    // TODO
    return this.renderView.EXPOSURE_STATE_FULL_VISIBLE;
  }
  moveToExposureState(state: number){
    if (state == this.renderView.exposureState) {
      return;
    }
    switch (state) {
      case this.renderView.EXPOSURE_STATE_FULL_VISIBLE:
        if (this.renderView.exposureState == this.renderView.EXPOSURE_STATE_INVISIBLE) {
          HREventUtils.sendComponentEvent(this.renderView.ctx, this.renderView.tag, HREventUtils.EVENT_LIST_ITEM_WILL_APPEAR, null);
        }
        HREventUtils.sendComponentEvent(this.renderView.ctx, this.renderView.tag, HREventUtils.EVENT_LIST_ITEM_APPEAR, null);
        break;
      case this.renderView.EXPOSURE_STATE_PART_VISIBLE:
        if (this.renderView.exposureState == this.renderView.EXPOSURE_STATE_FULL_VISIBLE) {
          HREventUtils.sendComponentEvent(this.renderView.ctx, this.renderView.tag, HREventUtils.EVENT_LIST_ITEM_WILL_DISAPPEAR, null);
        } else {
          HREventUtils.sendComponentEvent(this.renderView.ctx, this.renderView.tag, HREventUtils.EVENT_LIST_ITEM_WILL_APPEAR, null);
        }
        break;
      case this.renderView.EXPOSURE_STATE_INVISIBLE:
        if (this.renderView.exposureState == this.renderView.EXPOSURE_STATE_FULL_VISIBLE) {
          HREventUtils.sendComponentEvent(this.renderView.ctx, this.renderView.tag, HREventUtils.EVENT_LIST_ITEM_WILL_DISAPPEAR, null);
        }
        HREventUtils.sendComponentEvent(this.renderView.ctx, this.renderView.tag, HREventUtils.EVENT_LIST_ITEM_DISAPPEAR, null);
        break;
      default:
        break;
    }
    this.renderView.exposureState = state;
  }

  checkSendExposureEvent() {
    if(!this.renderView.exposureEventEnabled) {
      return;
    }
    let newState: number = this.calculateExposureState();
    this.moveToExposureState(newState);
  }
  /**
   * 检查是否已经触底，发生onEndReached事件给前端
   * 如果上次是没有到底，这次滑动底了，需要发事件通知，如果上一次已经是到底了，这次到底不会发事件
   */
  checkSendReachEndEvent(lastIndex: number) {
    let isThisTimeReachEnd: boolean = false;
    isThisTimeReachEnd = this.isReachEnd(lastIndex);

    if (!this.isLastTimeReachEnd && isThisTimeReachEnd) {
      this.sendOnReachedEvent();
    }
    this.isLastTimeReachEnd = isThisTimeReachEnd;
  }

  isReachEnd(lastIndex: number): boolean {
    if(this.renderView.preloadItemNumber > 0 && (this.children.length - lastIndex) < this.renderView.preloadItemNumber) {
      return true;
    } else {
      return false;
    }
  }

  handleScrollEvent(lastIndex: number) {
    this.checkSendOnScrollEvent();
    this.checkSendExposureEvent();
  }

  sendOnReachedEvent() {
    HREventUtils.sendComponentEvent(this.renderView.ctx, this.renderView.tag, HREventUtils.EVENT_RECYCLER_END_REACHED, null);
    HREventUtils.sendComponentEvent(this.renderView.ctx, this.renderView.tag, HREventUtils.EVENT_RECYCLER_LOAD_MORE, null);
  }

  build() {
    Column() {
      Stack() {
        ForEach(this.children, (item: HippyRenderBaseView) => {
          if (item.viewType === 'PullHeaderView') {
            this.buildRenderView(item, null)
          }
        }, (item: HippyRenderBaseView) => item.tag + '')
      }
      .zIndex(1)

      Stack() {
        List({ space: 20, initialIndex: 0, scroller: this.renderView.scroller }) {
          ForEach(this.children, (item: HippyRenderBaseView) => {
            if (item.viewType === 'ListViewItem') {
              ListItem() {
                this.buildRenderView(item, null)
              }
            }
          }, (item: HippyRenderBaseView) => item.tag + '')
        }
        .width('100%')
        .height('100%')
        .applyRenderViewBaseAttr(this.renderView)
        .listDirection(this.renderView.listDirection)
        .enableScrollInteraction(this.renderView.scrollEnabled)
        .cachedCount(this.renderView.cachedCount)
        .scrollBar(BarState.Off)
        .friction(0.6)
        .divider({ strokeWidth: 2, color: 0xFFFFFF, startMargin: 0, endMargin: 0 })
        .edgeEffect(EdgeEffect.Spring)
        .nestedScroll({
          scrollForward: this.renderView.scrollForward,
          scrollBackward: this.renderView.scrollBackward})
        .onScrollIndex((firstIndex: number, lastIndex: number, centerIndex: number) => {
          LogUtils.d(this.TAG, "onScrollIndex: firstIndex=" + firstIndex +
            ", lastIndex=" + lastIndex + ", centerIndex=" + centerIndex);
          this.checkSendReachEndEvent(lastIndex);
          this.checkSendExposureEvent();
        })
        .onScroll((scrollOffset: number, scrollState: ScrollState) => {
          LogUtils.d(this.TAG, "onScroll: scrollOffset=" + scrollOffset +
            ", scrollState=" + scrollState );
          this.handleScrollEvent(scrollOffset);
          let xOff: number = this.renderView.scroller.currentOffset().xOffset;
          let yOff: number = this.renderView.scroller.currentOffset().yOffset;
          // LogUtils.d(this.TAG, "current xOffset=" + xOff + ", yOffset=" + yOff )
          if (this.pullAction === ScrollAction.None && scrollOffset < 0 &&
            scrollState === ScrollState.Scroll && this.edgeState === EdgeAction.ReachStart) {
            //scroll to head
            let params = new Map<string, Number>() as HippyMap;
            params.set(this.CURRENT_OFFSET, -yOff);
            if (this.headerView) {
              HREventUtils.sendComponentEvent(this.headerView.ctx, this.headerView.tag,
                HREventUtils.EVENT_LIST_HEADER_PULLING, params);
              this.headerView.setProp("visibility", 1);
              this.headerView.setProp("cssPositionX", xOff);
              this.headerView.setProp("cssPositionY", -yOff+this.headerView.cssHeight);
              this.renderView.setProp("cssPositionX", 0);
              this.renderView.setProp("cssPositionY", this.headerView.cssHeight);
            }
            this.pullAction = ScrollAction.PullHeader
            LogUtils.d(this.TAG, "ScrollAction: PullHeader=" )
          } else if (this.pullAction === ScrollAction.PullHeader && scrollOffset > 0 &&
            scrollState === ScrollState.Fling && yOff < 0.1) {
            //scroll to head then release
            let params = new Map<string, Number>() as HippyMap;
            params.set(this.CURRENT_OFFSET, -yOff);
            if (this.headerView) {
              HREventUtils.sendComponentEvent(this.headerView.ctx, this.headerView.tag,
                HREventUtils.EVENT_LIST_HEADER_RELEASED, params);
            }
            this.pullAction = ScrollAction.ReleaseHeader;
            LogUtils.d(this.TAG, "ScrollAction: ReleaseHeader=" )
          } else if ((this.pullAction === ScrollAction.PullHeader ||
            this.pullAction === ScrollAction.ReleaseHeader) &&
            this.edgeState === EdgeAction.ReachStart) {
            if (this.headerView) {
              if (scrollState === ScrollState.Scroll) {
                let params = new Map<string, Number>() as HippyMap;
                params.set(this.CURRENT_OFFSET, -yOff);
                HREventUtils.sendComponentEvent(this.headerView.ctx, this.headerView.tag,
                  HREventUtils.EVENT_LIST_HEADER_PULLING, params);
                this.headerView.setProp("cssPositionX", xOff);
                this.headerView.setProp("cssPositionY", -yOff+this.headerView.cssHeight);
              } else {
                this.headerView.setProp("cssPositionX", xOff);
                this.headerView.setProp("cssPositionY", -yOff+this.headerView.cssHeight);
              }
            }
          } else if (this.pullAction === ScrollAction.None && scrollOffset > 0 &&
            scrollState === ScrollState.Scroll && this.edgeState === EdgeAction.ReachEnd) {
            //scroll to end,
            let params = new Map<string, Number>() as HippyMap;
            yOff = yOff - this.footerYOff
            params.set(this.CURRENT_OFFSET, yOff);
            LogUtils.d(this.TAG, "onScroll: footerYOff=" + yOff )
            if (this.footerView) {
              HREventUtils.sendComponentEvent(this.footerView.ctx, this.footerView.tag,
                HREventUtils.EVENT_LIST_FOOTER_PULLING, params);
              this.footerView.setProp("visibility", 1);
              this.footerView.setProp("cssPositionX", xOff);
              this.footerView.setProp("cssPositionY", -yOff);
              this.renderView.setProp("cssHeight", this.renderView.cssHeight-this.footerView.cssHeight)
            }
            this.pullAction = ScrollAction.PullFooter;
            LogUtils.d(this.TAG, "ScrollAction: PullFooter=" )
          } else if (this.pullAction === ScrollAction.PullFooter && scrollOffset < 0 &&
            scrollState === ScrollState.Fling && this.edgeState === EdgeAction.ReachEnd) {
            //scroll to end then release
            let params = new Map<string, Number>() as HippyMap;
            yOff = yOff - this.footerYOff
            params.set(this.CURRENT_OFFSET, yOff);
            if (this.footerView) {
              LogUtils.d(this.TAG, "onScroll: footerYOff=" + yOff )
              HREventUtils.sendComponentEvent(this.footerView.ctx, this.footerView.tag,
                HREventUtils.EVENT_LIST_FOOTER_RELEASED, params);
              this.footerView.setProp("cssPositionX", xOff);
              this.footerView.setProp("cssPositionY", -yOff);
            }
            this.pullAction = ScrollAction.ReleaseFooter;
            LogUtils.d(this.TAG, "ScrollAction: ReleaseFooter=" )
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
              }
              LogUtils.d(this.TAG, "onScroll: footerYOff=" + yOff )
              this.footerView.setProp("cssPositionX", xOff);
              this.footerView.setProp("cssPositionY", -yOff);
            }
          }
        })
        .onScrollStart(() => {
          LogUtils.d(this.TAG, "onScroll: onScrollStart=" );
          this.emitScrollEvent(HREventUtils.EVENT_SCROLLER_MOMENTUM_BEGIN);
        })
        .onScrollStop(() => {
          LogUtils.d(this.TAG, "onScroll: onScrollStop=" );
          this.emitScrollEvent(HREventUtils.EVENT_SCROLLER_MOMENTUM_END);
          let xOff: number = this.renderView.scroller.currentOffset().xOffset;
          let yOff: number = this.renderView.scroller.currentOffset().yOffset;
          let curOff = new Map<string, Number>() as HippyMap;
          curOff.set("x", xOff);
          curOff.set("y", yOff);
          let params = new Map<string, HippyMap>() as Hippy2Map;
          params.set(this.CURRENT_OFFSET, curOff);
          HREventUtils.sendComponentEvent(this.renderView.ctx, this.renderView.tag,
            HREventUtils.EVENT_SCROLLER_ON_SCROLL, params as HippyMap);
        })
        .onReachStart(() => {
          this.edgeState = EdgeAction.ReachStart;
          if (this.pullAction === ScrollAction.ReleaseHeader) {
            this.pullAction = ScrollAction.None;
          }
        })
        .onReachEnd(() => {
          this.sendOnReachedEvent();
          this.edgeState = EdgeAction.ReachEnd;
          if (this.pullAction === ScrollAction.ReleaseFooter) {
            this.pullAction = ScrollAction.None;
            this.edgeState = EdgeAction.None;
          }
          this.footerYOff = this.renderView.scroller.currentOffset().yOffset;
        })
        .onDragStart(() => this.emitScrollEvent(HREventUtils.EVENT_SCROLLER_BEGIN_DRAG))
        .onDragEnd(() => this.emitScrollEvent(HREventUtils.EVENT_SCROLLER_END_DRAG))
      }
      Stack() {
        ForEach(this.children, (item: HippyRenderBaseView) => {
          if (item.viewType === 'PullFooterView') {
            this.buildRenderView(item, null)
          }
        }, (item: HippyRenderBaseView) => item.tag + '')
      }
    }
    .zIndex(-1)
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
  .zIndex($$.cssZIndex)
  .border($$.cssBorder)
  .rotate($$.cssRotate)
  .scale($$.cssScale)
  .translate($$.cssTranslate)
  .onClick($$.eventClick)
}
