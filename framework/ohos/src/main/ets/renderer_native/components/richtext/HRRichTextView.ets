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
import { HippyAny, HippyMap } from '../../../support/common/HippyTypes'
import { NativeRenderContext } from '../../NativeRenderContext'
import HippyRenderBaseView from '../base/HippyRenderBaseView'
import { HRComponent } from '../../HippyRenderView'
import { LogUtils } from '../../../support/utils/LogUtils'
import { HRImageView } from '../image/HRImageView'

@Observed
export class HRRichTextView extends HippyRenderBaseView {
  private readonly TAG = "HRRichTextView"
  maxLines: number = 10000000

  // TODO(hot):
  text: string = ""
  fontColor: number | null = null
  fontSize: number | null = null
  lineHeight: number | null = null
  shadowColor: number = 0
  shadowRadius: number = 0
  shadowOffX: number = 0
  shadowOffY: number = 0
  fontStyle: FontStyle = FontStyle.Normal
  fontWeight: number | FontWeight | string = FontWeight.Normal
  fontFamily: string | null = null
  decorationType: TextDecorationType = TextDecorationType.None
  decorationColor: number = 0
  letterSpacing: number = 0
  textAlign: TextAlign = TextAlign.Start
  wordBreak: WordBreak = WordBreak.BREAK_WORD

  constructor(ctx: NativeRenderContext) {
    super(ctx)
  }

  setProp(propKey: string, propValue: HippyAny): boolean {
    if (propKey == "text") {
      if (propValue) {
        if (propValue.constructor.name == "String") {
          this.text = propValue as string
        } else {
          LogUtils.e(this.TAG, propKey);
        }
      }
      return true
    } else if (propKey == "color") {
      if (propValue) {
        if (propValue.constructor.name == "Number") {
          this.fontColor = propValue as number
        } else {
          LogUtils.e(this.TAG, propKey);
        }
      }
      return true
    } else if (propKey == "enableScale") { //todo
      return true;
    } else if (propKey == "fontFamily") { //todo 不生效
      if (propValue) {
        if (propValue.constructor.name == "String") {
          this.fontFamily = propValue as string;
        } else {
          LogUtils.e(this.TAG, propKey);
        }
      }
      return true;
    } else if (propKey == "fontSize") {
      if (propValue) {
        if (propValue.constructor.name == "Number") {
          this.fontSize = propValue as number
        } else {
          LogUtils.e(this.TAG, propKey);
        }
      }
      return true
    } else if (propKey == "fontStyle") {
      if (propValue) {
        if (propValue.constructor.name == "String") {
          if (propValue as string == 'italic') {
            this.fontStyle = FontStyle.Italic;
          }
          else {
            this.fontStyle = FontStyle.Normal;
          }
        } else {
          LogUtils.e(this.TAG, propKey);
        }
      }
      return true;
    } else if (propKey == "fontWeight") {
      if (propValue) {
        if (propValue.constructor.name == "String") {
          let t = propValue as string;
          if (t == "normal") {
            this.fontWeight = FontWeight.Normal;
          } else if (t == "bold") {
            this.fontWeight = FontWeight.Bold;
          } else {
            let n = parseInt(t);
            if (isNaN(n)) {
              this.fontWeight = FontWeight.Normal;
            } else {
              this.fontWeight = Math.min(Math.max(1, n), 1000);
            }
          }
        } else if (propValue.constructor.name == "Number") {
          this.fontWeight = propValue as number;
        } else {
          LogUtils.e(this.TAG, propKey);
        }
      }
      return true;
    } else if (propKey == "letterSpacing") {
      if (propValue) {
        if (propValue.constructor.name == "Number") {
          this.letterSpacing = propValue as number;
        } else {
          LogUtils.e(this.TAG, propKey);
        }
      }
      return true;
    } else if (propKey == "lineHeight") {
      if (propValue) {
        if (propValue.constructor.name == "Number") {
          this.lineHeight = propValue as number;
        } else {
          LogUtils.e(this.TAG, propKey);
        }
      }
      return true
    } else if (propKey == "lineSpacingExtra") { //todo
      return true;
    } else if (propKey == "lineSpacingMultiplier") { //todo
      return true;
    } else if (propKey == "numberOfLines") {
      if (propValue) {
        if (propValue.constructor.name == "Number") {
          this.maxLines = propValue as number
          if (this.maxLines <= 0) {
            this.maxLines = 10000000
          }
        } else {
          LogUtils.e(this.TAG, propKey);
        }
      }
      return true
    } else if (propKey == "textAlign") {
      if (propValue) {
        if (propValue.constructor.name == "String") {
          let t = propValue as string;
          if (t == "justify") {
            this.textAlign = TextAlign.JUSTIFY;
          } else if (t == "right") {
            this.textAlign = TextAlign.End;
          } else if (t == "center") {
            this.textAlign = TextAlign.Center;
          } else {
            this.textAlign = TextAlign.Start;
          }
        } else {
          LogUtils.e(this.TAG, propKey);
        }
      }
      return true;
    } else if (propKey == "textDecorationLine") { //安卓可以同时显示下划线和删除线
      if (propValue) {
        if (propValue.constructor.name == "String") {
          let t = propValue as string;
          this.decorationType = TextDecorationType.None;
          t.split(" ").forEach(s => {
            if (s == 'underline') {
              this.decorationType = TextDecorationType.Underline;
            } else if (s == 'line-through') {
              this.decorationType = TextDecorationType.LineThrough;
            }
          });
        } else {
          LogUtils.e(this.TAG, propKey);
        }
      }
      return true;
    } else if (propKey == "textShadowColor") {
      if (propValue) {
        if (propValue.constructor.name == "Number") {
          this.shadowColor = propValue as number;
        } else {
          LogUtils.e(this.TAG, propKey);
        }
      }
      return true;
    } else if (propKey == "textShadowOffset") {
      if (propValue) {
        if (propValue.constructor.name == "Map") {
          let t = propValue as HippyMap;
          this.shadowOffX = t.get('width') as number;
          this.shadowOffY = t.get('height') as number;
        } else {
          LogUtils.e(this.TAG, propKey + "" + propValue.constructor.name);
        }
      }
      return true;
    } else if (propKey == "textShadowRadius") {
      if (propValue) {
        if (propValue.constructor.name == "Number") {
          this.shadowRadius = propValue as number;
        } else {
          LogUtils.e(this.TAG, propKey);
        }
      }
      return true;
    } else if (propKey == "textDecorationColor") { //下划线/删除线的颜色，安卓无，next有
      if (propValue) {
        if (propValue.constructor.name == "Number") {
          this.decorationColor = propValue as number;
        } else {
          LogUtils.e(this.TAG, propKey);
        }
      }
      return true;
    } else if (propKey == "ellipsizeMode") { //todo 文本超长裁剪时，省略号在前/中/后，安卓有，next无
      return true;
    } else if (propKey == "textDecorationStyle") { //todo 下划线/删除线的类型：DOUBLE，DOTTED，DASHED，SOLID，安卓有，next无
      return true;
    } else if (propKey == "breakStrategy") { //todo 文本断行策略，不完全匹配
      if (propValue) {
        if (propValue.constructor.name == "String") {
          let t = propValue as string;
          if (t == "high_quality") {
            this.wordBreak = WordBreak.BREAK_ALL;
          } else if (t == "balanced") {
            this.wordBreak = WordBreak.BREAK_WORD;
          } else { //t == "simple"
            this.wordBreak = WordBreak.BREAK_WORD;//如果使用WordBreak.NORMAL，超过行宽的连续字符不会换行(被裁剪)
          }
        } else {
          LogUtils.e(this.TAG, propKey);
        }
      }
      return true;
    }
    return super.setProp(propKey, propValue)
  }
}

