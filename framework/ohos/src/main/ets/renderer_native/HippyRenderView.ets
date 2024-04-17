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
import window from '@ohos.window'
import { HippyEngine } from '../hippy_framework';
import { HippyAny, HippyRecord } from '../support/common/HippyTypes';
import HippyRenderBaseView, { HippyObservedArray } from './components/base/HippyRenderBaseView';
import {
  hippyRenderViewCreatorRegisterMap,
  HRRenderViewCreator } from './components/HippyRenderRegisterMap';
import { HRImage, HRImageView } from './components/image/HRImageView';
import { HRListItem, HRListItemView } from './components/list/HRListItemView';
import { HRListView } from './components/list/HRListView';
import { HRModal, HRModalView } from './components/modal/HRModalView';
import { HRTextInput, HRTextInputView } from './components/textinput/HRTextInputView'
import { HRViewPager, HRViewPagerView } from './components/pager/HRViewPager';
import { HRViewPagerItem, HRViewPagerItemView } from './components/pager/HRViewPagerItem';
import { HRWaterfall, HRWaterfallView } from './components/waterfall/HRWaterfall';
import { HRWaterfallItem, HRWaterfallItemView } from './components/waterfall/HRWaterfallItem';
import { HRRefreshWrapper,HRRefreshWrapperView } from './components/refreshwrapper/HRRefreshWrapper';
import { HRRefreshWrapperItem, HRRefreshWrapperItemView } from './components/refreshwrapper/HRRefreshWrapperItem';
import { HRRichText, HRRichTextView } from './components/richtext/HRRichTextView';
import { HRScroll, HRScrollView } from './components/scroller/HRScrollView';
import { HRDiv, HRDivView } from './components/view/HRDivView';
import { HRWeb, HRWebView } from './components/web/HRWebView'
import { IHippyRenderExceptionListener } from './IHippyRenderExceptionListener';
import { NativeRenderContext } from './NativeRenderContext';
import { HRManager } from './uimanager/HRManager';
import { HRSize } from './utils/HRTypes';
import { LogUtils } from '../support/utils/LogUtils';
import { HRPullHeader, HRPullHeaderView } from './components/pullheader/HRPullHeader';
import { HRPullFooter, HRPullFooterView } from './components/pullfooter/HRPullFooter';
import { HRDisplaySyncUtils } from './utils/HRDisplaySyncUtils';
import { HRList } from './components/list/HRList';

export type HRComponent = HRDivView | HRScroll | HRModal | null

@Observed
export class HippyRenderView extends HippyRenderBaseView {
  tag: number
  pagerName: string
  pagerData: HippyRecord
  private renderViewSize: HRSize
  private avoidArea: window.AvoidArea | null = null
  private lazyCoreTasks: Array<() => void> = []
  private renderExceptionListener: IHippyRenderExceptionListener | null = null
  uiContext: UIContext | null = null

  constructor(ctx: NativeRenderContext, pageName: string, pageData: HippyRecord, initViewSize: HRSize, avoidArea: window.AvoidArea | null, renderExceptionListener: IHippyRenderExceptionListener | null) {
    super(ctx)
    this.tag = ctx.getRootId()
    this.pagerName = pageName
    this.pagerData = pageData
    this.renderViewSize = initViewSize
    this.avoidArea = avoidArea
    this.renderExceptionListener = renderExceptionListener

    let renderManger = ctx.getNativeRender().getRenderManager()
    renderManger.initViewManager(this)
  }

  setProp(propKey: string, propValue: HippyAny): boolean {
    if (propKey != null && typeof propValue == 'boolean') {
      this.handleRootEvent(propKey, propValue)
    }
    return super.setProp(propKey, propValue as HippyAny)
  }

  private handleRootEvent(event: string, enable: boolean) {
    if (event == HRDisplaySyncUtils.DO_FRAME.toLowerCase()) {
      if (this.uiContext) {
        if (enable) {
          HRDisplaySyncUtils.registerDoFrameListener(this.uiContext, this.ctx.getInstanceId(), this.tag)
        } else {
          HRDisplaySyncUtils.unregisterDoFrameListener(this.uiContext, this.ctx.getInstanceId(), this.tag)
        }
      }
    }
  }

  onSizeChanged(width: number, height: number) {
    if (this.renderViewSize.width == width && this.renderViewSize.height == height) {
      return
    }

    let nativeRender = this.ctx.getNativeRender()
    nativeRender.updateDimension(width, height, false, false)
    nativeRender.onSizeChanged(this.tag, width, height, this.renderViewSize.width, this.renderViewSize.height)

    this.renderViewSize = new HRSize(width, height)
  }

  pageDidAppear(): void {

  }

  pageDidDisappear(): void {

  }

  aboutToDisappear(): void {
    if (this.uiContext) {
      HRDisplaySyncUtils.unregisterDoFrameListener(this.uiContext, this.ctx.getInstanceId(), this.tag)
    }
  }

  destroy(): void {

  }

  addSubRenderView(subRenderView: HippyRenderBaseView): void {
    super.addSubRenderView(subRenderView, -1)
  }

  getRenderViewCreatorRegisterMap(): Map<string, HRRenderViewCreator> {
    let viewRegisterMap = hippyRenderViewCreatorRegisterMap()
    let customCreatorMap = this.ctx.getCustomRenderViewCreatorMap()
    if (customCreatorMap) {
      customCreatorMap.forEach((value, key) => {
        viewRegisterMap.set(key, value)
      })
    }
    return viewRegisterMap
  }

