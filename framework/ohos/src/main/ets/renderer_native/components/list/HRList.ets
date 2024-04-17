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
import { HippyMap, Hippy2Map } from '../../../support/common/HippyTypes';
import { HRComponent } from '../../HippyRenderView';
import HippyRenderBaseView, {
  HippyObservedArray,
  HIPPY_COMPONENT_KEY_PREFIX } from '../base/HippyRenderBaseView';
import { HREventUtils } from '../../utils/HREventUtils'
import { LogUtils } from '../../../support/utils/LogUtils';
import { HRPullHeaderView } from '../pullheader/HRPullHeader';
import { HRPullFooterView } from '../pullfooter/HRPullFooter';
import { HRListItemView } from './HRListItemView';
import { HRListDataSource } from './HRListDataSource';
import { HRListView } from './HRListView';
import { HippyEndBatchCallback } from '../../uimanager/HRViewManager';

const INVALID_STICKY_INDEX: number = -1

enum ScrollAction {
  None,
  PullHeader,
  ReleaseHeader,
  PullFooter,
  ReleaseFooter
}

@Component
export struct HRList {
  private readonly TAG = "HRList"
  @ObjectLink renderView: HRListView
  @ObjectLink children: HippyObservedArray<HippyRenderBaseView>
  @BuilderParam buildRenderView:($$: HippyRenderBaseView, parent: HRComponent) => void
  @BuilderParam buildCustomRenderView: ($$: HippyRenderBaseView) => void | null
  private CURRENT_OFFSET: string = "contentOffset";
  private pullAction: ScrollAction = ScrollAction.None;
  private headerView: HRPullHeaderView | null = null;
  private footerView: HRPullFooterView | null = null;
  private footerYOff: number = 0;
  private lastScrollTime: number = 0;
  private isLastTimeReachEnd: boolean = false;

  @State stickyIndex: number = INVALID_STICKY_INDEX
  private stickyArray: number[] = []
  private stickyStack: number[] = []

  private isFirstTouch: boolean = true
  private isDragging: boolean = false

  private lastMoveY: number = 0

  private endBatchCallback: HippyEndBatchCallback | null = null

  dataSource: HRListDataSource | null = null

  private handleOnChildrenUpdated() {
    let childrenCount = this.children.length
    if (childrenCount > 0) {
      if (this.children[0] instanceof HRPullHeaderView) {
        this.headerView = this.children[0] as HRPullHeaderView
      }
      if (this.children[childrenCount - 1] instanceof HRPullFooterView) {
        this.footerView = this.children[childrenCount - 1] as HRPullFooterView
      }
    }

    let fullY = 0
    this.children.forEach((element, index) => {
      fullY += element.cssHeight

      if (element instanceof HRListItemView) {
        if ((element as HRListItemView).sticky) {
          this.stickyArray.push(index)
        }
      }
    })

    fullY -= this.renderView.cssHeight

    if (this.footerView) {
      this.renderView.lastItemFullVisibleYOffset = fullY - this.footerView.cssHeight
      this.renderView.footerViewFullVisibleYOffset = fullY
    } else {
      this.renderView.lastItemFullVisibleYOffset = fullY
      this.renderView.footerViewFullVisibleYOffset = 0
    }
  }

  aboutToAppear(): void {
    LogUtils.d(this.TAG, "aboutToAppear: ")
    this.dataSource = new HRListDataSource(this.children)
    this.renderView.uiCtx = this.getUIContext()

    this.handleOnChildrenUpdated()

    this.endBatchCallback = () => {
      this.handleOnChildrenUpdated()
      this.dataSource?.notifyReload()
    }
    this.renderView.ctx.getNativeRender().addEndBatchCallback(this.renderView.ctx.getRootId(), this.endBatchCallback)
  }

