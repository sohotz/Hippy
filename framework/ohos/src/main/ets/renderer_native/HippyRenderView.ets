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
import { HippyRecord } from '../support/common/HippyTypes';
import HippyRenderBaseView, { HippyObservedArray } from './components/base/HippyRenderBaseView';
import {
  hippyRenderViewCreatorRegisterMap,
  HRRenderViewCreator } from './components/HippyRenderRegisterMap';
import { HRImage, HRImageView } from './components/image/HRImageView';
import { HRListItem, HRListItemView } from './components/list/HRListItemView';
import { HRList, HRListView } from './components/list/HRListView';
import { HRModal, HRModalView } from './components/modal/HRModalView';
import { HRRichText, HRRichTextView } from './components/richtext/HRRichTextView';
import { HRScrollContent, HRScrollContentView } from './components/scroller/HRScrollContentView';
import { HRScroll, HRScrollView } from './components/scroller/HRScrollView';
import { HRDiv, HRDivView } from './components/view/HRDivView';
import { NativeRenderContext } from './NativeRenderContext';
import { HRManager } from './uimanager/HRManager';
import { HRSize } from './utils/HRTypes';

export type HRComponent = HRDivView | HRScroll | HRScrollContent | HRModal | null

@Observed
export class HippyRenderView extends HippyRenderBaseView {
  tag: number
  pagerName: string
  pagerData: HippyRecord
  private renderViewSize: HRSize
  private avoidArea: window.AvoidArea | null = null
  private lazyCoreTasks: Array<() => void> = []

  constructor(ctx: NativeRenderContext, pageName: string, pageData: HippyRecord, initViewSize: HRSize, avoidArea: window.AvoidArea | null) {
    super(ctx)
    this.pagerName = pageName
    this.pagerData = pageData
    this.renderViewSize = initViewSize
    this.avoidArea = avoidArea

    // TODO(hot):
    let renderManger = AppStorage.get("HRManager") as HRManager
    renderManger.init(this)
    this.tag = renderManger.getRootTag()
  }

  onSizeChanged(width: number, height: number) {
    if (this.renderViewSize.width == width && this.renderViewSize.height == height) {
      return
    }
    this.renderViewSize = new HRSize(width, height)
    // TODO(hot):
    let hippyEngine = AppStorage.get("HippyEngine") as HippyEngine
    hippyEngine.getNativeRenderProvider().onSizeChanged(this.tag, width, height)
  }

  sendEvent(event: string, data: HippyRecord): void {

  }

  pageDidAppear(): void {

  }

  pageDidDisappear(): void {

  }

  destroy(): void {

  }

  addSubRenderView(subRenderView: HippyRenderBaseView): void {
    super.addSubRenderView(subRenderView, -1)
  }

  getRenderViewCreatorRegisterMap(): Map<string, HRRenderViewCreator> {
    let viewRegisterMap = hippyRenderViewCreatorRegisterMap()
    // TODO(hot):add custom view type
    return viewRegisterMap
  }

}

@Component
export struct HippyRender {
  @ObjectLink renderView: HippyRenderView
  @ObjectLink children: HippyObservedArray<HippyRenderBaseView>
  @BuilderParam buildCustomRenderView: (renderView: HippyRenderBaseView) => void | null

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
        buildRenderView: this.buildRenderView
      })
    } else if ($$ instanceof HRScrollView) {
      HRScroll({ renderView: $$ as HRScrollView, children: $$.children, buildRenderView: this.buildRenderView })
    } else if ($$ instanceof HRScrollContentView) {
      HRScrollContent({
        renderView: $$ as HRScrollContentView,
        children: $$.children,
        buildRenderView: this.buildRenderView,
        xOffset: (parentComponent as HRScroll)?.xOffset,
        yOffset: (parentComponent as HRScroll)?.yOffset
      })
    } else if ($$ instanceof HRRichTextView) {
      HRRichText({ renderView: $$ as HRRichTextView, spans: $$.textAttr.spans })
    } else if ($$ instanceof HRImageView) {
      HRImage({ renderView: $$ as HRImageView })
    } else if ($$ instanceof HRModalView) {
      HRModal({ renderView: $$ as HRModalView, children: $$.children, buildRenderView: this.buildRenderView })
    } else if ($$ instanceof HRListView) {
      HRList({ renderView: $$ as HRListView, children: $$.children, buildRenderView: this.buildRenderView })
    } else if ($$ instanceof HRListItemView) {
      HRListItem({ renderView: $$ as HRListItemView, children: $$.children, buildRenderView: this.buildRenderView })
    } else {
      if (this.buildCustomRenderView !== null) {
        this.buildCustomRenderView($$)
      }
    }
  }
}
