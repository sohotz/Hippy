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

@Observed
export class HRListView extends HippyRenderBaseView {
  private readonly TAG = "HRListView"
  public flex: number = 0
  public scrollForward: number = NestedScrollMode.PARENT_FIRST
  public scrollBackward: number = NestedScrollMode.PARENT_FIRST
  public scroller: Scroller = new Scroller()
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
        .listDirection(Axis.Vertical)
        .scrollBar(BarState.Off)
        .friction(0.6)
        .divider({ strokeWidth: 2, color: 0xFFFFFF, startMargin: 0, endMargin: 0 })
        .edgeEffect(EdgeEffect.Spring)
        .nestedScroll({
          scrollForward: this.renderView.scrollForward,
          scrollBackward: this.renderView.scrollBackward})
        .onScrollIndex((firstIndex: number, lastIndex: number, centerIndex: number) => {
          LogUtils.d(this.TAG, "onScrollIndex: firstIndex=" + firstIndex +
            ", lastIndex=" + lastIndex + ", centerIndex=" + centerIndex)
        })
        .onScroll((scrollOffset: number, scrollState: ScrollState) => {
          LogUtils.d(this.TAG, "onScroll: scrollOffset=" + scrollOffset +
            ", scrollState=" + scrollState )
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
          LogUtils.d(this.TAG, "onScroll: onScrollStart=" )
        })
        .onScrollStop(() => {
          LogUtils.d(this.TAG, "onScroll: onScrollStop=" )
          let xOff: number = this.renderView.scroller.currentOffset().xOffset;
          let yOff: number = this.renderView.scroller.currentOffset().yOffset;
          let curOff = new Map<string, Number>() as HippyMap;
          curOff.set("x", xOff);
          curOff.set("y", yOff);
          let params = new Map<string, HippyMap>() as Hippy2Map;
          params.set(this.CURRENT_OFFSET, curOff);
          HREventUtils.sendComponentEvent(this.renderView.ctx, this.renderView.tag,
            HREventUtils.EVENT_SCROLLER_ON_SCROLL, params);
        })
        .onReachStart(() => {
          this.edgeState = EdgeAction.ReachStart;
          if (this.pullAction === ScrollAction.ReleaseHeader) {
            this.pullAction = ScrollAction.None;
          }
        })
        .onReachEnd(() => {
          this.edgeState = EdgeAction.ReachEnd;
          if (this.pullAction === ScrollAction.ReleaseFooter) {
            this.pullAction = ScrollAction.None;
            this.edgeState = EdgeAction.None;
          }
          this.footerYOff = this.renderView.scroller.currentOffset().yOffset;
        })
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
