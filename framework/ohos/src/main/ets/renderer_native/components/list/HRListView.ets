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
  HippyObservedArray,
  HIPPY_COMPONENT_KEY_PREFIX } from '../base/HippyRenderBaseView';
import { HREventUtils } from '../../utils/HREventUtils'
import { LogUtils } from '../../../support/utils/LogUtils';
import { HRPullHeaderView } from '../pullheader/HRPullHeader';
import { HRPullFooterView } from '../pullfooter/HRPullFooter';
import { HRRect } from '../../utils/HRTypes';
import { PixelUtil } from '../../../support/utils/PixelUtil';
import { HRValueUtils } from '../../utils/HRValueUtils';
import { HRListItemView } from './HRListItemView'

const INVALID_STICKY_INDEX: number = -1

enum EdgeAction {
  None,
  ReachStart,
  ReachEnd
}

@Observed
export class HRListView extends HippyRenderBaseView {
  private readonly TAG = "HRListView"
  public flex: number = 0
  public scrollForward: number = NestedScrollMode.SELF_FIRST
  public scrollBackward: number = NestedScrollMode.SELF_FIRST
  public readonly EXPOSURE_STATE_FULL_VISIBLE: number = 1;
  public readonly EXPOSURE_STATE_INVISIBLE: number = 2;
  public readonly EXPOSURE_STATE_PART_VISIBLE: number = 3;
  public scroller: ListScroller = new ListScroller()
  public uiCtx: UIContext | null = null
  public edgeState: EdgeAction = EdgeAction.None
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
  public childrenOffset: number[] = []

  constructor(ctx: NativeRenderContext) {
    super(ctx)
  }

