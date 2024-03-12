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
import HippyRenderBaseView, { HippyObservedArray } from '../base/HippyRenderBaseView'
import { LogUtils } from '../../../support/utils/LogUtils'
import { HippyAny, HippyRenderCallback } from '../../../support/common/HippyTypes';
import { HRValueUtils } from '../../utils/HRValueUtils';
const TAG: string = "HRModal";

@Observed
export class HRModalView extends HippyRenderBaseView {
  static readonly VIEW_NAME = "HRModalView"

  animationType: string | null = null
  transparent: boolean = true
  supportedOrientations: string | null = null
  immersionStatusBar: boolean = false;
  darkStatusBarText: boolean = false;

  constructor(ctx: NativeRenderContext) {
    super(ctx)
  }
  setProp(propKey: string, propValue: HippyAny | HippyRenderCallback): boolean {
    LogUtils.i(TAG, `modalview propkey: ${propKey}, value: ${propValue}`)
    switch (propKey) {
      case "transparent":
        this.transparent = HRValueUtils.getBoolean(propValue as HippyAny,true);
        break;
      case "immersionStatusBar":
        this.immersionStatusBar = HRValueUtils.getBoolean(propValue as HippyAny,true);
        break;
      case "supportedOrientations":
        this.supportedOrientations = HRValueUtils.getString(propValue as HippyAny);
        break;
      case "animationType":
        this.animationType = HRValueUtils.getString(propValue as HippyAny);
        break;
      case "darkStatusBarText":
        this.darkStatusBarText = HRValueUtils.getBoolean(propValue as HippyAny,true);
        break;
      default:
        break
    }
    return super.setProp(propKey, propValue as HippyAny)
  }
}

@CustomDialog
struct HippyDialog {
  controller: CustomDialogController
  @ObjectLink renderView: HRModalView
  @ObjectLink children: HippyObservedArray<HippyRenderBaseView>
  @BuilderParam buildRenderView: ($$: HippyRenderBaseView, parent: HRComponent) => void

  build() {
    Stack() {
      ForEach(this.children, (item: HippyRenderBaseView) => {
        this.buildRenderView(item, null)
      }, (item: HippyRenderBaseView) => item.tag + '')
    }
    .applyRenderViewBaseAttr(this.renderView)
    .backgroundColor(this.renderView.transparent ? Color.Transparent : this.renderView.cssBackgroundColor)
  }
}

@Component
export struct HRModal {
  @ObjectLink renderView: HRModalView
  @ObjectLink children: HippyObservedArray<HippyRenderBaseView>
  @BuilderParam buildRenderView: ($$: HippyRenderBaseView, parent: HRComponent) => void
  @BuilderParam buildCustomRenderView: ($$: HippyRenderBaseView) => void | null
  private dialogController: CustomDialogController = {} as CustomDialogController

  handleCancel(): void {
    LogUtils.d(TAG, `handleCancel EVENT_MODAL_REQUEST_CLOSE`)
    HREventUtils.sendComponentEvent(this.renderView.ctx, this.renderView.tag, HREventUtils.EVENT_MODAL_REQUEST_CLOSE, null)
  }

  aboutToAppear() {
    this.dialogController = new CustomDialogController({
      builder: HippyDialog({
        renderView: this.renderView,
        children: this.children,
        buildRenderView: this.buildRenderView
      }),
      autoCancel: true,
      customStyle: true,
      alignment: DialogAlignment.Center,
      openAnimation: { duration: 0 },
      closeAnimation: { duration: 0 },
      cancel: this.handleCancel,
    });
    this.dialogController.open();
    HREventUtils.sendComponentEvent(this.renderView.ctx, this.renderView.tag, HREventUtils.EVENT_MODAL_SHOW, null)
  }

  aboutToDisappear() {
    LogUtils.d(TAG, `aboutToDisappear`)
    this.dialogController.close()
  }

  build() {

  }
}

// base props for all components
@Extend(Stack)
function applyRenderViewBaseAttr($$: HippyRenderBaseView) {
  //.backgroundColor($$.cssBackgroundColor)
  //.position({ x: $$.cssPositionX, y: $$.cssPositionY })
  //.size({ width: $$.cssWidth, height: $$.cssHeight })
  .opacity($$.cssOpacity)
  .clip($$.cssOverflow)
  .visibility(($$ as HippyRenderBaseView).cssVisibility) // must add as, otherwise the compiler has error
  .zIndex($$.cssZIndex)
  .border($$.cssBorder)
  .rotate($$.cssRotate)
  .scale($$.cssScale)
  .translate($$.cssTranslate)
  //.onClick($$.eventClick)
}