  aboutToDisappear(): void {
    LogUtils.d(this.TAG, "aboutToDisappear: ")
    this.renderView.ctx.getNativeRender().removeEndBatchCallback(this.renderView.ctx.getRootId(), this.endBatchCallback as HippyEndBatchCallback)
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
      let currentTime = systemDateTime.getTime(false);
      // this.currentOffset = this.renderView.scroller.currentOffset().yOffset;
      if (currentTime - this.lastScrollTime >= this.renderView.scrollEventThrottle) {
        this.lastScrollTime = currentTime;
        this.emitScrollEvent(HREventUtils.EVENT_REFRESH_WRAPPER_SCROLL);
      }
    }
  }

  // 可视面积小于10%，任务view当前已经不在可视区域
  calculateExposureState(currentRatio: number) {
    if (currentRatio >= 1) {
      this.renderView.exposureState = this.renderView.EXPOSURE_STATE_FULL_VISIBLE;
    } else if (currentRatio > 0.1) {
      this.renderView.exposureState = this.renderView.EXPOSURE_STATE_PART_VISIBLE;
    } else {
      this.renderView.exposureState = this.renderView.EXPOSURE_STATE_INVISIBLE;
    }
  }

  moveToExposureState(state: number) {
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
  }

  checkSendExposureEvent() {
    if (!this.renderView.exposureEventEnabled) {
      return;
    }
    this.moveToExposureState(this.renderView.exposureState);
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
    if (this.renderView.preloadItemNumber > 0 && (this.children.length - lastIndex) < this.renderView.preloadItemNumber) {
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

  private checkBeginDrag() {
    if (this.isFirstTouch) {
      this.isFirstTouch = false
      if (this.headerView) {
        let offset = this.renderView.scroller.currentOffset()
        if (offset) {
          let y = offset.yOffset
          this.renderView.dtYOffset = y - this.headerView.cssHeight
        }
      }
    }

    if (!this.isDragging) {
      this.isDragging = true
    }
  }

  private checkEndDrag() {
    if (this.isDragging) {
      this.isDragging = false

      let offset = this.renderView.scroller.currentOffset()
      let y = offset.yOffset
      y -= this.renderView.dtYOffset
      if (this.headerView && this.pullAction == ScrollAction.PullHeader) {
        if (y <= 0) {
          HREventUtils.sendComponentEvent(this.headerView.ctx, this.headerView.tag,
            HREventUtils.EVENT_LIST_HEADER_RELEASED, null)
        } else if (y < this.headerView.cssHeight) {
          this.renderView.scroller.scrollToIndex(1, true)
        }
      } else if (this.footerView && this.pullAction == ScrollAction.PullFooter) {
        if (this.renderView.footerViewFullVisibleYOffset > 0 && y >= this.renderView.footerViewFullVisibleYOffset) {
          HREventUtils.sendComponentEvent(this.footerView.ctx, this.footerView.tag,
            HREventUtils.EVENT_LIST_FOOTER_RELEASED, null)
        } else if (y > this.renderView.lastItemFullVisibleYOffset) {
          this.renderView.scroller.scrollTo({
            "xOffset": 0,
            "yOffset": this.renderView.lastItemFullVisibleYOffset + this.renderView.dtYOffset,
            "animation": true
          })
        }
      }
    }
  }

  private checkPullOnItemVisibleAreaChange(index: number, isVisible: boolean, currentRatio: number) {
    let lastIndex = this.dataSource!.totalCount() - 1
    if (this.headerView && index == 0) {
      if (isVisible) {
        this.renderView.dtYOffset = 0
        if (this.isDragging) {
          this.pullAction = ScrollAction.PullHeader
        } else {
          this.renderView.scroller.scrollToIndex(1, true)
        }
      } else if (currentRatio <= 0.0) {
        this.pullAction = ScrollAction.None
      }
    } else if (this.footerView && index == lastIndex) {
      if (isVisible) {
        if (this.isDragging) {
          this.pullAction = ScrollAction.PullFooter
        } else {
          this.renderView.scroller.scrollTo({
            "xOffset": 0,
            "yOffset": this.renderView.lastItemFullVisibleYOffset + this.renderView.dtYOffset,
            "animation": true
          })
        }
      } else {
        if (currentRatio <= 0.0) {
          this.pullAction = ScrollAction.None
        }
      }
    }
  }

  private checkPullOnScroll() {
    let offset = this.renderView.scroller.currentOffset();
    let xOff: number = offset.xOffset;
    let yOff: number = offset.yOffset;

    if (this.headerView && this.pullAction === ScrollAction.PullHeader) {
      let params = new Map<string, Number>() as HippyMap;
      params.set(this.CURRENT_OFFSET, -yOff + this.headerView.cssHeight);
      HREventUtils.sendComponentEvent(this.headerView.ctx, this.headerView.tag,
        HREventUtils.EVENT_LIST_HEADER_PULLING, params);
    } else if (this.footerView && this.pullAction === ScrollAction.PullFooter) {
      let params = new Map<string, Number>() as HippyMap;
      params.set(this.CURRENT_OFFSET, yOff);
      HREventUtils.sendComponentEvent(this.footerView.ctx, this.footerView.tag,
        HREventUtils.EVENT_LIST_FOOTER_PULLING, params);
    }
  }

  private checkStickyOnItemVisibleAreaChange(index: number, isVisible: boolean, currentRatio: number) {
    let moveUp = false
    let offset = this.renderView.scroller.currentOffset()
    if (offset) {
      let y = offset.yOffset
      if (y > this.lastMoveY) {
        moveUp = true
      }
      this.lastMoveY = y
    }

    if (!isVisible && moveUp) {
      if (this.stickyIndex != index && this.stickyArray.indexOf(index) >= 0) {
        this.stickyStack.push(index)
        this.stickyIndex = index
      }
    }

    if (isVisible && currentRatio >= 1.0) {
      if (this.stickyStack.length > 0 && this.stickyStack[this.stickyStack.length - 1] == index) {
        this.stickyStack.pop()
      }
      if (this.stickyStack.length > 0) {
        this.stickyIndex = this.stickyStack[this.stickyStack.length - 1]
      } else {
        this.stickyIndex = INVALID_STICKY_INDEX
      }
    }
  }

  build() {
    Stack() {
      List({ initialIndex: this.headerView ? 1 : 0, scroller: this.renderView.scroller }) {
        LazyForEach(this.dataSource, (item: HippyRenderBaseView, index) => {
          ListItem() {
            this.buildRenderView(item, null)
          }
          .onVisibleAreaChange([0.0, 1.0], (isVisible: boolean, currentRatio: number) => {
            this.checkPullOnItemVisibleAreaChange(index, isVisible, currentRatio)
            this.checkStickyOnItemVisibleAreaChange(index, isVisible, currentRatio)
          })
        }, (item: HippyRenderBaseView) => item.tag + '')
      }
      .listDirection(this.renderView.listDirection)
      .enableScrollInteraction(this.renderView.scrollEnabled)
      .cachedCount(this.renderView.cachedCount)
      .scrollBar(BarState.Off)
      .edgeEffect(EdgeEffect.Spring)
      .onTouch((e: TouchEvent) => {
        switch (e.type) {
          case TouchType.Down:
          case TouchType.Move:
            this.checkBeginDrag()
            break
          case TouchType.Up:
          case TouchType.Cancel:
            this.checkEndDrag()
            break
        }
      })
      .nestedScroll({
        scrollForward: this.renderView.scrollForward,
        scrollBackward: this.renderView.scrollBackward
      })
      .onScrollIndex((firstIndex: number, lastIndex: number, centerIndex: number) => {
        LogUtils.d(this.TAG, "onScrollIndex: firstIndex=" + firstIndex +
          ", lastIndex=" + lastIndex + ", centerIndex=" + centerIndex + ",Yoffset=" + this.renderView.scroller.currentOffset()
          .yOffset);
        this.checkSendReachEndEvent(lastIndex);
        this.checkSendExposureEvent();
      })
      .onScroll((scrollOffset: number, scrollState: ScrollState) => {
        this.handleScrollEvent(scrollOffset);
        this.checkPullOnScroll()
      })
      .onScrollStart(() => {
        LogUtils.d(this.TAG, "onScroll: onScrollStart=");
        this.emitScrollEvent(HREventUtils.EVENT_SCROLLER_MOMENTUM_BEGIN);
      })
      .onScrollStop(() => {
        // LogUtils.d(this.TAG, "onScroll: onScrollStop=");
        // this.emitScrollEvent(HREventUtils.EVENT_SCROLLER_MOMENTUM_END);
        // let xOff: number = this.renderView.scroller.currentOffset().xOffset;
        // let yOff: number = this.renderView.scroller.currentOffset().yOffset;
        // this.emitScrollEvent(HREventUtils.EVENT_SCROLLER_ON_SCROLL)
        //
        // //scroll to head then release
        // if (this.headerView && this.pullAction == ScrollAction.PullHeader) {
        //   LogUtils.d(this.TAG, "onScroll: onScrollStop=ScrollAction.PullHeader" + yOff);
        //   if (this.renderView.edgeState == EdgeAction.ReachStart) {
        //     LogUtils.d(this.TAG, "onScroll: onScrollStop=this.renderView.edgeState == EdgeAction.ReachStart" + yOff);
        //     HREventUtils.sendComponentEvent(this.headerView.ctx, this.headerView.tag,
        //       HREventUtils.EVENT_LIST_HEADER_RELEASED, null);
        //   } else {
        //     LogUtils.d(this.TAG, "onScroll: onScrollStop=scrollToIndex" + yOff);
        //     this.renderView.scroller.scrollToIndex(1, true)
        //   }
        //   this.pullAction = ScrollAction.None;
        // }
      })
      .onReachStart(() => {
        LogUtils.d(this.TAG, 'onReachStart')
        // this.renderView.edgeState = EdgeAction.ReachStart;
        // if (this.pullAction === ScrollAction.ReleaseHeader) {
        //   this.pullAction = ScrollAction.None;
        // }
      })
      .onReachEnd(() => {
        LogUtils.d(this.TAG, 'onReachEnd')
        this.sendOnReachedEvent();
        // this.renderView.edgeState = EdgeAction.ReachEnd;
        // if (this.pullAction === ScrollAction.ReleaseFooter) {
        //   this.pullAction = ScrollAction.None;
        //   this.renderView.edgeState = EdgeAction.None;
        // }
        // this.footerYOff = this.renderView.scroller.currentOffset().yOffset;
      })
      .onDragStart(() => this.emitScrollEvent(HREventUtils.EVENT_SCROLLER_BEGIN_DRAG))
      .onDragEnd(() => this.emitScrollEvent(HREventUtils.EVENT_SCROLLER_END_DRAG))
      .onVisibleAreaChange([0.0, 1.0], (isVisible: boolean, currentRatio: number) => {
        this.calculateExposureState(currentRatio);
      })

      // sticky item
      ForEach(this.stickyArray, (index: number) => {
        if (index == this.stickyIndex) {
          this.buildRenderView(this.children[index], null)
        }
      })
    }.applyRenderViewBaseAttr(this.renderView)
  }
}

// base props for all components
@Extend(Stack)
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
