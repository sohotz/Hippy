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
import { HippyAny } from '../../../support/common/HippyTypes'
import { HRConvertUtil } from '../../utils/HRConvertUtil'

export class HRTextSpan {
  text: string = ""
  fontSize: number | null = null
  fontWeight: FontWeight | null = null
  fontColor: string | null = null
  fontStyle: FontStyle | null = null
  fontFamily: string | null = null
  letterSpacing: number | null = null
  lineHeight: number | null = null
  textDecoration: TextDecorationType | null = null
  textAlign: Alignment | null = null

  public static parseFromProps(props: Record<string, HippyAny>, mainSpan: HRTextSpan): HRTextSpan {
    const span = new HRTextSpan()
    const text = props["value"] ? props["value"] : props["text"]
    if (text) {
      span.text = text as string
    }
    const fontSize = props["fontSize"]
    if (fontSize) {
      span.fontSize = fontSize as number
    } else {
      span.fontSize = mainSpan.fontSize
    }
    const fontWeight = props["fontWeight"]
    if (fontWeight) {
      span.fontWeight = parseInt(fontWeight as string) as number // 400 500
    } else {
      span.fontWeight = mainSpan.fontWeight
    }
    const color = props["color"]
    if (color) {
      span.fontColor = HRConvertUtil.toColor(color as string)
    } else {
      span.fontColor = mainSpan.fontColor
    }
    const fontStyle = props["fontStyle"]
    if (fontStyle) {
      span.fontStyle = HRConvertUtil.toFontStyle(fontStyle as string)
    } else {
      span.fontStyle = mainSpan.fontStyle
    }
    const letterSpacing = props["letterSpacing"]
    if (letterSpacing) {
      span.letterSpacing = letterSpacing as number
    } else {
      span.letterSpacing = mainSpan.letterSpacing
    }
    const lineHeight = props["lineHeight"]
    if (lineHeight) {
      span.lineHeight = lineHeight as number
    } else {
      span.lineHeight = mainSpan.lineHeight
    }
    const textDecoration = props["textDecoration"]
    if (textDecoration) {
      span.textDecoration = HRConvertUtil.toTextDecorationType(textDecoration as string)
    } else {
      span.textDecoration = mainSpan.textDecoration
    }
    const textAlign = props["textAlign"]
    if (textAlign) {
      span.textAlign = HRConvertUtil.toTextAlign(textAlign as string)
    } else {
      span.textAlign = mainSpan.textAlign
    }
    return span
  }

}
