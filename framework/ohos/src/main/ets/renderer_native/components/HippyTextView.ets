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
import HashMap from '@ohos.util.HashMap';

export class AttributedFragment {
  text?: string
  fontColor?: ColorSegments
  fontWeight?: number
  fontSize?: number
  fontStyle?: string
  textDecorationLine?: number
  textDecorationColor?: ColorSegments
  lineHeight?: number
  backgroundColor?: ColorSegments
  letterSpacing?: number
}

export class PaddingProps {
  top: number = 0
  right: number = 0
  bottom: number = 0
  left: number = 0
}

export class TextProps extends ViewBaseProps {
  text?: string
  color?: number
  fontSize?: number
  lineHeight?: number

  textAlign?: string
  fragments?: AttributedFragment[]
  padding?: PaddingProps
  maximumNumberOfLines?: number
  textAlignVertical?: string
  selectable?: boolean
  ellipsizeMode?: string
}

function stringToAlignment(alignment: string): TextAlign {
  if (alignment === 'center') {
    return TextAlign.Center
  } else if (alignment === 'right') {
    return TextAlign.End
  } else {
    return TextAlign.Start
  }
}

function stringToVerticalAlign(textAlignVertical: string): Alignment | undefined {
  switch (textAlignVertical) {
    case "top":
      return Alignment.TopStart
    case "bottom":
      return Alignment.BottomStart
    case "center":
      return Alignment.Start
    default:
      return undefined
  }
}

function stringToFontStyle(style: string | undefined): FontStyle {
  if (style === 'italic') {
    return FontStyle.Italic
  }
  return FontStyle.Normal
}

function numberToTextDecorationLine(textDecorationLine: number): TextDecorationType {
  switch(textDecorationLine) {
    case 1:
      return TextDecorationType.Underline
    case 2:
      return TextDecorationType.LineThrough
    default :
      return TextDecorationType.None
  }
}

function stringToTextOverflow(ellipsizeMode: string): TextOverflow {
  switch(ellipsizeMode) {
    case "Clip":
      return TextOverflow.Clip
    case "Head":
      return TextOverflow.Ellipsis
    case "Tail":
      return TextOverflow.Ellipsis
    case "Middle":
      return TextOverflow.Ellipsis
    default:
      return TextOverflow.Clip
  }
}

function handledLetterSpacing(letterSpacing: number): number | undefined {
  if(isNaN(letterSpacing)) {
    return undefined
  }
  return letterSpacing
}

function handledMaxLines(maxLines: number): number {
  return (maxLines ?? 0) > 0 ? maxLines : Infinity
}

export type TextDescriptor = Descriptor<"Text", TextProps>

@Component
export struct HippyTextView {
  ctx!: NativeRenderContext
  tag!: number
  @State descriptor: TextDescriptor = {} as TextDescriptor
  private unregisterDescriptorChangesListener?: () => void = undefined

  aboutToAppear() {
    this.descriptor = this.ctx.descriptorRegistry.getDescriptor<TextDescriptor>(this.tag)
    this.unregisterDescriptorChangesListener = this.ctx.descriptorRegistry.subscribeToDescriptorChanges(this.tag,
      (newDescriptor) => {
        this.descriptor = (newDescriptor as TextDescriptor)
      }
    )
  }

  aboutToDisappear() {
    this.unregisterDescriptorChangesListener?.()
  }

  build() {
    HippyViewBase({
      ctx: this.ctx,
      tag: this.tag,
    }) {
      Text() {
        Span(this.descriptor.props.text)
          .fontColor(this.descriptor.props.color)
          //.fontWeight(fragment.fontWeight)
          .fontSize(this.descriptor.props.fontSize)
          //.fontStyle(stringToFontStyle(fragment.fontStyle))
          //.decoration({type: numberToTextDecorationLine(fragment.textDecorationLine), color: convertColorSegmentsToString(fragment.textDecorationColor)})
          .lineHeight(this.descriptor.props.lineHeight)
          //.backgroundColor(convertColorSegmentsToString(fragment.backgroundColor)) // not supported
          //.letterSpacing(handledLetterSpacing(fragment.letterSpacing))

        // ForEach(this.descriptor.props.fragments, (fragment: AttributedFragment) => {
        //   Span(fragment.text)
        //     .fontColor(convertColorSegmentsToString(fragment.fontColor))
        //     .fontWeight(fragment.fontWeight)
        //     .fontSize(fragment.fontSize)
        //     .fontStyle(stringToFontStyle(fragment.fontStyle))
        //     .decoration({type: numberToTextDecorationLine(fragment.textDecorationLine), color: convertColorSegmentsToString(fragment.textDecorationColor)})
        //     .lineHeight(fragment.lineHeight)
        //     .backgroundColor(convertColorSegmentsToString(fragment.backgroundColor)) // not supported
        //     .letterSpacing(handledLetterSpacing(fragment.letterSpacing))
        // })
      }
      .width("100%")
      .height("100%")
      .maxLines(handledMaxLines(this.descriptor.props.maximumNumberOfLines))
      .textOverflow({ overflow: stringToTextOverflow(this.descriptor.props.ellipsizeMode) })
      .textAlign(TextAlign.Center)//stringToAlignment(this.descriptor.props.textAlign))
      .align(Alignment.Start)//stringToVerticalAlign(this.descriptor.props.textAlignVertical))
      .padding(this.descriptor.props.padding)
      .copyOption(this.descriptor.props.selectable ? CopyOptions.LocalDevice : CopyOptions.None)
      .backgroundColor(this.descriptor.props.backgroundColor)
    }
  }
}
