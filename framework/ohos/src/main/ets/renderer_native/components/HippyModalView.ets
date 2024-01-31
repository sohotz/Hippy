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
import { Descriptor } from '../descriptor/DescriptorBase';
import { NativeGestureDispatcher } from '../NativeGestureDispatcher';

export class ModalProps extends ViewBaseProps {
  animationType?: string
  transparent?: boolean
  supportedOrientations?: string
}

export type ModalDescriptor = Descriptor<"Modal", ModalProps>

@CustomDialog
struct HippyDialog {
  ctx!: NativeRenderContext
  tag!: number
  @BuilderParam renderDescriptors: () => void
  controller: CustomDialogController;

  build() {
    Stack() {
      HippyViewBase({
        ctx: this.ctx,
        tag: this.tag,
      }) {
        this.renderDescriptors()
      }
    }
    .width('100%').height('100%').backgroundColor(0x55ffffff).backdropBlur(5)
    .onClick((e:ClickEvent) => {
      NativeGestureDispatcher.handleClickEvent(this.ctx, this.tag, "requestClose")
    })
  }
}

@Component
export struct HippyModalView {
  ctx!: NativeRenderContext
  tag!: number
  @State descriptor: ModalDescriptor = {} as ModalDescriptor
  @BuilderParam renderDescriptors: () => void
  private dialogController: CustomDialogController = {} as CustomDialogController
  private unregisterDescriptorChangesListener?: () => void = undefined

  aboutToAppear() {
    this.descriptor = this.ctx.descriptorRegistry.getDescriptor<ModalDescriptor>(this.tag);
    this.unregisterDescriptorChangesListener = this.ctx.descriptorRegistry.subscribeToDescriptorChanges(this.tag,
      (newDescriptor) => {
        this.descriptor = (newDescriptor as ModalDescriptor);
      }
    )
    this.dialogController = new CustomDialogController({
      builder:HippyDialog({
        ctx: this.ctx,
        tag: this.tag,
        renderDescriptors: () => {
          this.renderDescriptors()
        }
      }),
      autoCancel: true,
      customStyle: true,
      alignment: DialogAlignment.Center
    });
    this.dialogController.open()
  }

  aboutToDisappear() {
    this.unregisterDescriptorChangesListener?.()
    this.dialogController.close()
  }

  build() {
  }
}
