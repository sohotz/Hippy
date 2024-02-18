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
import { HippyAny } from '../../../support/common/HippyTypes';
import { HRComponent } from '../../HippyRenderView';
import HippyRenderBaseView, { HippyObservedArray } from '../base/HippyRenderBaseView';
import { HRScrollView } from './HRScrollView';

@Observed
export class HRScrollContentView extends HippyRenderBaseView {
  viewWidth: number = 0
  viewHeight: number = 0
  startOffset: number = 0 // content margin, used to support refresh

  setProp(propKey: string, propValue: HippyAny): boolean {
    return super.setProp(propKey, propValue)
  }
}

@Component
export struct HRScrollContent {
  @ObjectLink renderView: HRScrollContentView
  @ObjectLink children: HippyObservedArray<HippyRenderBaseView>
  @BuilderParam buildRenderView: ($$: HippyRenderBaseView, parent: HRComponent) => void
  @Prop xOffset: number
  @Prop yOffset: number

  build() {
    Stack() {
      ForEach(this.children, (item: HippyRenderBaseView) => {
        this.buildRenderView(item, this)
      }, (item: HippyRenderBaseView) => item.tag + '')
    }
    .size({ width: this.renderView.cssWidth, height: this.renderView.cssHeight })
    .onAreaChange((oldArea: Area, newArea: Area) => {
      this.renderView.viewWidth = newArea.width as number
      this.renderView.viewHeight = newArea.height as number
    })
    .offset({
      x: (this.renderView.parentView as HRScrollView)?.axis == ScrollDirection.Horizontal ? this.renderView.startOffset : 0,
      y: (this.renderView.parentView as HRScrollView)?.axis == ScrollDirection.Vertical ? this.renderView.startOffset : 0,
    })
  }
}
