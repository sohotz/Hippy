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
import { HippyAny, HippyRecord, HippyRenderCallback } from '../../../support/common/HippyTypes';
import { HRConvertUtil } from '../../utils/HRConvertUtil';
import HippyRenderBaseView from '../base/HippyRenderBaseView';

// attr
export const kText = "text"
export const kFontSize = "fontSize"
export const kFontWeight = "fontWeight"
export const kTintColor = "tintColor"
export const kColor = "color"
export const kPlaceholderColor = "placeholderColor"
export const kPlaceholder = "placeholder"
export const kKeyboardType = "keyboardType"
export const kReturnKeyType = "returnKeyType"
export const kTextAlign = "textAlign"
export const kMaxTextLength = "maxTextLength"
export const kEditable = "editable"
// event
export const kTextDidChange = "textDidChange"
export const kInputFocus = "inputFocus"
export const kInputBlur = "inputBlur"
export const kKeyboardHeightChange = "keyboardHeightChange"
export const kInputReturn = "inputReturn"
export const kTextLengthBeyondLimit = "textLengthBeyondLimit"
// method
export const kSetText = "setText"
export const kFocus = "focus"
export const kBlur = "blur"
export const kGetCursorIndex = "getCursorIndex"
export const kSetCursorIndex = "setCursorIndex"

export default abstract class HRInputBaseView extends HippyRenderBaseView {
  cssText: string = ""
  cssFontSize: number | null = null
  cssFontWeight: FontWeight | null = null
  cssCursorColor: string | null = null
  cssColor: string | null = null
  cssPlaceholderColor: string | null = null
  cssPlaceholder: string = ""
  cssKeyboardType: InputType | null = null
  cssReturnKeyType: EnterKeyType | null = null
  cssTextAlign: TextAlign | null = null
  cssMaxTextLength: number = -1
  cssEditable: boolean = true
  eventTextDidChange: ((value: string) => void) | null = null
  eventInputFocus: (() => void) | null = null
  eventInputBlur: (() => void) | null = null
  // TODO:
  eventKeyboardHeightChange: HippyRenderCallback | null = null
  eventInputReturn: ((value: EnterKeyType) => void) | null = null
  // TODO:
  eventTextLengthBeyondLimit: HippyRenderCallback | null = null

  setProp(propKey: string, propValue: HippyRenderCallback | HippyAny): boolean {
    switch (propKey) {
      case kText:
        this.cssText = propValue as string
        return true;
      case kFontSize:
        this.setFontSize(propValue as number)
        return true;
      case kFontWeight:
        this.setFontWeight(propValue as string)
        return true;
      case kTintColor:
        this.cssCursorColor = HRConvertUtil.toColor(propValue as string)
        return true;
      case kColor:
        this.cssColor = HRConvertUtil.toColor(propValue as string)
        return true;
      case kPlaceholderColor:
        this.cssPlaceholderColor = HRConvertUtil.toColor(propValue as string)
        return true;
      case kPlaceholder:
        this.cssPlaceholder = propValue as string
        return true;
      case kKeyboardType:
        this.setKeyboardType(propValue as string)
        return true;
      case kReturnKeyType:
        this.setReturnKeyType(propValue as string)
        return true;
      case kTextAlign:
        this.setTextAlign(propValue as string)
        return true;
      case kMaxTextLength:
        this.cssMaxTextLength = propValue as number
        return true;
      case kEditable:
        this.cssEditable = propValue == 1
        return true;
      case kTextDidChange:
        this.setTextDidChange(propValue as HippyRenderCallback)
        return true;
      case kInputFocus:
        this.setInputFocus(propValue as HippyRenderCallback)
        return true;
      case kInputBlur:
        this.setInputBlur(propValue as HippyRenderCallback)
        return true;
      case kKeyboardHeightChange:
        this.eventKeyboardHeightChange = propValue as HippyRenderCallback
        return true;
      case kInputReturn:
        this.setInputReturn(propValue as HippyRenderCallback)
        return true;
      case kTextLengthBeyondLimit:
        this.eventTextLengthBeyondLimit = propValue as HippyRenderCallback
        return true;
      default:
        return super.setProp(propKey, propValue)
    }
  }

  call(method: string, params: Array<HippyAny>, callback: HippyRenderCallback | null): void {
    switch (method) {
      case kSetText:
        const text = params.length > 0 ? params[0] as string : ''
        this.cssText = text
        this.setCursorPosition(text.length)
        break
      case kFocus:
        focusControl.requestFocus(`input_${this.tag}`)
        break
      case kBlur:
        this.clearFocus()
        break
      default:
        super.call(method, params, callback)
        break
    }
  }

  abstract setCursorPosition(position: number)
  abstract clearFocus()

  private setFontSize(propValue: number) {
    this.cssFontSize = propValue > 0 ? propValue : 16
  }

  private setFontWeight(propValue: string) {
    this.cssFontWeight = FontWeight.Normal
    try {
      const num = parseInt(propValue, 10)
      if (num <= 300) {
        this.cssFontWeight = FontWeight.Lighter
      } else if (num >= 500) {
        this.cssFontWeight = FontWeight.Medium
      } else if (num >= 700) {
        this.cssFontWeight = FontWeight.Bold
      } else if (num >= 800) {
        this.cssFontWeight = FontWeight.Bolder
      }
    } catch (ignored) {
    }
  }

  private setKeyboardType(propValue: string) {
    switch (propValue) {
      case 'password':
        this.cssKeyboardType = InputType.Password
        break;
      case 'number':
        this.cssKeyboardType = InputType.Number
        break;
      case 'email':
        this.cssKeyboardType = InputType.Email
        break;
      default:
        this.cssKeyboardType = InputType.Normal
        break;
    }
  }

  private setReturnKeyType(propValue: string) {
    switch (propValue) {
      case 'search':
        this.cssReturnKeyType = EnterKeyType.Search
        break
      case 'send':
        this.cssReturnKeyType = EnterKeyType.Send
        break
      case 'next':
      case 'continue':
        this.cssReturnKeyType = EnterKeyType.Next
        break
      case 'go':
        this.cssReturnKeyType = EnterKeyType.Go
        break
      default:
        this.cssReturnKeyType = EnterKeyType.Done
        break
    }
  }

  private setTextAlign(propValue: string) {
    switch (propValue) {
      case 'center':
        this.cssTextAlign = TextAlign.Center
        break
      case 'right':
        this.cssTextAlign = TextAlign.End
        break
      default:
        this.cssTextAlign = TextAlign.Start
        break
    }
  }

  private setTextDidChange(callback: HippyRenderCallback) {
    this.eventTextDidChange = (value) => {
      this.cssText = value
      callback({ 'text': value } as HippyRecord)
    }
  }

  private setInputFocus(callback: HippyRenderCallback) {
    if (!this.eventTextDidChange) {
      this.eventTextDidChange = (value) => this.cssText = value
    }
    this.eventInputFocus = () => {
      callback({ 'text': this.cssText } as HippyRecord)
    }
  }

  private setInputBlur(callback: HippyRenderCallback) {
    if (!this.eventTextDidChange) {
      this.eventTextDidChange = (value) => this.cssText = value
    }
    this.eventInputFocus = () => {
      callback({ 'text': this.cssText } as HippyRecord)
    }
  }

  private setInputReturn(callback: HippyRenderCallback) {
    if (!this.eventTextDidChange) {
      this.eventTextDidChange = (value) => this.cssText = value
    }
    this.eventInputReturn = (_) => {
      callback({ 'text': this.cssText } as HippyRecord)
    }
  }
}