  setProp(propKey: string, propValue: HippyAny | HippyRenderCallback): boolean {
    LogUtils.d(this.TAG, "setProp: propKey " + propKey + ", value: " + propValue)
    switch (propKey) {
      case 'nestedScrollTopPriority':
        {
          let pvalue = HRValueUtils.getString(propValue as HippyAny)
          if (pvalue === 'parent') {
            this.scrollForward = NestedScrollMode.PARENT_FIRST
            //this.scrollBackward = NestedScrollMode.PARENT_FIRST
          } else if (pvalue === 'self') {
            this.scrollForward = NestedScrollMode.SELF_FIRST
            //this.scrollBackward = NestedScrollMode.SELF_FIRST
          }
        }
        break;
      case "cssHeight":
        this.cssHeight = HRValueUtils.getNumber(propValue as HippyAny);
        break;
      case "horizontal":
        this.listDirection = HRValueUtils.getBoolean(propValue as HippyAny, false) == true ? Axis.Horizontal : Axis.Vertical;
        break;
      case "scrollEnabled":
        this.scrollEnabled = HRValueUtils.getBoolean(propValue as HippyAny,true);
        break;
      case "initialContentOffset":
        this.initialOffset = HRValueUtils.getNumber(propValue as HippyAny);
        break;
      case "itemViewCacheSize":
        this.cachedCount = HRValueUtils.getNumber(propValue as HippyAny);
        break;
      case "scrollEventThrottle":
        this.scrollEventThrottle = HRValueUtils.getNumber(propValue as HippyAny);
        break;
      case "preloadItemNumber":
        this.preloadItemNumber = HRValueUtils.getNumber(propValue as HippyAny);
        break;
      case "exposureEventEnabled":
        this.exposureEventEnabled = HRValueUtils.getBoolean(propValue as HippyAny,true);
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
          this.scroller.scrollToItemInGroup(1, endOff-4)
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
  PullHeader,
  ReleaseHeader,
  PullFooter,
  ReleaseFooter
}

class DataSource implements IDataSource {
  private listeners: DataChangeListener[] = [];

  public items: HippyObservedArray<HippyRenderBaseView> = []
  private count:number = 0

  constructor(items:HippyObservedArray<HippyRenderBaseView>) {
    this.items = items

    this.count = items.length
    if (items[0] instanceof HRPullHeaderView) { //pullheader
      this.count--
    }
    if (items[items.length - 1] instanceof HRPullFooterView) { //pullfooter
      this.count--
    }

    LogUtils.d('HRList', `total:${items.length}, count:${this.count}`)
  }

  public getData(index: number): HippyRenderBaseView {
    if (this.items[0] instanceof HRPullHeaderView) {
      // skip pullheaderview
      return this.items[index + 1]
    } else {
      return this.items[index]
    }
  }

  public totalCount(): number {
    return this.count
  }

  registerDataChangeListener(listener: DataChangeListener): void {
    if (this.listeners.indexOf(listener) < 0) {
      console.info('add listener');
      this.listeners.push(listener);
    }
  }

  unregisterDataChangeListener(listener: DataChangeListener): void {
    const pos = this.listeners.indexOf(listener);
    if (pos >= 0) {
      console.info('remove listener');
      this.listeners.splice(pos, 1);
    }
  }

  notifyReload() {
    this.count = this.items.length
    if (this.items[0] instanceof HRPullHeaderView) { //pullheader
      LogUtils.d('HRL', `notifyReload - pullheader`)
      this.count--
    }
    if (this.items[this.items.length - 1] instanceof HRPullFooterView) { //pullfooter
      LogUtils.d('HRL', `notifyReload - pullfooter`)
      this.count--
    }

    this.listeners.forEach(listener => {
      LogUtils.d('HRL', `notifyReload:${this.items.length}, count:${this.count}`)
      listener.onDataReloaded();
    })
  }
}

@Component
export struct HRList {
  private readonly TAG = "HRList"
  @ObjectLink renderView: HRListView
  @ObjectLink @Watch('onChildrenUpdate') children: HippyObservedArray<HippyRenderBaseView>
  @BuilderParam buildRenderView:($$: HippyRenderBaseView, parent: HRComponent) => void
  @BuilderParam buildCustomRenderView: ($$: HippyRenderBaseView) => void | null
  private CURRENT_OFFSET: string = "contentOffset";
  private pullAction: ScrollAction = ScrollAction.None;
  private headerView: HRPullHeaderView | null = null;
  private footerView: HRPullFooterView | null = null;
  private footerYOff: number = 0;
  private lastScrollTime: number = 0;
  private currentOffset: number = 0;
  private isLastTimeReachEnd: boolean = false;
  @State stickyIndex: number = INVALID_STICKY_INDEX
  private stickyArray: number[] = []

  dataSource:DataSource | null = null

  onChildrenUpdate() {
    LogUtils.d(this.TAG, `onChildrenUpdate, total:${this.children.length}`)
    this.dataSource?.notifyReload()
  }

  aboutToAppear(): void {
    LogUtils.d(this.TAG, "aboutToAppear: ")
    this.dataSource = new DataSource(this.children)
    this.renderView.uiCtx = this.getUIContext()

    let offset = 0
    this.children.forEach((element, index) => {
      this.renderView.childrenOffset.push(offset)
      if (element.cssVisibility != Visibility.None) {
        offset += element.cssHeight
      }

      if (element instanceof HRPullHeaderView) {
        this.headerView = element as HRPullHeaderView;
        this.headerView.parentView = this.renderView;
      } else if (element instanceof  HRPullFooterView) {
        this.footerView = element as HRPullFooterView;
        this.footerView.parentView = this.renderView;
        this.footerView.setProp("cssPositionX", 0)
        this.footerView.setProp("cssPositionY", this.footerView.cssHeight)
      } else if (element instanceof HRListItemView) {
        if ((element as HRListItemView).sticky) {
          this.stickyArray.push(index)
        }
      }
    })

    let i = 0
    let firstitem = this.headerView ? 1 : 0
    for (i = 0; i < this.stickyArray.length; i++) {
      if (this.stickyArray[i] <= firstitem) {
        this.stickyIndex = this.stickyArray[i]
        break
      }
    }
    LogUtils.d('AREA', 'stickyIndex:' + this.stickyIndex)
    //this.stickyArray.push(20) //test multi sticky
  }

  aboutToDisappear(): void {
    LogUtils.d(this.TAG, "aboutToDisappear: ")
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

  //可视面积小于10%，任务view当前已经不在可视区域
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

  @Builder
  stickyItem() {
    Stack() {
      // if (this.stickyIndex != INVALID_STICKY_INDEX) {
      //   this.buildRenderView(this.children[this.stickyIndex], null)
      // }

      //this.buildRenderView(this.children[this.stickyIndex], null) cannot refresh the sticky item, use ForEach instead
      ForEach(this.stickyArray, (index: number) => {
        if (index == this.stickyIndex) {
          this.buildRenderView(this.children[index], null)
        }
      })
    }
    .onAreaChange((oldValue: Area, newValue: Area) => {
      if (oldValue.width == 0 && newValue.width != 0) {
        let firstitem = this.headerView ? 1 : 0
        if (this.stickyIndex == firstitem) {
          LogUtils.d('AREA', 'show header:'+ this.stickyIndex)
          this.renderView.scroller.scrollToItemInGroup(1, firstitem)
        }
      }
    })
  }

  build() {
    Column() {
      Stack() {
        List({ initialIndex: this.headerView ? 1 : 0, scroller: this.renderView.scroller }) {
          ListItem() {
            if (this.headerView) {
              this.buildRenderView(this.headerView, null)
            }
          }
          .onVisibleAreaChange([0.0, 1.0], (isVisible: boolean, currentRatio: number) => {
            if (this.headerView && isVisible) {
              this.pullAction = ScrollAction.PullHeader
            }

            if (this.headerView && !isVisible && currentRatio <= 0) {
              this.pullAction = ScrollAction.None
              let i = 0
              for (i = 0; i < this.stickyArray.length; i++) {
                if (this.stickyArray[i] == 1) {
                  this.stickyIndex = this.stickyArray[i]
                  LogUtils.d('AREA', 'headview invisible: stikcy:' + this.stickyIndex)
                  break
                }
              }
            }

            LogUtils.d('AREA', `headview isVisible:${isVisible}, currentRatio:${currentRatio}, pullaction:${this.pullAction}}`)
          })

          ListItemGroup({ header: this.stickyItem() }) {
            LazyForEach(this.dataSource, (item: HippyRenderBaseView, index) => {
            //ForEach(this.children, (item: HippyRenderBaseView, index) => {
              if (item.viewType === 'ListViewItem') {
                ListItem() {
                  this.buildRenderView(item, null)
                }
                .onVisibleAreaChange([0.0, 1.0], (isVisible: boolean, currentRatio: number) => {
                  let childidx = index
                  if (this.headerView) {
                    childidx = index + 1
                  }
                  LogUtils.d('AREA', 'index:' + index + ', isVisible: ' + isVisible + ', currentRatio:' + currentRatio)

                  //should check slide direction, maybe using onscroll offset
                  if (isVisible && currentRatio >= 1.0) {
                    if (childidx == this.stickyIndex) {
                      LogUtils.d('AREA', 'calc INVALID_STICKY_INDEX')
                      this.stickyIndex = INVALID_STICKY_INDEX
                      this.renderView.scroller.scrollToItemInGroup(1, index)
                    }
                  }

                  if (!isVisible && currentRatio > 0) {
                    let i: number = this.stickyArray.length - 1
                    for (; i >= 0; i--) {
                      let itemIdx = this.stickyArray[i]
                      if (childidx == itemIdx) {
                        this.stickyIndex = itemIdx
                        this.renderView.scroller.scrollToItemInGroup(1, index + 1)
                        break
                      }
                    }
                    LogUtils.d('AREA', `calc sticky index:${this.stickyIndex}`)
                  }
                })
              }
            }, (item: HippyRenderBaseView) => item.tag + '')
          }
        }
        .sticky(StickyStyle.Header)
        .listDirection(this.renderView.listDirection)
        .enableScrollInteraction(this.renderView.scrollEnabled)
        .cachedCount(this.renderView.cachedCount)
        .scrollBar(BarState.Off)
        .edgeEffect(EdgeEffect.Spring)
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
          let xOff: number = this.renderView.scroller.currentOffset().xOffset;
          let yOff: number = this.renderView.scroller.currentOffset().yOffset;

          if (this.pullAction === ScrollAction.PullHeader && this.headerView) {
            //LogUtils.d(this.TAG, `pulling:${-yOff}, height:${this.headerView.cssHeight}`)
            let params = new Map<string, Number>() as HippyMap;
            params.set(this.CURRENT_OFFSET, -yOff + this.headerView.cssHeight);
            HREventUtils.sendComponentEvent(this.headerView.ctx, this.headerView.tag,
              HREventUtils.EVENT_LIST_HEADER_PULLING, params);
          }
          else if (this.pullAction === ScrollAction.None && scrollOffset > 0 &&
            scrollState === ScrollState.Scroll && this.renderView.edgeState === EdgeAction.ReachEnd) {
            //scroll to end,
            let params = new Map<string, Number>() as HippyMap;
            yOff = yOff - this.footerYOff
            params.set(this.CURRENT_OFFSET, yOff);
            LogUtils.d(this.TAG, "onScroll: footerYOff0=" + yOff)
            if (this.footerView) {
              HREventUtils.sendComponentEvent(this.footerView.ctx, this.footerView.tag,
                HREventUtils.EVENT_LIST_FOOTER_PULLING, params);
              this.footerView.setProp("visibility", 1);
              this.footerView.setProp("cssPositionX", xOff);
              this.footerView.setProp("cssPositionY", -yOff);
              this.renderView.setProp("cssHeight", this.renderView.cssHeight - this.footerView.cssHeight)
            }
            this.pullAction = ScrollAction.PullFooter;
            LogUtils.d(this.TAG, "ScrollAction: PullFooter=")
          } else if (this.pullAction === ScrollAction.PullFooter && scrollOffset < 0 &&
            scrollState === ScrollState.Fling && this.renderView.edgeState === EdgeAction.ReachEnd) {
            //scroll to end then release
            let params = new Map<string, Number>() as HippyMap;
            yOff = yOff - this.footerYOff
            params.set(this.CURRENT_OFFSET, yOff);
            if (this.footerView) {
              LogUtils.d(this.TAG, "onScroll: footerYOff release=" + yOff)
              HREventUtils.sendComponentEvent(this.footerView.ctx, this.footerView.tag,
                HREventUtils.EVENT_LIST_FOOTER_RELEASED, params);
              this.footerView.setProp("cssPositionX", xOff);
              this.footerView.setProp("cssPositionY", -yOff);
            }
            this.pullAction = ScrollAction.ReleaseFooter;
            LogUtils.d(this.TAG, "ScrollAction: ReleaseFooter=")
          } else if ((this.pullAction === ScrollAction.PullFooter ||
            this.pullAction === ScrollAction.ReleaseFooter) &&
            this.renderView.edgeState === EdgeAction.ReachEnd) {
            yOff = yOff - this.footerYOff
            if (this.footerView) {
              if (scrollState === ScrollState.Scroll) {
                let params = new Map<string, Number>() as HippyMap;
                params.set(this.CURRENT_OFFSET, -yOff);
                HREventUtils.sendComponentEvent(this.footerView.ctx, this.footerView.tag,
                  HREventUtils.EVENT_LIST_FOOTER_PULLING, params);
                LogUtils.d(this.TAG, `onScroll: footerYOff2=${-yOff}`)
              }

              this.footerView.setProp("cssPositionX", xOff);
              this.footerView.setProp("cssPositionY", -yOff);
            }
          }
        })
        .onScrollStart(() => {
          LogUtils.d(this.TAG, "onScroll: onScrollStart=");
          this.emitScrollEvent(HREventUtils.EVENT_SCROLLER_MOMENTUM_BEGIN);
        })
        .onScrollStop(() => {
          LogUtils.d(this.TAG, "onScroll: onScrollStop=");
          this.emitScrollEvent(HREventUtils.EVENT_SCROLLER_MOMENTUM_END);
          let xOff: number = this.renderView.scroller.currentOffset().xOffset;
          let yOff: number = this.renderView.scroller.currentOffset().yOffset;
          this.emitScrollEvent(HREventUtils.EVENT_SCROLLER_ON_SCROLL)

          //scroll to head then release
          if (this.headerView && this.pullAction == ScrollAction.PullHeader) {
            LogUtils.d(this.TAG, "onScroll: onScrollStop=ScrollAction.PullHeader" + yOff);
            if (this.renderView.edgeState == EdgeAction.ReachStart) {
              LogUtils.d(this.TAG, "onScroll: onScrollStop=this.renderView.edgeState == EdgeAction.ReachStart" + yOff);
              HREventUtils.sendComponentEvent(this.headerView.ctx, this.headerView.tag,
                HREventUtils.EVENT_LIST_HEADER_RELEASED, null);
            } else {
              LogUtils.d(this.TAG, "onScroll: onScrollStop=scrollToIndex" + yOff);
              this.renderView.scroller.scrollToIndex(1, true)
            }
            this.pullAction = ScrollAction.None;
          }
        })
        .onReachStart(() => {
          LogUtils.d(this.TAG, 'onReachStart')
          this.renderView.edgeState = EdgeAction.ReachStart;
          if (this.pullAction === ScrollAction.ReleaseHeader) {
            this.pullAction = ScrollAction.None;
          }
        })
        .onReachEnd(() => {
          LogUtils.d(this.TAG, 'onReachEnd')
          this.sendOnReachedEvent();

          this.renderView.edgeState = EdgeAction.ReachEnd;
          if (this.pullAction === ScrollAction.ReleaseFooter) {
            this.pullAction = ScrollAction.None;
            this.renderView.edgeState = EdgeAction.None;
          }
          this.footerYOff = this.renderView.scroller.currentOffset().yOffset;
        })
        .onDragStart(() => this.emitScrollEvent(HREventUtils.EVENT_SCROLLER_BEGIN_DRAG))
        .onDragEnd(() => this.emitScrollEvent(HREventUtils.EVENT_SCROLLER_END_DRAG))
        .onVisibleAreaChange([0.0, 1.0], (isVisible: boolean, currentRatio: number) => {
          this.calculateExposureState(currentRatio);
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
