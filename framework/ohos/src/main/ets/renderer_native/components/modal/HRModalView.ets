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
import { HRComponent } from '../../HippyRenderView'
import { NativeRenderContext } from '../../NativeRenderContext'
import { HRGestureDispatcher } from '../../uimanager/HRGestureDispatcher'
import { HREventType, HREventUtils } from '../../utils/HREventUtils'
import HippyRenderBaseView, { HippyObervedArray } from '../base/HippyRenderBaseView'

@Observed
export class HRModalView extends HippyRenderBaseView {
  static readonly VIEW_NAME = "HRModalView"

  animationType: string | null = null
  transparent: boolean = true
  supportedOrientations: string | null = null

  handleClose() {
    HRGestureDispatcher.handleClickEvent(this.ctx, this.tag, HREventUtils.EVENT_MODAL_REQUEST_CLOSE)
  }

  constructor(ctx: NativeRenderContext) {
    super(ctx)
  }
}

@CustomDialog
struct HippyDialog {
  controller: CustomDialogController
  @ObjectLink renderView: HRModalView
  @ObjectLink children: HippyObervedArray<HippyRenderBaseView>
  @BuilderParam buildRenderView: ($$: HippyRenderBaseView, parent: HRComponent) => void

  build() {
    Stack() {
      ForEach(this.children, (item: HippyRenderBaseView) => {
        this.buildRenderView(item, null)
      }, (item: HippyRenderBaseView) => item.tag + '')
    }
    .size({width:this.renderView.cssWidth, height: this.renderView.cssHeight})
    .backgroundColor(0x55ffffff).backdropBlur(5)
    .onClick((e:ClickEvent) => {
      this.renderView.handleClose()
    })
  }
}

@Component
export struct HRModal {
  @ObjectLink renderView: HRModalView
  @ObjectLink children: HippyObervedArray<HippyRenderBaseView>
  @BuilderParam buildRenderView: ($$: HippyRenderBaseView, parent: HRComponent) => void
  private dialogController: CustomDialogController = {} as CustomDialogController

  aboutToAppear() {
    this.dialogController = new CustomDialogController({
      builder: HippyDialog({
        renderView: this.renderView,
        children: this.children,
        buildRenderView: this.buildRenderView
      }),
      autoCancel: true,
      customStyle: true,
      alignment: DialogAlignment.Center
    });
    this.dialogController.open()
  }

  aboutToDisappear() {
    this.dialogController.close()
  }

  build() {

  }
}