  setRenderExceptionListener(listener: IHippyRenderExceptionListener) {
    this.renderExceptionListener = listener
  }
}

@Component
export struct HippyRender {
  private readonly TAG = "HippyRender"
  @ObjectLink renderView: HippyRenderView
  @ObjectLink children: HippyObservedArray<HippyRenderBaseView>
  @BuilderParam buildCustomRenderView: ($$: HippyRenderBaseView) => void | null

  aboutToAppear() {
    this.renderView.uiContext = this.getUIContext()
  }

  aboutToDisappear(): void {
    this.renderView.aboutToDisappear()
    this.renderView.uiContext = null
  }

  build() {
    Stack() {
      ForEach(this.children, (item: HippyRenderBaseView) => {
        this.buildRenderView(item, null)
      }, (item: HippyRenderBaseView) => item.tag + '')
    }
    .width('100%')
    .height('100%')
  }

  @Builder
  buildRenderView($$: HippyRenderBaseView, parentComponent: HRComponent) {
    if ($$ instanceof HRDivView) {
      HRDiv({
        renderView: $$ as HRDivView,
        children: $$.children,
        buildRenderView: this.buildRenderView,
        buildCustomRenderView: this.buildCustomRenderView
      })
    } else if ($$ instanceof HRScrollView) {
      HRScroll({
        renderView: $$ as HRScrollView,
        children: $$.children,
        buildRenderView: this.buildRenderView,
        buildCustomRenderView: this.buildCustomRenderView
      })
    } else if ($$ instanceof HRRichTextView) {
      HRRichText({ renderView: $$ as HRRichTextView, buildRenderView: this.buildRenderView })
    } else if ($$ instanceof HRImageView) {
      HRImage({ renderView: $$ as HRImageView })
    } else if ($$ instanceof HRModalView) {
      HRModal({
        renderView: $$ as HRModalView,
        children: $$.children,
        buildRenderView: this.buildRenderView,
        buildCustomRenderView: this.buildCustomRenderView
      })
    } else if ($$ instanceof HRListView) {
      HRList({
        renderView: $$ as HRListView,
        children: $$.children,
        buildRenderView: this.buildRenderView,
        buildCustomRenderView: this.buildCustomRenderView
      })
    } else if ($$ instanceof HRListItemView) {
      HRListItem({
        renderView: $$ as HRListItemView,
        children: $$.children,
        buildRenderView: this.buildRenderView,
        buildCustomRenderView: this.buildCustomRenderView
      })
    } else if ($$ instanceof HRTextInputView) {
      HRTextInput({ renderView: $$ as HRTextInputView, children: $$.children, buildRenderView: this.buildRenderView })
    } else if ($$ instanceof HRWebView) {
      HRWeb({ renderView: $$ as HRWebView, children: $$.children, buildRenderView: this.buildRenderView })
    } else if ($$ instanceof HRViewPagerView) {
      HRViewPager({
        renderView: $$ as HRViewPagerView,
        children: $$.children,
        buildRenderView: this.buildRenderView,
        buildCustomRenderView: this.buildCustomRenderView
      })
    } else if ($$ instanceof HRViewPagerItemView) {
      HRViewPagerItem({
        renderView: $$ as HRViewPagerItemView,
        children: $$.children,
        buildRenderView: this.buildRenderView,
        buildCustomRenderView: this.buildCustomRenderView
      })
    } else if ($$ instanceof HRWaterfallItemView) {
      HRWaterfallItem({
        renderView: $$ as HRWaterfallItemView,
        children: $$.children,
        buildRenderView: this.buildRenderView,
        buildCustomRenderView: this.buildCustomRenderView
      })
    } else if ($$ instanceof HRWaterfallView) {
      HRWaterfall({
        renderView: $$ as HRWaterfallView,
        children: $$.children,
        buildRenderView: this.buildRenderView,
        buildCustomRenderView: this.buildCustomRenderView
      })
    } else if ($$ instanceof HRRefreshWrapperView) {
      HRRefreshWrapper({
        renderView: $$ as HRRefreshWrapperView,
        children: $$.children,
        buildRenderView: this.buildRenderView,
        buildCustomRenderView: this.buildCustomRenderView
      })
    } else if ($$ instanceof HRRefreshWrapperItemView) {
      HRRefreshWrapperItem({
        renderView: $$ as HRRefreshWrapperItemView,
        children: $$.children,
        buildRenderView: this.buildRenderView,
        buildCustomRenderView: this.buildCustomRenderView
      })
    } else if ($$ instanceof HRPullHeaderView) {
      HRPullHeader({
        renderView: $$ as HRPullHeaderView,
        children: $$.children,
        buildRenderView: this.buildRenderView,
        buildCustomRenderView: this.buildCustomRenderView
      })
    } else if ($$ instanceof HRPullFooterView) {
      HRPullFooter({
        renderView: $$ as HRPullFooterView,
        children: $$.children,
        buildRenderView: this.buildRenderView,
        buildCustomRenderView: this.buildCustomRenderView
      })
    } else {
      if (this.buildCustomRenderView) {
        this.buildCustomRenderView($$)
      }
    }
  }
}
