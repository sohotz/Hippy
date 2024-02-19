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
import { HippyAny,
  HippyRecord,
  HippyRenderCallback,
  HippyTouchParamType,
  HippyValue } from '../../../support/common/HippyTypes';
import { LogUtils } from '../../../support/utils/LogUtils';
import { HRNodeProps } from '../../dom_node/HRNodeProps';
import { NativeRenderContext } from '../../NativeRenderContext';
import { HRGestureDispatcher } from '../../uimanager/HRGestureDispatcher';
import { HRConvertUtil } from '../../utils/HRConvertUtil';
import { HRRect } from '../../utils/HRTypes';

interface LinearGradient {
  angle?: number | string
  direction?: GradientDirection
  colors: Array<[ResourceColor, number]>
  repeating?: boolean
}

function touchEventToParamRecord(event: TouchEvent) : HippyTouchParamType {
  let touches = event.touches.map((value)=>{
    return {
      'x': value.x,
      'y': value.y,
      'pageX': value.windowX,
      'pageY': value.windowY,
    } as HippyRecord
  })
  let param : HippyTouchParamType = {
    'x': touches[0].x,
    'y': touches[0].y,
    'pageX': touches[0].pageX,
    'pageY': touches[0].pageY,
    'touches': touches,
  }
  return param
}

@Observed
export class HippyObservedArray<T> extends Array<T> {
}

export default abstract class HippyRenderBaseView {
  ctx: NativeRenderContext
  tag: number = -1
  viewType: string = ""
  children: HippyObservedArray<HippyRenderBaseView> = new HippyObservedArray()
  parentView: HippyRenderBaseView | null = null

  cssBackgroundColor: number | null = null
  cssVisibility: Visibility| null = null
  cssOpacity: number | null = null
  cssPositionX: number = 0
  cssPositionY: number = 0
  cssWidth: number = 0
  cssHeight: number = 0
  cssOverflow: boolean | null = null
  cssTouchEnable: boolean | null = null
  cssZIndex: number | null = null
  cssBorderRadius:  BorderRadiuses | null = null
  cssBorder: BorderOptions  | undefined = undefined
  cssRotate: RotateOptions | null = null
  cssScale: ScaleOptions | null = null
  cssTranslate: TranslateOptions | null = null
  cssLinearGradient: LinearGradient | null = null
  eventClick: ((event: ClickEvent) => void) | null = null
  eventDoubleClick: ((event: GestureEvent | null) => void) | null = null
  eventLongPress: ((event: GestureEvent | null, state: string) => void) | null = null
  eventPan: ((event: GestureEvent | null, state: string) => void) | null = null
  eventTouchUp: ((event: TouchEvent) => void) | null = null
  eventTouchDown: ((event: TouchEvent) => void) | null = null
  eventTouchMove: ((event: TouchEvent) => void) | null = null

  constructor(ctx: NativeRenderContext) {
    this.ctx = ctx

    // TODO(hot):
    this.eventClick = (event) => {
      HRGestureDispatcher.handleClickEvent(this.ctx, this.tag, HRNodeProps.ON_CLICK)
    }
  }

