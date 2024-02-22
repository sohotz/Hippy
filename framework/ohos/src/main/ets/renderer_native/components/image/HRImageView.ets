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
import { HippyAny, HippyRecord, HippyRenderCallback } from '../../../support/common/HippyTypes';
import { HRNodeProps } from '../../dom_node/HRNodeProps';
import { NativeRenderContext } from '../../NativeRenderContext';
import { HRConvertUtil } from '../../utils/HRConvertUtil';
import HippyRenderBaseView from '../base/HippyRenderBaseView';

interface ImageLoadEvent {
  width: number
  height: number
  componentWidth: number
  componentHeight: number
  loadingStatus: number
  contentWidth: number
  contentHeight: number
  contentOffsetX: number
  contentOffsetY: number
}

@Observed
export class HRImageView extends HippyRenderBaseView {
  private static readonly BASE64_IMAGE_PREFIX = "data:image"
  private static readonly ASSETS_IMAGE_PREFIX = "assets://"

  cssSrc: string | null = null
  cssResizeMode: ImageFit = ImageFit.Cover
  onCompleteCallback: ((event?: ImageLoadEvent) => void) | null = null
  eventLoadSuccess: HippyRenderCallback | null = null
  eventLoadResolution: HippyRenderCallback | null = null

  constructor(ctx: NativeRenderContext) {
    super(ctx)
  }

  setProp(propKey: string, propValue: HippyAny | HippyRenderCallback): boolean {
    switch (propKey) {
      case "src":
        this.cssSrc = propValue as string
        return true
      case "defaultSource":
        return true
      case "tintColor":
        return true
      case "tintColorBlendMode":
        return true
      case HRNodeProps.BACKGROUND_IMAGE:
        return true
      case HRNodeProps.BACKGROUND_POSITION_X:
        return true
      case HRNodeProps.BACKGROUND_POSITION_Y:
        return true
      case HRNodeProps.BACKGROUND_SIZE:
        return true
      case HRNodeProps.RESIZE_MODE:
        this.cssResizeMode = HRConvertUtil.toImageFit(propValue as string)
        return true
      default:
        break
    }
    return super.setProp(propKey, propValue as HippyAny)
  }

  private initOnCompleteCallbackIfNeeded() {
    if (!this.onCompleteCallback) {
      this.onCompleteCallback = (event) => {
        if (event?.loadingStatus !== 1) {
          return
        }
        if (this.eventLoadSuccess) {
          this.eventLoadSuccess({ 'src': this.cssSrc } as HippyRecord)
        }
        if (this.eventLoadResolution) {
          this.eventLoadResolution({ 'imageWidth': event.width, 'imageHeight': event.height } as HippyRecord)
        }
      }
    }
  }

  getImage(): ResourceStr | null {
    if (this.cssSrc?.startsWith(HRImageView.BASE64_IMAGE_PREFIX)) {
      return this.getBase64Image(this.cssSrc)
    }
    if (this.cssSrc?.startsWith(HRImageView.ASSETS_IMAGE_PREFIX)) {
      return $rawfile(this.cssSrc.substring(HRImageView.ASSETS_IMAGE_PREFIX.length))
    }
    return this.cssSrc;
  }

  private getBase64Image(key: string): string | null {
    const value = "" // TODO:
    if (value) {
      return value as string
    }
    return null
  }
}

@Component
export struct HRImage {
  @ObjectLink renderView: HRImageView
  build() {
    Image(this.renderView.getImage())
      .applyRenderViewBaseAttr(this.renderView)
      .objectFit(this.renderView.cssResizeMode)
      .onComplete(this.renderView.onCompleteCallback)
  }
}

// base props for all components
@Extend(Image)
function applyRenderViewBaseAttr($$: HippyRenderBaseView) {
  .backgroundColor($$.cssBackgroundColor)
  .position({x: $$.cssPositionX, y: $$.cssPositionY})
  .size({width:$$.cssWidth, height: $$.cssHeight})
  .opacity($$.cssOpacity)
  //.clip($$.cssOverflow)
  .visibility(($$ as HippyRenderBaseView).cssVisibility) // must add as, otherwise the compiler has error
  .zIndex($$.cssZIndex)
  .border($$.cssBorder)
  .rotate($$.cssRotate)
  .scale($$.cssScale)
  .translate($$.cssTranslate)
  .onClick($$.eventClick)
}
