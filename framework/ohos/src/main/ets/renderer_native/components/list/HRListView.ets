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
import { NativeRenderContext } from '../../NativeRenderContext';
import HippyRenderBaseView from '../base/HippyRenderBaseView';
import { LogUtils } from '../../../support/utils/LogUtils';
import { HRValueUtils } from '../../utils/HRValueUtils';

@Observed
export class HRListView extends HippyRenderBaseView {
  private readonly TAG = "HRListView"
  public flex: number = 0
  public scrollForward: number = NestedScrollMode.SELF_FIRST
  public scrollBackward: number = NestedScrollMode.SELF_FIRST
  public readonly EXPOSURE_STATE_FULL_VISIBLE: number = 1;
  public readonly EXPOSURE_STATE_INVISIBLE: number = 2;
  public readonly EXPOSURE_STATE_PART_VISIBLE: number = 3;
  public scroller: ListScroller = new ListScroller()
  public uiCtx: UIContext | null = null
  listDirection: Axis = Axis.Vertical;
  scrollEnabled: boolean = true;
  initialOffset: number = 0;
  initialIndex: number = 0;
  cachedCount: number = 0;
  scrollEventThrottle: number = 30;
  preloadItemNumber: number = 0;
  onScrollEventEnable: boolean = true;
  exposureEventEnabled: Boolean = true;
  exposureState: number = this.EXPOSURE_STATE_INVISIBLE;
  rowShouldSticky: boolean = false

  lastItemFullVisibleYOffset: number = 0
  footerViewFullVisibleYOffset: number = 0
  // 当initialIndex设为1时，初始yOffset并不是headerView的高度，而是一个200+的值，
  // 只有下滑一下触发了headerView的创建yOffset才变对。
  // 所以这里做个修正：dtYOffset = 初始yOffset - headerView的高度
  dtYOffset: number = 0

  constructor(ctx: NativeRenderContext) {
    super(ctx)
  }

  setProp(propKey: string, propValue: HippyAny | HippyRenderCallback): boolean {
    LogUtils.d(this.TAG, "setProp: propKey " + propKey + ", value: " + propValue)
    switch (propKey) {
      case 'nestedScrollTopPriority':
        {
          let pvalue = HRValueUtils.getString(propValue as HippyAny)
          if (pvalue === 'parent') {
            this.scrollForward = NestedScrollMode.PARENT_FIRST
            //this.scrollBackward = NestedScrollMode.PARENT_FIRST
          } else if (pvalue === 'self') {
            this.scrollForward = NestedScrollMode.SELF_FIRST
            //this.scrollBackward = NestedScrollMode.SELF_FIRST
          }
        }
        return true
      case "cssHeight":
        this.cssHeight = HRValueUtils.getNumber(propValue as HippyAny);
        return true
      case "horizontal":
        this.listDirection = HRValueUtils.getBoolean(propValue as HippyAny, false) == true ? Axis.Horizontal : Axis.Vertical;
        return true
      case "scrollEnabled":
        this.scrollEnabled = HRValueUtils.getBoolean(propValue as HippyAny, true);
        return true
      case "initialContentOffset":
        this.initialOffset = HRValueUtils.getNumber(propValue as HippyAny);
        return true
      case "itemViewCacheSize":
        this.cachedCount = HRValueUtils.getNumber(propValue as HippyAny);
        return true
      case "scrollEventThrottle":
        this.scrollEventThrottle = HRValueUtils.getNumber(propValue as HippyAny);
        return true
      case "preloadItemNumber":
        this.preloadItemNumber = HRValueUtils.getNumber(propValue as HippyAny);
        return true
      case "exposureEventEnabled":
        this.exposureEventEnabled = HRValueUtils.getBoolean(propValue as HippyAny, true);
        return true
      case 'rowShouldSticky':
        this.rowShouldSticky = HRValueUtils.getBoolean(propValue as HippyAny, false);
        return true
      case 'bounces':

        return true
      default:
        break;
    }

    return super.setProp(propKey, propValue as HippyAny)
  }

  call(method: string, params: Array<HippyAny>, callback: HippyRenderCallback | null): void {
    this.uiCtx?.runScopedTask(() => {
    LogUtils.d(this.TAG, "call: method " + method + ", params: " + params)
    switch (method) {
      case "scrollToIndex":
        {
          let startOff = params[0] as number;
          let endOff = params[1] as number;
          let needAnimation = params[2] as boolean;
          this.scroller.scrollToItemInGroup(1, endOff-4)
        }
        break;
      case "scrollToContentOffset":
        {
          let startOff = params[0] as number;
          let endOff = params[1] as number;
          let needAnimation = params[2] as boolean;
          this.scroller.scrollTo({
            xOffset: 0,
            yOffset: endOff,
            animation: needAnimation
          })
        }
        break;
      case "scrollToTop":
        {
          this.scroller.scrollToIndex(0);
        }
        break
      default:
        break;
      }
    })
  }

}