  setProp(propKey: string, propValue: HippyAny | HippyRenderCallback): boolean {
    switch (propKey) {
      case HRNodeProps.VISIBILITY:
        this.cssVisibility = propValue as number == 1 ? Visibility.Visible : Visibility.Hidden
        break
      case HRNodeProps.BACKGROUND_COLOR:
        this.cssBackgroundColor = propValue as number
        break
      case HRNodeProps.OPACITY:
        this.cssOpacity = propValue as number
        break
      case HRNodeProps.BORDER_RADIUS:
        // this.cssBorderRadius = HRConvertUtil.toBorderRadius(propValue as string)
        // if (this.cssBorder) {
        //   this.cssBorder.radius = this.cssBorderRadius ? this.cssBorderRadius : undefined
        // } else {
        //   this.cssBorder = {width: 0, color: Color.Transparent, style: BorderStyle.Solid, radius: this.cssBorderRadius ? this.cssBorderRadius : undefined}
        // }
        break
      case HRNodeProps.BORDER_TOP_LEFT_RADIUS:
      case HRNodeProps.BORDER_TOP_RIGHT_RADIUS:
      case HRNodeProps.BORDER_BOTTOM_LEFT_RADIUS:
      case HRNodeProps.BORDER_BOTTOM_RIGHT_RADIUS:
        break
      case HRNodeProps.BORDER_WIDTH:
        // this.cssBorder = HRConvertUtil.toBorder(propValue as string, this.cssBorderRadius)
        break
      case HRNodeProps.TRANSFORM:
        // let transform = HRConvertUtil.toTransform(propValue as string)
        // this.cssScale = transform.scale
        // this.cssTranslate = transform.translate
        // this.cssRotate = transform.rotate
        break
      case HRNodeProps.OVERFLOW:
        this.cssOverflow = propValue as number == 1
        break
      // case cssTouchEnable:
      //   this.cssTouchEnable = propValue == 1
      //   break
      case HRNodeProps.Z_INDEX:
        this.cssZIndex = propValue as number
        break
      case HRNodeProps.ON_CLICK:
        this.eventClick = (event) => {
          HRGestureDispatcher.handleClickEvent(this.ctx, this.tag, HRNodeProps.ON_CLICK)
        }
        break
      case HRNodeProps.ON_LONG_CLICK:
        this.eventLongPress = (event, state) => {
          if(event) {
            let localX: number = event['localX'];
            let localY: number = event['localY'];
            (propValue as HippyRenderCallback)({
              'x': localX,
              'y': localY,
              'state': state
            } as HippyRecord)
          }
        }
        break
      case HRNodeProps.ON_TOUCH_END:
        this.eventTouchUp = (event) => {
          let params = touchEventToParamRecord(event) as HippyTouchParamType
          //(propValue as HippyRenderCallback)(params)
        }
        break
      case HRNodeProps.ON_TOUCH_DOWN:
        this.eventTouchDown = (event) => {
          let params = touchEventToParamRecord(event) as HippyTouchParamType
          //(propValue as HippyRenderCallback)(params)
        }
        break
      case HRNodeProps.ON_TOUCH_MOVE:
        this.eventTouchMove = (event) => {
          let params = touchEventToParamRecord(event) as HippyTouchParamType
          //(propValue as HippyRenderCallback)(params)
        }
        break
      case HRNodeProps.ON_TOUCH_CANCEL:
        break
      case HRNodeProps.BACKGROUND_IMAGE:
        this.setBackgroundImage(propValue as string)
        break
      default:
        break
    }

    return true
  }

  call(method: string, params: Array<HippyAny>, callback: HippyRenderCallback | null): void {

  }

  onDestroy(): void {

  }

  addSubRenderView(subView: HippyRenderBaseView, index: number): void {
    if (index < 0 || index > this.children.length) {
      index = this.children.length
    }
    subView.parentView = this
    this.children.splice(index, 0, subView)
  }

  removeSubView(subView: HippyRenderBaseView) {
    let index = this.children.indexOf(subView)
    if (index >= 0 && index < this.children.length) {
      this.children.splice(index, 1)
    }
  }

  removeFromParentView(): void {
    if (this.parentView) {
      this.parentView.removeSubView(this)
      this.parentView = null
    }
  }

  protected updateRenderViewFrame(frame: HRRect): void {
    this.cssPositionX = frame.x
    this.cssPositionY = frame.y
    this.cssWidth = frame.width
    this.cssHeight = frame.height
  }

  setRenderViewFrame(frame: HRRect): void {
    this.updateRenderViewFrame(frame)
  }

  setBackgroundImage(value: string): void {
    const linearGradientPrefix = 'linear-gradient(';
    if (value?.startsWith(linearGradientPrefix) && value.endsWith(')')) {
      const lg = value.substring(linearGradientPrefix.length, value.length - 1)
      this.cssLinearGradient = this.parseLinearGradient(lg)
    } else {
      this.cssLinearGradient = null
    }
  }

  private static readonly gradientDirectionOrder = [
    GradientDirection.Top,
    GradientDirection.Bottom,
    GradientDirection.Left,
    GradientDirection.Right,
    GradientDirection.LeftTop,
    GradientDirection.RightTop,
    GradientDirection.LeftBottom,
    GradientDirection.RightBottom
  ]

  private parseLinearGradient(lg: string): LinearGradient | null {
    try {
      const splits = lg.split(',')
      return {
        direction: HippyRenderBaseView.gradientDirectionOrder[parseInt(splits[0], 10)],
        colors: splits.slice(1).map((value) => {
          const pair = value.trim().split(' ');
          return [parseInt(pair[0], 10), parseFloat(pair[1])]
        }),
      }
    } catch (err) {
      LogUtils.e('HippyRenderBaseView', `parse linearGradient input = "${lg}" error = ${JSON.stringify(err)}`)
      return null
    }
  }

}
