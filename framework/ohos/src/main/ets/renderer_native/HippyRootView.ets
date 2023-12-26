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

import { HippyAbility, HippyEngine } from '../hippy_framework'
import { NativeRenderContext } from './NativeRenderContext';
import { ComponentBuilderContext, HippyComponentFactory } from './HippyComponentFactory';
import { Descriptor, Tag } from './descriptor/DescriptorBase';
import { ViewBaseProps } from './components/HippyViewBase';
import { convertColorSegmentsToString } from './descriptor/DescriptorUtils';

export type RootDescriptor = Descriptor<"RootView", ViewBaseProps>

@Component
@Preview
export struct HippyRootView {
  @StorageLink('HippyAbility') private hippyAbility: HippyAbility | undefined = undefined

  public hippyEngine: HippyEngine | undefined = undefined

  // TODO(hot):
  @State private shouldShow: boolean = true

  private hippyEngine_: HippyEngine | undefined = undefined

  ctx!: NativeRenderContext
  tag!: Tag
  @BuilderParam buildCustomComponent: (ctx: ComponentBuilderContext) => void
  @State descriptor: RootDescriptor = {} as RootDescriptor
  private unregisterDescriptorChangesListener?: () => void = undefined

  surfaceWidth: number = 0
  surfaceHeight: number = 0
  surfaceOffsetX: number = 0
  surfaceOffsetY: number = 0

  aboutToAppear() {
    this.hippyEngine_ = this.getOrCreateHippyEngine()
    this.hippyEngine_.runJsBundle(this.hippyAbility!.context.resourceManager)

    this.ctx = this.hippyEngine_.getNativeRenderContext()
    this.tag = this.ctx.getRootId()
    this.ctx.descriptorRegistry.createRootDescriptor(this.tag)
    this.descriptor = this.ctx.descriptorRegistry.getDescriptor(this.tag)
    this.unregisterDescriptorChangesListener = this.ctx.descriptorRegistry.subscribeToDescriptorChanges(
      this.tag,
      (newDescriptor) => {
        this.descriptor = newDescriptor as RootDescriptor
      }
    )
  }

  aboutToDisappear() {
    this.unregisterDescriptorChangesListener?.();
  }

  private getOrCreateHippyEngine(): HippyEngine {
    return this.hippyAbility!.getHippyEngine()
  }

  handleAreaChange(oldValue: Area, newValue: Area) {
    this.surfaceWidth = newValue.width as number
    this.surfaceHeight = newValue.height as number
    this.surfaceOffsetX = newValue.globalPosition.x as number
    this.surfaceOffsetY = newValue.globalPosition.y as number
    this.updateSurfaceConstraints()
  }

  updateSurfaceConstraints() {
    // TODO(hot):
    this.hippyEngine_!.getNativeRenderProvider().onSizeChanged(10, this.surfaceWidth, this.surfaceHeight)
  }

  handleTouch(event: TouchEvent) {

  }

  build() {
    Stack() {
      if (this.shouldShow) {
        ForEach(this.descriptor.childrenTags, (childrenTag: number) => {
          HippyComponentFactory({ ctx: this.ctx, tag: childrenTag, buildCustomComponent: this.buildCustomComponent })
        }, (childrenTag: number) => String(childrenTag))
      }
    }
    .width("100%")
    .height("100%")
    .backgroundColor(this.descriptor.props.backgroundColor)//convertColorSegmentsToString(this.descriptor.props.backgroundColor))
    .focusable(false)
    .onAreaChange((oldValue: Area, newValue: Area) => {
      this.handleAreaChange(oldValue, newValue)
    })
    .onTouch(this.handleTouch)
  }
}
