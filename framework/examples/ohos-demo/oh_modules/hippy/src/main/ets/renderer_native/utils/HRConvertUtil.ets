/*
 * Tencent is pleased to support the open source community by making
 * Hippy available.
 *
 * Copyright (C) 2022 THL A29 Limited, a Tencent company.
 * All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License")
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
import display from '@ohos.display'
import window from '@ohos.window'
import { HRSize, HRTransform } from './HRTypes'

export class HRConvertUtil {
  public static toBorderRadius(cssString: string):  BorderRadiuses | null {
    if (cssString.length) {
      let splits = cssString.split(",")
      if (splits.length == 4) {
        return {topLeft: parseFloat( splits[0]), topRight: parseFloat(splits[1]), bottomLeft: parseFloat(splits[2]), bottomRight: parseFloat(splits[3])}
      }
    }
    return null
  }

  public static toBorder(cssString: string, borderRadius: BorderRadiuses | null): BorderOptions  | undefined {
    if (cssString) {
       let splits = cssString.split(" ")
      if (splits.length == 3) {
        return {width: parseFloat(splits[0]), color: HRConvertUtil.toColor(splits[2]) , radius: borderRadius ? borderRadius: undefined , style: HRConvertUtil.toBorderStyle(splits[1])}
      }
    }
    return undefined
  }

  public static toTransform(cssString: string): HRTransform {
    //"$rotate|$scale|$translate|$anchor|$skew"
    let transform = new HRTransform()
    let splits = cssString.split("|")
    transform.anchor = HRConvertUtil.toTransformAnchor(splits[3])
    transform.rotate = HRConvertUtil.toTransformRotate(splits[0], transform.anchor )
    transform.scale = HRConvertUtil.toTransformScale(splits[1], transform.anchor)
    transform.translate = HRConvertUtil.toTransformTranslate(splits[2])
    return transform
  }

  public static toTextDecorationType(cssString: string): TextDecorationType {
    if (cssString == "underline") {
      return TextDecorationType.Underline
    }
    if (cssString == "line-through") {
      return TextDecorationType.LineThrough
    }
    return TextDecorationType.None
  }

  public static toFontStyle(cssString: string): FontStyle {
    if (cssString == "normal") {
      return FontStyle.Normal
    }
    if (cssString == "italic") {
      return FontStyle.Italic
    }
    return FontStyle.Normal
  }

  public static toTextAlign(cssString: string): Alignment {
    if (cssString == "left") {
      return Alignment.Start
    }
    if (cssString == "center") {
      return Alignment.Center
    }
    if (cssString == "right") {
      return Alignment.End
    }
    return Alignment.Start
  }

  public static toImageFit(cssString: string): ImageFit {
    if (cssString == 'contain') {
      return ImageFit.Contain
    }
    if (cssString == 'stretch') {
      return ImageFit.Fill
    }
    return ImageFit.Cover
  }

  private static toTransformRotate(cssString: string, anchor: Position | null): RotateOptions | null {
    let anchorX = (anchor ? anchor.x as number : 0.5) * 100.0
    let anchorY = (anchor ? anchor.y as number : 0.5) * 100.0
    return {angle: parseFloat(cssString), centerX: (anchorX) + "%", centerY: anchorY + "%" }
  }

  private static toTransformScale(cssString: string, anchor: Position | null): ScaleOptions | null {
    let anchors = cssString.split(" ")
    let anchorX = (anchor ? anchor.x as number : 0.5) * 100.0
    let anchorY = (anchor ? anchor.y as number : 0.5) * 100.0
    return {x: parseFloat(anchors[0]), y: parseFloat(anchors[1]), centerX: (anchorX) + "%", centerY: anchorY + "%" }
  }

  private static toTransformTranslate(cssString: string): TranslateOptions | null {
    let anchors = cssString.split(" ")
    return {x: parseFloat(anchors[0]) * 100 + "%", y: parseFloat(anchors[1]) * 100 + "%"}
  }

  private static toTransformAnchor(cssString: string): Position | null {
    let anchors = cssString.split(" ")
    return {x: parseFloat(anchors[0]), y: parseFloat(anchors[1])}
  }

  private static toBorderStyle(cssString: string): BorderStyle | undefined {
    if (cssString == 'solid') {
      return BorderStyle.Solid
    } else if (cssString == 'dotted') {
      return BorderStyle.Dotted
    } else if (cssString == 'dashed') {
      return BorderStyle.Dashed
    }
    return undefined
  }

  public static toColor(colorString: string) {
    const rgba = parseInt(colorString);
    const a = ((rgba >> 24) & 0xFF) / 255.0;
    const r = ((rgba >> 16) & 0xFF);
    const g = ((rgba >> 8) & 0xFF);
    const b = ((rgba) & 0xFF);
    return `rgba(${r},${g},${b},${a})`
  }

  public static screenSize(): HRSize {
    let width = 0
    let height = 0

    try {
      const displayRes = display.getDefaultDisplaySync()
      width = displayRes.width
      height = displayRes.height
    } catch (e) {
    }
    return new HRSize(px2vp(width) , px2vp(height))
  }

  public static statusBarHeight(avoidArea: window.AvoidArea | null) {
    let defaultHeight = 20
    if (avoidArea) {
      let height = px2vp(avoidArea.topRect.height)
      return height ? height : defaultHeight
    }
    return defaultHeight
  }

}
