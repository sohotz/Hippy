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
import measure from '@ohos.measure';
import { HippyAny } from '../../../support/common/HippyTypes';
import { HRSize } from '../../utils/HRTypes';
import { HRTextSpan } from './HRTextSpan';

export const MAX_LINES = 10000000

export class HRRichTextShadow {
  private props: Record<string, HippyAny> = {}
  private maxLines: number = MAX_LINES
  private mainSpan: HRTextSpan = new HRTextSpan()
  public spans: HRTextSpan[] = []

  constructor() {
    this.fullDefaultAttr(this.mainSpan)
  }

  setProp(propKey: string, propValue: HippyAny): void {
    if (propKey == "numberOfLines") {
      this.maxLines = propValue as number
      if (this.maxLines <= 0) {
        this.maxLines = MAX_LINES
      }
      return
    }
    this.props[propKey] = propValue
  }

  call(methodName: string, params: Array<HippyAny>): HippyAny {
    return null
  }

  calculateRenderViewSize(constraintWidth: number, constraintHeight: number): HRSize {
    let cHeight: number = constraintHeight > 0 ? constraintHeight : 100000000
    this.mainSpan = HRTextSpan.parseFromProps(this.props, this.mainSpan)
    this.spans = this.buildSpans()
    const measureSpan = this.generateMeasureSpan()
    let size: SizeOptions = measure.measureTextSize({
      textContent: measureSpan.text,
      fontSize: measureSpan.fontSize,
      fontWeight: measureSpan.fontWeight,
      maxLines: this.maxLines,
      fontStyle: measureSpan.fontStyle,
      letterSpacing: measureSpan.letterSpacing,
      fontFamily: measureSpan.fontFamily,
      lineHeight: measureSpan.lineHeight ? measureSpan.lineHeight : undefined,
      constraintWidth: constraintWidth,
    })

    if (size == null) {
      return new HRSize(constraintWidth,  (measureSpan.fontSize ? measureSpan.fontSize : 15))
    }

    if (px2vp(size.height as number) < (measureSpan.fontSize ? measureSpan.fontSize : 15) * 2) { // measure real width
      let realWidth = measure.measureText({
        textContent:measureSpan.text,
        fontSize: measureSpan.fontSize,
        fontWeight: measureSpan.fontWeight,
        fontStyle: measureSpan.fontStyle,
        letterSpacing: measureSpan.letterSpacing,
        fontFamily: measureSpan.fontFamily,
        lineHeight: measureSpan.lineHeight,
      })
      size.width = realWidth
    }

    let measureWidth = size.width as number ? size.width as number : 0
    measureWidth = px2vp(measureWidth)
    measureWidth = measureWidth > constraintWidth ? constraintWidth : measureWidth
    let measureHeight = size.height as number ? size.height as number : 0
    measureHeight = px2vp(measureHeight)
    measureHeight = measureHeight > cHeight? cHeight : measureHeight
    return new HRSize(measureWidth, measureHeight)
  }

  private generateMeasureSpan(): HRTextSpan {
    let textContent = ""
    let measureSpan: HRTextSpan = new HRTextSpan()
    for (let index = 0; index < this.spans.length; index++) {
      let span = this.spans[index]
      if (!measureSpan.fontSize) {
        measureSpan.fontSize = span.fontSize
      }
      if (!measureSpan.fontWeight) {
        measureSpan.fontWeight = span.fontWeight
      }
      if (!measureSpan.fontStyle) {
        measureSpan.fontStyle = span.fontStyle
      }
      if (!measureSpan.fontFamily) {
        measureSpan.fontFamily = span.fontFamily
      }
      if (!measureSpan.letterSpacing) {
        measureSpan.letterSpacing = span.letterSpacing
      }
      if (!measureSpan.lineHeight) {
        measureSpan.lineHeight = span.lineHeight
      }
      if (!measureSpan.textAlign) {
        measureSpan.textAlign = span.textAlign
      }
      if (!measureSpan.textDecoration) {
        measureSpan.textDecoration = span.textDecoration
      }
      textContent += span.text
    }
    measureSpan.text = textContent
    return measureSpan
  }

  private fullDefaultAttr(span: HRTextSpan) {
    if (!span.fontColor) {
      span.fontColor = "rgba(0, 0, 0, 1)"
    }
    if (!span.fontSize) {
      span.fontSize = 15
    }
  }

  private buildSpans(): HRTextSpan[] {
    const spans: HRTextSpan[] = []
    let values = this.props["values"]
    let spanProps: Record<string, HippyAny>[] = []
    if (values) {
      spanProps = JSON.parse(values as string) as Record<string, HippyAny>[]
      spanProps.forEach((item: Record<string, HippyAny>) => {
        spans.push(HRTextSpan.parseFromProps(item, this.mainSpan))
      })
    } else {
      spans.push(this.mainSpan)
    }
    return spans
  }
}
