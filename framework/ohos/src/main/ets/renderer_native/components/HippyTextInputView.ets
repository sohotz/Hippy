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

import { NativeRenderContext } from '../NativeRenderContext';
import { HippyViewBase, ViewBaseProps } from './HippyViewBase';
import { ColorSegments, Descriptor } from '../descriptor/DescriptorBase';
import { convertColorSegmentsToString } from '../descriptor/DescriptorUtils';

export class TextInputProps extends ViewBaseProps {
  fontColor?: ColorSegments
  fontSize?: number
  text?: string
  multiline?: boolean
  placeholder?: string
  placeholderTextColor?: ColorSegments
  maxLength?: number
}

export type TextInputDescriptor = Descriptor<"TextInput", TextInputProps>

@Component
export struct HippyTextInputView {
  ctx!: NativeRenderContext
  tag!: number
  @State descriptor: TextInputDescriptor = {} as TextInputDescriptor
  private unregisterDescriptorChangesListener?: () => void = undefined

  handleChangeEvent(value: string) {
    // TODO(hot):
  }

  aboutToAppear() {
    this.descriptor = this.ctx.descriptorRegistry.getDescriptor<TextInputDescriptor>(this.tag)
    this.unregisterDescriptorChangesListener = this.ctx.descriptorRegistry.subscribeToDescriptorChanges(this.tag,
      (newDescriptor) => {
        this.descriptor = (newDescriptor as TextInputDescriptor)
      }
    )
  }

  aboutToDisappear() {
    this.unregisterDescriptorChangesListener?.()
  }

  build() {
    HippyViewBase({ctx: this.ctx, tag: this.tag}) {
      if (this.descriptor.props.multiline) {
        TextArea({ placeholder: this.descriptor.props.placeholder })
          .width("100%")
          .height("100%")
          .fontColor(convertColorSegmentsToString(this.descriptor.props.fontColor))
          .fontSize(this.descriptor.props.fontSize)
          .placeholderColor(convertColorSegmentsToString(this.descriptor.props.placeholderTextColor))
          .maxLength(this.descriptor.props.maxLength)
      } else {
        TextInput({ placeholder: this.descriptor.props.placeholder })
          .width("100%")
          .height("100%")
          .fontColor(convertColorSegmentsToString(this.descriptor.props.fontColor))
          .fontSize(this.descriptor.props.fontSize)
          .borderRadius(this.descriptor.props.borderRadius)
          .onChange((event) => this.handleChangeEvent(event))
          .placeholderColor(convertColorSegmentsToString(this.descriptor.props.placeholderTextColor))
          .maxLength(this.descriptor.props.maxLength)
      }
    }
  }
}
