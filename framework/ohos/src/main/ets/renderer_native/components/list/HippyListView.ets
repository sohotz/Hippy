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

import { NativeRenderContext } from '../../NativeRenderContext';
import { HippyViewBase, ViewBaseProps } from '../HippyViewBase';
import { ColorSegments, Descriptor } from '../../descriptor/DescriptorBase';
import { ComponentBuilderContext, HippyComponentFactory } from '../..';

export class ListViewProps extends ViewBaseProps {

}

export type ListViewDescriptor = Descriptor<"ListView", ListViewProps>

@Component
export struct HippyListView {
  ctx!: NativeRenderContext
  tag!: number
  @BuilderParam buildCustomComponent: (ctx: ComponentBuilderContext) => void
  @State descriptor: ListViewDescriptor = {} as ListViewDescriptor
  private unregisterDescriptorChangesListener?: () => void = undefined
  scroller: Scroller = new Scroller()

  @BuilderParam renderDescriptors: () => void

  cnt: number = 0

  aboutToAppear() {
    this.descriptor = this.ctx.descriptorRegistry.getDescriptor<ListViewDescriptor>(this.tag)
    this.unregisterDescriptorChangesListener = this.ctx.descriptorRegistry.subscribeToDescriptorChanges(this.tag,
      (newDescriptor) => {
        const newImageDescriptor = (newDescriptor as ListViewDescriptor)
        this.descriptor = newImageDescriptor
      }
    )
  }

  aboutToDisappear() {
    this.unregisterDescriptorChangesListener?.()
  }

  build() {
    HippyViewBase({
      ctx: this.ctx,
      tag: this.tag,
    }) {
      List({ space: 20, initialIndex: 0 }) {
        ForEach(this.descriptor.childrenTags, (childrenTag: number) => {
          ListItem() {
            // Text(String(++this.cnt))
            //   .width('100%')
            //   .height(40)
            //   .fontSize(16)
            //   .textAlign(TextAlign.Center)
            //   .borderRadius(10)
            //   .backgroundColor(0xFFFFFF)
            HippyComponentFactory({ ctx: this.ctx, tag: childrenTag, buildCustomComponent: this.buildCustomComponent })
          }
        }, (childrenTag: number) => String(childrenTag))
      }
      .listDirection(Axis.Vertical)
      .scrollBar(BarState.Off)
      .friction(0.6)
      .divider({ strokeWidth: 2, color: 0xFFFFFF, startMargin: 0, endMargin: 0 })
      .edgeEffect(EdgeEffect.Spring)
      .onScrollIndex((firstIndex: number, lastIndex: number, centerIndex: number) => {

      })
      .onScroll((scrollOffset: number, scrollState: ScrollState) => {

      })
      .width("100%")
      .height("100%")
    }
  }
}
