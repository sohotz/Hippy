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
import deviceInfo  from '@ohos.deviceInfo';
import { HippyAny } from '../../../support/common/HippyTypes';
import { HRComponent } from '../../HippyRenderView';
import { NativeRenderContext } from '../../NativeRenderContext';
import HippyRenderBaseView, { HippyObservedArray } from '../base/HippyRenderBaseView';

const HippyGestureTag = 'HippyGestureTag'

@Observed
export class HRDivView extends HippyRenderBaseView {
  constructor(ctx: NativeRenderContext) {
    super(ctx)
  }

  setProp(propKey: string, propValue: HippyAny): boolean {
    return super.setProp(propKey, propValue)
  }
}

@Component
export struct HRDiv {
  @ObjectLink renderView: HRDivView
  @ObjectLink children: HippyObservedArray<HippyRenderBaseView>
  @BuilderParam buildRenderView:($$: HippyRenderBaseView, parent: HRComponent) => void

  shouldAddGesture() {
    return deviceInfo.sdkApiVersion >= 11 && (
      this.renderView.eventDoubleClick != null ||
        this.renderView.eventLongPress != null ||
        this.renderView.eventPan ||
        this.renderView.eventTouchUp != null ||
        this.renderView.eventTouchDown != null ||
        this.renderView.eventTouchMove != null
    )
  }

  build() {
    if(this.shouldAddGesture()){
      Stack() {
        ForEach(this.children, (item: HippyRenderBaseView) => {
          this.buildRenderView(item, null)
        }, (item: HippyRenderBaseView) => item.tag + '')
      }
      .applyRenderViewBaseAttr(this.renderView)
      //.applyGestures(this.renderView)
    }else {
      Stack() {
        ForEach(this.children, (item: HippyRenderBaseView) => {
          this.buildRenderView(item, null)
        }, (item: HippyRenderBaseView) => item.tag + '')
      }
      .applyRenderViewBaseAttr(this.renderView)
    }
  }
}

// base props for all components
@Extend(Stack)
function applyRenderViewBaseAttr($$: HippyRenderBaseView) {
  .backgroundColor($$.cssBackgroundColor)
  .position({x: $$.cssPositionX, y: $$.cssPositionY})
  .size({width:$$.cssWidth, height: $$.cssHeight})
  .opacity($$.cssOpacity)
  .clip($$.cssOverflow)
  .visibility(($$ as HippyRenderBaseView).cssVisibility) // must add as, otherwise the compiler has error
  .touchable($$.cssTouchEnable) // TODO: use new api
  .zIndex($$.cssZIndex)
  .border($$.cssBorder)
  .borderWidth(2) // TODO:
  .rotate($$.cssRotate)
  .scale($$.cssScale)
  .translate($$.cssTranslate)
  .onClick($$.eventClick)
}

@Extend(Stack)
function applyGestures($$: HippyRenderBaseView) {
  /*
  .onGestureJudgeBegin((gestureInfo: GestureInfo, event: BaseGestureEvent): GestureJudgeResult => {
    if (gestureInfo.tag != HippyGestureTag) {
      // let other gestures continue
      return GestureJudgeResult.CONTINUE
    }
    return (gestureInfo.type == GestureControl.GestureType.LONG_PRESS_GESTURE && $$.eventLongPress != null) ||
      (gestureInfo.type == GestureControl.GestureType.TAP_GESTURE && $$.eventDoubleClick != null) ||
      (gestureInfo.type == GestureControl.GestureType.PAN_GESTURE && $$.eventPan != null) ?
    GestureJudgeResult.CONTINUE : GestureJudgeResult.REJECT
  })
  .parallelGesture(GestureGroup(GestureMode.Parallel,
    TapGesture({ count: 2 })
      .tag(HippyGestureTag)
      .onAction((event?: GestureEvent) => {
        if (event && $$.eventDoubleClick) {
          $$.eventDoubleClick(event)
        }
      }) ,
    LongPressGesture({ repeat: false })
      .tag(HippyGestureTag)
      .onAction((event?: GestureEvent) => {
        if (event && $$.eventLongPress) {
          $$.eventLongPress(event, "start")
        }
      })
      .onActionEnd((event?: GestureEvent) => {
        if (event && $$.eventLongPress) {
          $$.eventLongPress(event, "end")
        }
      })
      .onActionCancel(() => {
        if ($$.eventLongPress) {
          $$.eventLongPress(null, "end")
        }
      }),
    PanGesture()
      .tag(HippyGestureTag)
      .onActionStart((event: GestureEvent) => {
        if ($$.eventPan) {
          $$.eventPan(event, "start")
        }
      })
      .onActionUpdate((event: GestureEvent) => {
        if ($$.eventPan) {
          $$.eventPan(event, "move")
        }
      })
      .onActionEnd((event?: GestureEvent) => {
        if ($$.eventPan) {
          $$.eventPan(event != undefined ? event : null, "end")
        }
      })
  ))
  .onTouch((event: TouchEvent) => {
    switch (event.type) {
      case TouchType.Down: {
        if ($$.eventTouchDown) {
          $$.eventTouchDown(event)
        }
      }
        break
      case TouchType.Up: {
        if ($$.eventTouchUp) {
          $$.eventTouchUp(event)
        }
      }
        break
      case TouchType.Move: {
        if ($$.eventTouchMove) {
          $$.eventTouchMove(event)
        }
      }
        break
    }
  })
  //*/
}
