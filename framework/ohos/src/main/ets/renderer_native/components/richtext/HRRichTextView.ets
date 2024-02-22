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
import { NativeRenderContext } from '../../NativeRenderContext'
import HippyRenderBaseView, { HippyObservedArray } from '../base/HippyRenderBaseView'
import { HRRichTextShadow, MAX_LINES } from './HRRichTextShadow'
import { HRTextSpan } from './HRTextSpan'

@Observed
export class HRTextAttr {
  spans: HippyObservedArray<HRTextSpan> = new HippyObservedArray()
}

@Observed
export class HRRichTextView extends HippyRenderBaseView {
  textAttr: HRTextAttr = new HRTextAttr()
  maxLines: number = MAX_LINES

  // TODO(hot):
  text: string = ""
  fontColor: number | null = null
  fontSize: number | null = null
  lineHeight: number | null = null

  constructor(ctx: NativeRenderContext) {
    super(ctx)
  }

  setProp(propKey: string, propValue: HippyAny): boolean {
    if (propKey == "numberOfLines") {
      this.maxLines = propValue as number
      if (this.maxLines <= 0) {
        this.maxLines = MAX_LINES
      }
      return true
    } else if (propKey == "text") {
      if (propValue) {
        this.text = propValue as string
      }
      return true
    } else if (propKey == "color") {
      if (propValue) {
        this.fontColor = propValue as number
      }
      return true
    } else if (propKey == "fontSize") {
      if (propValue) {
        this.fontSize = propValue as number
      }
      return true
    } else if (propKey == "lineHeight") {
      if (propValue) {
        this.lineHeight = propValue as number
      }
      return true
    }
    return super.setProp(propKey, propValue)
  }

  setShadow(textShadow: HRRichTextShadow): void {
    this.textAttr.spans.splice(0, this.textAttr.spans.length)
    textShadow.spans.forEach((item) => {
      this.textAttr.spans.push(item)
    })
  }
}

@Component
export struct HRRichText {
  @ObjectLink renderView: HRRichTextView
  @ObjectLink spans: HippyObservedArray<HRTextSpan>

  build() {
    Text() {
      Span(this.renderView.text)
        .fontColor(this.renderView.fontColor)
        .fontSize(this.renderView.fontSize)
        .lineHeight(this.renderView.lineHeight)
      // ForEach(this.spans, (span: HRTextSpan) => {
      //   Span(span.text)
      //     .fontSize(span.fontSize)
      //     .fontWeight(span.fontWeight)
      //     .fontColor(span.fontColor)
      //     .fontStyle(span.fontStyle)
      //     .fontFamily(span.fontFamily)
      //     .letterSpacing(span.letterSpacing)
      //     .lineHeight(span.lineHeight)
      //     .decoration({type:span.textDecoration})
      //     .align(span.textAlign)
      // })
    }
    .textOverflow({overflow: TextOverflow.Ellipsis})
    .maxLines(this.renderView.maxLines)
    .applyRenderViewBaseAttr(this.renderView)
  }

}

// base props for all components
@Extend(Text)
function applyRenderViewBaseAttr($$: HippyRenderBaseView) {
  .backgroundColor($$.cssBackgroundColor)
  .position({x: $$.cssPositionX, y: $$.cssPositionY})
  .size({width:$$.cssWidth, height: $$.cssHeight})
  .opacity($$.cssOpacity)
  .clip($$.cssOverflow)
  .visibility(($$ as HippyRenderBaseView).cssVisibility) // must add as, otherwise the compiler has error
  .zIndex($$.cssZIndex)
  .border($$.cssBorder)
  .borderWidth(0.5) // TODO:
  .rotate($$.cssRotate)
  .scale($$.cssScale)
  .translate($$.cssTranslate)
  .onClick($$.eventClick)
}
