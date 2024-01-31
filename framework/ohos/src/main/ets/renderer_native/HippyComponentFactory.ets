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

import { Descriptor, Tag } from './descriptor/DescriptorBase'
import { NativeRenderContext } from "./NativeRenderContext"
import { HippyView } from './components/HippyView';
import { HippyImageView } from './components/HippyImageView';
import { HippyTextView } from './components/HippyTextView';
import { HippyTextInputView } from './components/HippyTextInputView';
import { HippyListView } from './components/list/HippyListView';
import { HippyListItemView } from './components/list/HippyListItemView';
import { HippyModalView } from './components/HippyModalView';

export class ComponentBuilderContext {
  renderContext: NativeRenderContext
  descriptor: Descriptor

  constructor(renderContext: NativeRenderContext, descriptor: Descriptor) {
    this.renderContext = renderContext
    this.descriptor = descriptor
  }
}

@Component
export struct HippyComponentFactory {
  ctx!: NativeRenderContext
  tag!: number
  @BuilderParam buildCustomComponent: (ctx: ComponentBuilderContext) => void
  @State descriptorType: string = ''
  @State childrenTags: Tag[] = []
  private unregisterDescriptorChangesListener?: () => void = undefined

  aboutToAppear() {
    const descriptor = this.ctx.descriptorRegistry.getDescriptor(this.tag)
    if (descriptor) {
      this.setDescriptor(descriptor)
    }
    this.unregisterDescriptorChangesListener = this.ctx.descriptorRegistry.subscribeToDescriptorChanges(
      this.tag,
      (newDescriptor) => this.setDescriptor(newDescriptor)
    )
  }

  aboutToDisappear() {
    this.unregisterDescriptorChangesListener?.()
  }

  setDescriptor(newDescriptor: Descriptor) {
    this.descriptorType = newDescriptor.type
    this.childrenTags = newDescriptor.childrenTags
  }

  @Builder
  renderDescriptors() {
    ForEach(this.childrenTags, (childrenTag: number) => {
      HippyComponentFactory({ ctx: this.ctx, tag: childrenTag, buildCustomComponent: this.buildCustomComponent })
    }, (childrenTag: number) => String(childrenTag))
  }

  build() {
    if (this.descriptorType === "View") {
      HippyView({
        ctx: this.ctx,
        tag: this.tag,
        renderDescriptors: () => {
          this.renderDescriptors()
        }
      })
    } else if (this.descriptorType === "Image") {
      HippyImageView({ ctx: this.ctx, tag: this.tag })
    } else if (this.descriptorType === "Text") {
      HippyTextView({ ctx: this.ctx, tag: this.tag })
    } else if (this.descriptorType === "TextInput") {
      HippyTextInputView({ ctx: this.ctx, tag: this.tag })
    } else if (this.descriptorType === "Modal") {
      HippyModalView({
        ctx: this.ctx,
        tag: this.tag,
        renderDescriptors: () => {
          this.renderDescriptors()
        }
      })
    } else if (this.descriptorType === "ListView") {
      HippyListView({
        ctx: this.ctx,
        tag: this.tag,
        renderDescriptors: () => {
          this.renderDescriptors()
        },
        buildCustomComponent: this.buildCustomComponent
      })
    } else if (this.descriptorType === 'ListViewItem') {
      HippyListItemView({
        ctx: this.ctx,
        tag: this.tag,
        renderDescriptors: () => {
          this.renderDescriptors()
        }
      })
    } else if (this.descriptorType !== '') {
      this.buildCustomComponent(
        new ComponentBuilderContext(
          this.ctx,
          this.ctx.descriptorRegistry.getDescriptor(this.tag)
        )
      )
    }
  }
}
