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
import { HippyAny, HippyRenderCallback } from '../../../support/common/HippyTypes';
import { HRNodeProps } from '../../dom_node/HRNodeProps';
import { NativeRenderContext } from '../../NativeRenderContext';
import { HRGestureDispatcher } from '../../uimanager/HRGestureDispatcher';
import { HRRect } from '../../utils/HRTypes';

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
        break
      case HRNodeProps.BORDER_TOP_LEFT_RADIUS:
      case HRNodeProps.BORDER_TOP_RIGHT_RADIUS:
      case HRNodeProps.BORDER_BOTTOM_LEFT_RADIUS:
      case HRNodeProps.BORDER_BOTTOM_RIGHT_RADIUS:
        break
      case HRNodeProps.BORDER_WIDTH:
        break
      case HRNodeProps.TRANSFORM:
        break
      case HRNodeProps.OVERFLOW:
        this.cssOverflow = propValue as number == 1
        break
      case HRNodeProps.Z_INDEX:
        this.cssZIndex = propValue as number
        break
      case HRNodeProps.ON_CLICK:
        this.eventClick = (event) => {
          HRGestureDispatcher.handleClickEvent(this.ctx, this.tag, HRNodeProps.ON_CLICK)
        }
        break
      default:
        break
    }

    return true
  }

  call(method: string, params: HippyAny, callback: HippyRenderCallback | null): void {

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
