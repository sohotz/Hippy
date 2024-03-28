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
import { HREventUtils } from '../../utils/HREventUtils'
import HippyRenderBaseView, {
  HippyObservedArray,
  HIPPY_COMPONENT_KEY_PREFIX } from '../base/HippyRenderBaseView';
import { LogUtils } from '../../../support/utils/LogUtils';
import { HRValueUtils } from '../../utils/HRValueUtils';
import { HRPullFooterView } from '../pullfooter/HRPullFooter';
import { HRNodeProps } from '../../dom_node/HRNodeProps';
import { HRWaterfallItemView } from './HRWaterfallItem';

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
  public scroller: Scroller = new Scroller()
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
        default:
          break;
      }
    })
  }
}

class DataSource implements IDataSource {
  private listeners: DataChangeListener[] = [];

  items: HippyObservedArray<HippyRenderBaseView> = []
  private count:number = 0

  constructor(items:HippyObservedArray<HippyRenderBaseView>) {
    this.items = items

    this.count = items.length
    if (items[0].viewType != 'WaterfallItem') { //banner
      this.count--
    }
    if (items[items.length - 1].viewType != 'WaterfallItem') { //pullfooter
      this.count--
    }
  }

  public getData(index: number): HippyRenderBaseView {
    if (this.items[0].viewType === 'WaterfallItem') {
      return this.items[index]
    } else {
      // skip banner
      return this.items[index + 1]
    }
  }

  public totalCount(): number {
    return this.count
  }

  registerDataChangeListener(listener: DataChangeListener): void {
    if (this.listeners.indexOf(listener) < 0) {
      this.listeners.push(listener);
    }
  }

  unregisterDataChangeListener(listener: DataChangeListener): void {
    const pos = this.listeners.indexOf(listener);
    if (pos >= 0) {
      this.listeners.splice(pos, 1);
    }
  }

  notifyReload() {
    this.count = this.items.length
    if (this.items[0].viewType != 'WaterfallItem') { //banner
      this.count--
    }
    if (this.items[this.items.length - 1] instanceof HRPullFooterView) { //pullfooter
      this.count--
    }

    this.listeners.forEach(listener => {
      listener.onDataReloaded();
    })
  }
}

@Component
export struct HRWaterfall {
  private readonly TAG = "HRWaterfall"
  @ObjectLink renderView: HRWaterfallView
  @ObjectLink @Watch('onChildrenUpdate') children: HippyObservedArray<HippyRenderBaseView>
  @BuilderParam buildRenderView: ($$: HippyRenderBaseView, parent: HRComponent) => void
  @BuilderParam buildCustomRenderView: ($$: HippyRenderBaseView) => void | null
  private WATERFALL_INDEX_FIRST: string = "first";
  private WATERFALL_INDEX_LAST: string = "last";
  private footerView: HRPullFooterView | null = null;
  footerYOff:number = 0
  @State index: number = 0

  dataSource:DataSource|null = null
  delta:number = 0

  handleScrollToIndex(first: number, last: number){
    LogUtils.d(this.TAG, "handleScrollToIndex: first: " + first + "last: " + last)
    let params = new Map<string, number>() as HippyMap;
    params.set(this.WATERFALL_INDEX_FIRST, first);
    params.set(this.WATERFALL_INDEX_LAST, last);
    HREventUtils.sendComponentEvent(this.renderView.ctx, this.renderView.tag, HREventUtils.EVENT_WATERFALL_SCROLL_REPORT, params);
  }

  onChildrenUpdate() {
    LogUtils.d(this.TAG, `onChildrenUpdate, total:${this.children.length}`)
    if (this.children[this.children.length - 1] instanceof  HRPullFooterView) {
      this.footerView = this.children[this.children.length - 1] as HRPullFooterView
      this.footerView.setProp("cssPositionX", 0)
      this.footerView.setProp("cssPositionY", 0)
    }
    this.dataSource?.notifyReload()
  }

  aboutToAppear() {
    LogUtils.d(this.TAG, `aboutToAppear:${this.children.length}}`)
    this.dataSource = new DataSource(this.children)

    this.renderView.uiCtx = this.getUIContext()

    if (this.children[this.children.length - 1] instanceof  HRPullFooterView) {
      this.footerView = this.children[this.children.length - 1] as HRPullFooterView
      this.footerView.setProp("cssPositionX", 0)
      this.footerView.setProp("cssPositionY", 0)
    }
  }