@Component
export struct HRRichText {
  @ObjectLink renderView: HRRichTextView
  @BuilderParam buildRenderView: ($$: HippyRenderBaseView, parent: HRComponent) => void

  @Builder TextSpan(txt: HRRichTextView) {
    Span(txt.text)
      .fontColor(txt.fontColor)
      .fontSize(txt.fontSize)
      .lineHeight(txt.lineHeight)
      .textShadow({
        radius: txt.shadowRadius,
        color: txt.shadowColor,
        offsetX: txt.shadowOffX,
        offsetY: txt.shadowOffY
      })
      .fontStyle(txt.fontStyle)
      .fontWeight(txt.fontWeight)
      .fontFamily(txt.fontFamily)
      .decoration({
        type: txt.decorationType,
        color: txt.decorationColor
      })
      .letterSpacing(txt.letterSpacing)
      .applyRenderViewBaseAttr_TextSpan(txt)
  }

  @Builder ImageSpan(img: HRImageView) {
    ImageSpan(img.getImage())
      //todo image attrs
      .applyRenderViewBaseAttr_ImageSpan(img)
  }
  build() {
    Text(this.renderView.text) {
      if (this.renderView.children.length > 0) {
        ForEach(this.renderView.children, (item: HippyRenderBaseView) => {
          if (item.viewType == "Text") {
            this.TextSpan(item as HRRichTextView)
          }
          if (item.viewType == "Image") {
            this.ImageSpan(item as HRImageView)
          }
        }, (item: HippyRenderBaseView) => item.tag + '')
      }
    }
    .fontColor(this.renderView.fontColor)
    .fontSize(this.renderView.fontSize)
    .lineHeight(this.renderView.lineHeight)
    .textOverflow({ overflow: TextOverflow.Ellipsis })
    .maxLines(this.renderView.maxLines)
    .textShadow({
      radius: this.renderView.shadowRadius,
      color: this.renderView.shadowColor,
      offsetX: this.renderView.shadowOffX,
      offsetY: this.renderView.shadowOffY
    })
    .fontStyle(this.renderView.fontStyle)
    .fontWeight(this.renderView.fontWeight)
    .fontFamily(this.renderView.fontFamily)
    .decoration({
      type: this.renderView.decorationType,
      color: this.renderView.decorationColor
    })
    .letterSpacing(this.renderView.letterSpacing)
    .textAlign(this.renderView.textAlign)
    .wordBreak(this.renderView.wordBreak)
    .applyRenderViewBaseAttr(this.renderView)
    // .alignRules(?)
    // .align(Alignment.Center)
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

@Extend(Span)
function applyRenderViewBaseAttr_TextSpan($$: HippyRenderBaseView) {
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

@Extend(ImageSpan)
function applyRenderViewBaseAttr_ImageSpan($$: HippyRenderBaseView) {
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
