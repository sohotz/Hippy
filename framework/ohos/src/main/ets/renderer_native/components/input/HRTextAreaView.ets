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
import HRInputBaseView from './HRInputBaseView';
import window from '@ohos.window';
import { Callback } from '@ohos.base';
import { HippyRecord } from '../../../support/common/HippyTypes';
import HippyRenderBaseView from '../base/HippyRenderBaseView';

@Observed
export class HRTextAreaView extends HRInputBaseView {
  static readonly VIEW_NAME = 'HRTextAreaView'
  readonly controller: TextAreaController = new TextAreaController()

  constructor(ctx: NativeRenderContext) {
    super(ctx)
    this.cssBackgroundColor = 0
    this.cssBorder = { radius: 0 }
  }

  setCursorPosition(position: number): void {
    this.controller.caretPosition(position)
  }

  clearFocus(): void {
    this.controller.stopEditing()
  }
}

@Component
export struct HRTextArea {
  @ObjectLink renderView: HRTextAreaView
  windowPromise?: Promise<window.Window> = undefined
  keyboardListener: Callback<number> = (height) => {
    const callback = this.renderView.eventKeyboardHeightChange
    if (callback) {
      const param: HippyRecord = {
        'height': px2vp(height),
        'duration': 120,
      };
      callback(param)
    }
  }

  aboutToAppear() {
    this.windowPromise = window.getLastWindow(getContext(this)).then((data) => {
      data.on('keyboardHeightChange', this.keyboardListener)
      return data
    })
  }

  aboutToDisappear() {
    this.windowPromise!.then((data) => {
      data.off('keyboardHeightChange', this.keyboardListener)
    })
    this.windowPromise = undefined
  }

  build() {
    TextArea({
      placeholder: this.renderView.cssPlaceholder,
      text: this.renderView.cssText,
      controller: this.renderView.controller,
    }).applyRenderViewBaseAttr(this.renderView)
      .applyRenderViewTextAttr(this.renderView)
  }
}

@Extend(TextArea)
function applyRenderViewBaseAttr($$: HippyRenderBaseView) {
  .backgroundColor($$.cssBackgroundColor)
  .position({ x: $$.cssPositionX, y: $$.cssPositionY })
  .size({ width: $$.cssWidth, height: $$.cssHeight })
  .opacity($$.cssOpacity)
  //.clip($$.cssOverflow)
  .visibility(($$ as HippyRenderBaseView).cssVisibility) // must add as, otherwise the compiler has error
  .touchable($$.cssTouchEnable) // TODO: use new api
  .zIndex($$.cssZIndex)
  .border($$.cssBorder)
  .rotate($$.cssRotate)
  .scale($$.cssScale)
  .translate($$.cssTranslate)
  .onClick($$.eventClick)
}

@Extend(TextArea)
function applyRenderViewTextAttr($$: HRTextAreaView) {
  .key(`input_${$$.tag}`)
  .padding(0)
  .fontSize($$.cssFontSize)
  .fontWeight($$.cssFontWeight)
  .caretColor($$.cssCursorColor)
  .fontColor($$.cssColor)
  .placeholderColor($$.cssPlaceholderColor)
  .textAlign($$.cssTextAlign)
  .maxLength($$.cssMaxTextLength)
  .enabled($$.cssEditable)
  .onChange($$.eventTextDidChange)
  .onFocus($$.eventInputFocus)
  .onBlur($$.eventInputBlur)
}