  aboutToDisappear() {
  }

  @Builder
  pullFooter() {
    if (this.footerView) {
      Stack() {
        this.buildRenderView(this.footerView, null)
      }
    }
  }

  build() {
    Scroll() {
      Column({space:this.renderView.interItemSpacing}) {
        if (this.renderView.containBannerView) {
          Stack() {
            this.buildRenderView(this.children[0], null)
          }
          .padding({left:this.renderView.bannerViewMatch?0:this.renderView.columnSpacing/2, right:this.renderView.bannerViewMatch?0:this.renderView.columnSpacing/2, bottom:0, top:0})
        }
        WaterFlow({footer:this.pullFooter, scroller:this.renderView.scroller}) {
          LazyForEach(this.dataSource, (item:HippyRenderBaseView)=>{
          //ForEach(this.children, (item:HippyRenderBaseView)=>{
            if (item instanceof HRWaterfallItemView) {
              FlowItem() {
                this.buildRenderView(item, null)
              }
            }
          }, (item: HippyRenderBaseView) => item.tag + '')
        }
        .columnsTemplate(this.renderView.columnsTemplate)
        .columnsGap(this.renderView.columnSpacing)
        .rowsGap(this.renderView.interItemSpacing)
        .onScrollIndex((first: number, last: number) => this.handleScrollToIndex(first,last))
        .onScroll((offset:number)=>{
          this.delta = offset

          let yOff = this.renderView.scroller.currentOffset().yOffset
          if (this.footerView && this.footerView.cssVisibility == Visibility.Visible) {
            let params = new Map<string, Number>() as HippyMap;
            yOff = yOff - this.footerYOff
            //LogUtils.d(this.TAG, `onScroll yOffset:${-yOff*2}`)
            params.set("contentOffset", -yOff*2);

            HREventUtils.sendComponentEvent(this.footerView.ctx, this.footerView.tag,
              HREventUtils.EVENT_LIST_FOOTER_PULLING, params);
          }
        })
        .onReachEnd(()=>{
          LogUtils.d(this.TAG, `onReachEnd, footerYOff:${this.footerYOff}, footview:${this.footerView?.cssVisibility}, delta:${this.delta}`)
          if(this.footerView && this.footerView.cssVisibility != Visibility.Visible) {
            this.footerView.setProp("visibility", 1)
            this.footerView.setProp("cssPositionX", 0)
            this.footerView.setProp("cssPositionY", 0)
            // pullfooter hidding also trigger this event and current delta is less than 0
            // if set footerYOff at this time will cause calculateing error
            if (this.delta >= 0) {
              this.footerYOff = this.renderView.scroller.currentOffset().yOffset;
            }
            LogUtils.d(this.TAG, '')
          }
        })
        .onScrollStop(()=>{
          if (this.footerView && this.footerView.cssVisibility === Visibility.Visible) {
            let yOff = this.renderView.scroller.currentOffset().yOffset
            LogUtils.d(this.TAG, `scroll stop yOffset:${yOff}, footerYOff:${this.footerYOff}`)

            if (yOff === this.footerYOff) {
              //do nothing; first time the
              return
            }

            yOff = yOff - this.footerYOff + 1
            LogUtils.d(this.TAG, `scroll stop yOffset:${yOff}, ${this.footerView.cssHeight}`)
            if (yOff > this.footerView.cssHeight) {
              let params = new Map<string, Number>() as HippyMap;
              params.set("contentOffset", yOff);
              HREventUtils.sendComponentEvent(this.footerView.ctx, this.footerView.tag,
                HREventUtils.EVENT_LIST_FOOTER_RELEASED, params);
            } else {
              LogUtils.d(this.TAG, `hidden footerView none`)
              // this.footerView.setProp("cssPositionY", -1)
              this.footerView.cssVisibility = Visibility.None
            }
          }
        })
        .width('100%')
        .height('100%')
        .nestedScroll({
          scrollForward: NestedScrollMode.PARENT_FIRST,
          scrollBackward: NestedScrollMode.SELF_FIRST})
      }.padding(this.renderView.padding)
    }
    .applyRenderViewBaseAttr(this.renderView)
    .enableScrollInteraction(false)
    .scrollBar(BarState.Off)
    .nestedScroll({
      scrollForward: NestedScrollMode.PARENT_FIRST,
      scrollBackward: NestedScrollMode.SELF_FIRST})
  }
}

// base props for all components
@Extend(Scroll)
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
