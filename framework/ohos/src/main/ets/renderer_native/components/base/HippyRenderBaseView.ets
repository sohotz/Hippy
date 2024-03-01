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
import matrix4 from '@ohos.matrix4';
import { HippyAny,
  HippyArray,
  HippyMap,
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
import { HRValueUtils } from '../../utils/HRValueUtils';

export const HIPPY_COMPONENT_KEY_PREFIX = "HippyKey"

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
  cssZIndex: number | null = null
  cssAccessibilityLabel: string | null = null
  cssFocusable: boolean | null = null
  cssBorder: BorderOptions | undefined = undefined
  cssShadow: ShadowOptions | undefined = undefined
  cssShadowColor: number = 0
  cssShadowOpacity: number | undefined = undefined
  cssLinearGradient: LinearGradient | null = null
  cssBackgroundImage: ResourceStr | null = null // TODO(hot):
  cssBackgroundImagePosition: Position | null = null
  cssBackgroundImageSize: ImageSize | null = null
  cssRotate: RotateOptions | null = null
  cssScale: ScaleOptions | null = null
  cssTranslate: TranslateOptions | null = null
  cssMatrix: matrix4.Matrix4Transit | null = null
  eventClick: ((event: ClickEvent) => void) | null = null
  eventDoubleClick: ((event: GestureEvent | null) => void) | null = null
  eventLongPress: ((event: GestureEvent | null, state: string) => void) | null = null
  eventPan: ((event: GestureEvent | null, state: string) => void) | null = null
  eventTouchUp: ((event: TouchEvent) => void) | null = null
  eventTouchDown: ((event: TouchEvent) => void) | null = null
  eventTouchMove: ((event: TouchEvent) => void) | null = null

  constructor(ctx: NativeRenderContext) {
    this.ctx = ctx
  }

  protected handleGestureBySelf(): boolean {
    return false
  }

  setProp(propKey: string, propValue: HippyAny): boolean {
    switch (propKey) {
      case HRNodeProps.VISIBILITY:
        this.cssVisibility = HRValueUtils.getString(propValue) != HRNodeProps.HIDDEN ? Visibility.Visible : Visibility.Hidden
        break
      case HRNodeProps.BACKGROUND_COLOR:
        this.cssBackgroundColor = HRValueUtils.getNumber(propValue)
        break
      case HRNodeProps.OPACITY:
        this.cssOpacity = HRValueUtils.getNumber2(propValue, 1.0)
        break
      case HRNodeProps.TRANSFORM:
        this.setTransformProp(propKey, propValue)
        break
      case HRNodeProps.OVERFLOW:
        this.cssOverflow = HRValueUtils.getString(propValue) != HRNodeProps.HIDDEN ? false : true
        break
      case HRNodeProps.Z_INDEX:
        this.cssZIndex = HRValueUtils.getNumber(propValue)
        break
      case HRNodeProps.PROP_ACCESSIBILITY_LABEL:
        this.cssAccessibilityLabel = HRValueUtils.getString(propValue)
        break
      case HRNodeProps.FOCUSABLE:
        this.cssFocusable = HRValueUtils.getBoolean(propValue, false)
        break
      case HRNodeProps.REQUEST_FOCUS:
        focusControl.requestFocus(HIPPY_COMPONENT_KEY_PREFIX + this.tag)
        break
      case HRNodeProps.LINEAR_GRADIENT:
        this.setLinearGradientProp(propKey, propValue)
        break
      default:
        {
          let handled = this.setBackgroundImageProp(propKey, propValue)
          if (!handled) {
            handled = this.setBorderProp(propKey, propValue)
          }
          if (!handled) {
            handled = this.setShadowProp(propKey, propValue)
          }
          if (!handled) {
            this.setEventProp(propKey, propValue)
          }
        }
        break
    }
    return true
  }

  private setTransformProp(propKey: string, propValue: HippyAny): boolean {
    let transform = HRConvertUtil.toTransform(propValue as HippyArray)
    if (transform) {
      this.cssScale = transform.scale
      this.cssTranslate = transform.translate
      this.cssRotate = transform.rotate
      this.cssMatrix = transform.matrix
    }
    return true
  }

  private setLinearGradientProp(propKey: string, propValue: HippyAny): boolean {
    let m = HRValueUtils.getMap(propValue)
    if (!m) {
      return false
    }

    let angle = HRValueUtils.getString((m as Map<string, HippyAny>).get('angle') as HippyAny)
    let colorStopList = HRValueUtils.getArray((m as Map<string, HippyAny>).get('colorStopList') as HippyArray)
    if (!angle || angle.length == 0 || !colorStopList || colorStopList.length == 0) {
      return false
    }

    let size = colorStopList.length
    let colorsArray = new Array<Array<number>>()
    for (let i = 0; i < size; i++) {
      let colorStop = HRValueUtils.getMap(colorStopList[i])
      if (!colorStop) {
        continue
      }
      let color = HRValueUtils.getNumber(colorStop.get('color') as HippyAny)
      let ratio = 0.0
      if (colorStop.get('ratio')) {
        ratio = HRValueUtils.getNumber(colorStop.get('ratio') as HippyAny)
      } else if (i == size - 1) {
        ratio = 1.0
      }
      colorsArray.push([color, ratio])
    }

    switch (angle) {
      case 'totopright':
        this.cssLinearGradient = { direction: GradientDirection.RightTop, colors: colorsArray }
        break
      case 'tobottomright':
        this.cssLinearGradient = { direction: GradientDirection.RightBottom, colors: colorsArray }
        break
      case 'tobottomleft':
        this.cssLinearGradient = { direction: GradientDirection.LeftBottom, colors: colorsArray }
        break
      case 'totopleft':
        this.cssLinearGradient = { direction: GradientDirection.LeftTop, colors: colorsArray }
        break
      default: {
        let value = Math.trunc(HRValueUtils.getNumber(angle)) % 360
        this.cssLinearGradient = { angle: value, colors: colorsArray }
      }
        break
    }
    return true
  }

  private setBackgroundImageProp(propKey: string, propValue: HippyAny): boolean {
    switch (propKey) {
      case HRNodeProps.BACKGROUND_IMAGE:
        if (typeof propValue == 'string') {
          this.cssBackgroundImage = this.convertToLocalPathIfNeeded(propValue)
        }
        break
      case HRNodeProps.BACKGROUND_POSITION_X:
        if (!this.cssBackgroundImagePosition) {
          this.cssBackgroundImagePosition = { x: 0, y: 0}
        }
        this.cssBackgroundImagePosition.x = HRValueUtils.getNumber(propValue)
        break
      case HRNodeProps.BACKGROUND_POSITION_Y:
        if (!this.cssBackgroundImagePosition) {
          this.cssBackgroundImagePosition = { x: 0, y: 0}
        }
        this.cssBackgroundImagePosition.y = HRValueUtils.getNumber(propValue)
        break
      case HRNodeProps.BACKGROUND_SIZE:
        this.cssBackgroundImageSize = HRConvertUtil.toImageSize(propValue)
        break
      default:
        return false
    }
    return true
  }

  private convertToLocalPathIfNeeded(uri: string): ResourceStr {
    // hpfile://./assets/defaultSource.jpg
    if (uri != null && uri.startsWith("hpfile://")) {
      let relativePath = uri.replace("hpfile://./", "");
      let bundlePath = "vue2/" // TODO(hot):
      uri = bundlePath.substring(0, bundlePath.lastIndexOf("/") + 1) + relativePath
      return $rawfile(uri)
    }
    return uri
  }

  private setBorderProp(propKey: string, propValue: HippyAny): boolean {
    switch (propKey) {
      case HRNodeProps.BORDER_RADIUS: {
        this.checkAndBuildCssBorderRadius();
        let value: number = HRValueUtils.getNumber(propValue);
        (this.cssBorder?.radius as BorderRadiuses).topLeft = value;
        (this.cssBorder?.radius as BorderRadiuses).topRight = value;
        (this.cssBorder?.radius as BorderRadiuses).bottomRight = value;
        (this.cssBorder?.radius as BorderRadiuses).bottomLeft = value;
      }
        break
      case HRNodeProps.BORDER_TOP_LEFT_RADIUS:
        this.checkAndBuildCssBorderRadius();
        (this.cssBorder?.radius as BorderRadiuses).topLeft = HRValueUtils.getNumber(propValue)
        break
      case HRNodeProps.BORDER_TOP_RIGHT_RADIUS:
        this.checkAndBuildCssBorderRadius();
        (this.cssBorder?.radius as BorderRadiuses).topRight = HRValueUtils.getNumber(propValue)
        break
      case HRNodeProps.BORDER_BOTTOM_RIGHT_RADIUS:
        this.checkAndBuildCssBorderRadius();
        (this.cssBorder?.radius as BorderRadiuses).bottomRight = HRValueUtils.getNumber(propValue)
        break
      case HRNodeProps.BORDER_BOTTOM_LEFT_RADIUS:
        this.checkAndBuildCssBorderRadius();
        (this.cssBorder?.radius as BorderRadiuses).bottomLeft = HRValueUtils.getNumber(propValue)
        break
      case HRNodeProps.BORDER_WIDTH: {
        this.checkAndBuildCssBorderWidth();
        let value: number = HRValueUtils.getNumber(propValue);
        (this.cssBorder?.width as EdgeWidths).top = value;
        (this.cssBorder?.width as EdgeWidths).right = value;
        (this.cssBorder?.width as EdgeWidths).bottom = value;
        (this.cssBorder?.width as EdgeWidths).left = value;
      }
        break
      case HRNodeProps.BORDER_TOP_WIDTH:
        this.checkAndBuildCssBorderWidth();
        (this.cssBorder?.width as EdgeWidths).top = HRValueUtils.getNumber(propValue)
        break
      case HRNodeProps.BORDER_RIGHT_WIDTH:
        this.checkAndBuildCssBorderWidth();
        (this.cssBorder?.width as EdgeWidths).right = HRValueUtils.getNumber(propValue)
        break
      case HRNodeProps.BORDER_BOTTOM_WIDTH:
        this.checkAndBuildCssBorderWidth();
        (this.cssBorder?.width as EdgeWidths).bottom = HRValueUtils.getNumber(propValue)
        break
      case HRNodeProps.BORDER_LEFT_WIDTH:
        this.checkAndBuildCssBorderWidth();
        (this.cssBorder?.width as EdgeWidths).left = HRValueUtils.getNumber(propValue)
        break
      case HRNodeProps.BORDER_STYLE: {
        this.checkAndBuildCssBorderStyle();
        let value = HRConvertUtil.toBorderStyle(HRValueUtils.getString(propValue));
        (this.cssBorder?.style as EdgeStyles).top = value;
        (this.cssBorder?.style as EdgeStyles).right = value;
        (this.cssBorder?.style as EdgeStyles).bottom = value;
        (this.cssBorder?.style as EdgeStyles).left = value;
      }
        break
      case HRNodeProps.BORDER_TOP_STYLE:
        this.checkAndBuildCssBorderStyle();
        (this.cssBorder?.style as EdgeStyles).top = HRConvertUtil.toBorderStyle(HRValueUtils.getString(propValue));
        break
      case HRNodeProps.BORDER_RIGHT_STYLE:
        this.checkAndBuildCssBorderStyle();
        (this.cssBorder?.style as EdgeStyles).right = HRConvertUtil.toBorderStyle(HRValueUtils.getString(propValue));
        break
      case HRNodeProps.BORDER_BOTTOM_STYLE:
        this.checkAndBuildCssBorderStyle();
        (this.cssBorder?.style as EdgeStyles).bottom = HRConvertUtil.toBorderStyle(HRValueUtils.getString(propValue));
        break
      case HRNodeProps.BORDER_LEFT_STYLE:
        this.checkAndBuildCssBorderStyle();
        (this.cssBorder?.style as EdgeStyles).left = HRConvertUtil.toBorderStyle(HRValueUtils.getString(propValue));
        break
      case HRNodeProps.BORDER_COLOR: {
        this.checkAndBuildCssBorderColor();
        let value: number = HRValueUtils.getNumber(propValue);
        (this.cssBorder?.color as EdgeColors).top = value;
        (this.cssBorder?.color as EdgeColors).right = value;
        (this.cssBorder?.color as EdgeColors).bottom = value;
        (this.cssBorder?.color as EdgeColors).left = value;
      }
        break
      case HRNodeProps.BORDER_TOP_COLOR:
        this.checkAndBuildCssBorderColor();
        (this.cssBorder?.color as EdgeColors).top = HRValueUtils.getNumber(propValue);
        break
      case HRNodeProps.BORDER_RIGHT_COLOR:
        this.checkAndBuildCssBorderColor();
        (this.cssBorder?.color as EdgeColors).right = HRValueUtils.getNumber(propValue);
        break
      case HRNodeProps.BORDER_BOTTOM_COLOR:
        this.checkAndBuildCssBorderColor();
        (this.cssBorder?.color as EdgeColors).bottom = HRValueUtils.getNumber(propValue);
        break
      case HRNodeProps.BORDER_LEFT_COLOR:
        this.checkAndBuildCssBorderColor();
        (this.cssBorder?.color as EdgeColors).left = HRValueUtils.getNumber(propValue);
        break
      default:
        return false
    }
    return true
  }

  private checkAndBuildCssBorder() {
    if(!this.cssBorder) {
      this.cssBorder = {width: undefined, color: undefined, radius: undefined, style: undefined}
    }
  }

  private checkAndBuildCssBorderWidth() {
    this.checkAndBuildCssBorder();
    (this.cssBorder as BorderOptions).width = {top: undefined, right: undefined, bottom: undefined, left: undefined}
  }

  private checkAndBuildCssBorderColor() {
    this.checkAndBuildCssBorder();
    (this.cssBorder as BorderOptions).color = {top: undefined, right: undefined, bottom: undefined, left: undefined}
  }

  private checkAndBuildCssBorderRadius() {
    this.checkAndBuildCssBorder();
    (this.cssBorder as BorderOptions).radius = {topLeft: undefined, topRight: undefined, bottomLeft:undefined, bottomRight: undefined}
  }

  private checkAndBuildCssBorderStyle() {
    this.checkAndBuildCssBorder();
    (this.cssBorder as BorderOptions).style = {top: undefined, right: undefined, bottom: undefined, left: undefined}
  }

  private setShadowProp(propKey: string, propValue: HippyAny): boolean {
    switch (propKey) {
      case HRNodeProps.SHADOW_OFFSET: {
        this.checkAndBuildCssShadow();
        let m = HRValueUtils.getMap(propValue);
        let x = m ? (m as HippyMap).get('x') : 0;
        let y = m ? (m as HippyMap).get('y') : 0;
        (this.cssShadow as ShadowOptions).offsetX = HRValueUtils.getNumber(x as HippyAny);
        (this.cssShadow as ShadowOptions).offsetY = HRValueUtils.getNumber(y as HippyAny);
      }
        break
      case HRNodeProps.SHADOW_OFFSET_X:
        this.checkAndBuildCssShadow();
        (this.cssShadow as ShadowOptions).offsetX = HRValueUtils.getNumber(propValue);
        break
      case HRNodeProps.SHADOW_OFFSET_Y:
        this.checkAndBuildCssShadow();
        (this.cssShadow as ShadowOptions).offsetY = HRValueUtils.getNumber(propValue);
        break
      case HRNodeProps.SHADOW_OPACITY:
        this.checkAndBuildCssShadow();
        this.cssShadowOpacity = HRValueUtils.getNumber(propValue); // 0-1.0
        (this.cssShadow as ShadowOptions).color = HRConvertUtil.toColorWithNumberAndOpacity(this.cssShadowColor, this.cssShadowOpacity);
        break
      case HRNodeProps.SHADOW_RADIUS:
        this.checkAndBuildCssShadow();
        (this.cssShadow as ShadowOptions).radius = HRValueUtils.getNumber(propValue);
        break
      case HRNodeProps.SHADOW_COLOR:
        this.checkAndBuildCssShadow();
        this.cssShadowColor = HRValueUtils.getNumber(propValue);
        if (this.cssShadowOpacity == undefined) {
          (this.cssShadow as ShadowOptions).color = HRConvertUtil.toColorWithNumber(this.cssShadowColor);
        } else {
          (this.cssShadow as ShadowOptions).color = HRConvertUtil.toColorWithNumberAndOpacity(this.cssShadowColor, this.cssShadowOpacity);
        }
        break
      default:
        return false
    }
    return true
  }

  private checkAndBuildCssShadow() {
    if(!this.cssShadow) {
      this.cssShadow = {radius: 0, type: ShadowType.COLOR, color: undefined, offsetX: undefined, offsetY: undefined}
    }
  }

  private setEventProp(propKey: string, propValue: HippyAny): boolean {
    switch (propKey) {
      case HRNodeProps.ON_CLICK:
        if (typeof propValue == 'boolean') {
          this.setClickable(propValue)
        }
        break
      case HRNodeProps.ON_LONG_CLICK:
        if (typeof propValue == 'boolean') {
          this.setLongClickable(propValue)
        }
        break
      case HRNodeProps.ON_PRESS_IN:
        if (typeof propValue == 'boolean') {
          this.setPressIn(propValue)
        }
        break
      case HRNodeProps.ON_PRESS_OUT:
        if (typeof propValue == 'boolean') {
          this.setPressOut(propValue)
        }
        break
      case HRNodeProps.ON_TOUCH_DOWN:
        if (typeof propValue == 'boolean') {
          this.setTouchDownHandle(propValue)
        }
        break
      case HRNodeProps.ON_TOUCH_MOVE:
        if (typeof propValue == 'boolean') {
          this.setTouchMoveHandle(propValue)
        }
        break
      case HRNodeProps.ON_TOUCH_END:
        if (typeof propValue == 'boolean') {
          this.setTouchEndHandle(propValue)
        }
        break
      case HRNodeProps.ON_TOUCH_CANCEL:
        if (typeof propValue == 'boolean') {
          this.setTouchCancelHandle(propValue)
        }
        break
      case HRNodeProps.ON_INTERCEPT_TOUCH_EVENT:
        if (typeof propValue == 'boolean') {
          this.setInterceptTouch(propValue)
        }
        break
      case HRNodeProps.ON_INTERCEPT_PULL_UP_EVENT:
        if (typeof propValue == 'boolean') {
          this.setInterceptPullUp(propValue)
        }
        break
      case HRNodeProps.ON_ATTACHED_TO_WINDOW:
        if (typeof propValue == 'boolean') {
          this.setAttachedToWindowHandle(propValue)
        }
        break
      case HRNodeProps.ON_DETACHED_FROM_WINDOW:
        if (typeof propValue == 'boolean') {
          this.setDetachedFromWindowHandle(propValue)
        }
        break
      default:
        return false
    }
    return true
  }

  private setClickable(flag: boolean) {
    if (!this.handleGestureBySelf()) {
      if (flag) {
        this.eventClick = (event) => {
          HRGestureDispatcher.handleClickEvent(this.ctx, this.tag, HRNodeProps.ON_CLICK)
        }
      } else {
        this.eventClick = null
      }
    }
  }

  private setLongClickable(flag: boolean) {
    if (!this.handleGestureBySelf()) {

    }

    this.eventLongPress = (event, state) => {
      if(event) {
        // let localX: number = event['localX'];
        // let localY: number = event['localY'];
        // (propValue as HippyRenderCallback)({
        //   'x': localX,
        //   'y': localY,
        //   'state': state
        // } as HippyRecord)
      }
    }
  }

  private setPressIn(flag: boolean) {
    if (!this.handleGestureBySelf()) {

    }
  }

  private setPressOut(flag: boolean) {
    if (!this.handleGestureBySelf()) {

    }
  }

  private setTouchDownHandle(flag: boolean) {
    if (!this.handleGestureBySelf()) {

    }
    this.eventTouchDown = (event) => {
      let params = touchEventToParamRecord(event) as HippyTouchParamType
      //(propValue as HippyRenderCallback)(params)
    }
  }

  private setTouchMoveHandle(flag: boolean) {
    if (!this.handleGestureBySelf()) {

    }
    this.eventTouchMove = (event) => {
      let params = touchEventToParamRecord(event) as HippyTouchParamType
      //(propValue as HippyRenderCallback)(params)
    }
  }

  private setTouchEndHandle(flag: boolean) {
    if (!this.handleGestureBySelf()) {

    }
    this.eventTouchUp = (event) => {
      let params = touchEventToParamRecord(event) as HippyTouchParamType
      //(propValue as HippyRenderCallback)(params)
    }
  }

  private setTouchCancelHandle(flag: boolean) {
    if (!this.handleGestureBySelf()) {

    }
  }

  private setInterceptTouch(flag: boolean) {
    if (!this.handleGestureBySelf()) {

    }
  }

  private setInterceptPullUp(flag: boolean) {
    if (!this.handleGestureBySelf()) {

    }
  }

  private setAttachedToWindowHandle(flag: boolean) {
    if (!this.handleGestureBySelf()) {

    }
  }

  private setDetachedFromWindowHandle(flag: boolean) {
    if (!this.handleGestureBySelf()) {

    }
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

}
