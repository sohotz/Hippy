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
import { HippyAny, HippyRecord, HippyRenderCallback,
  HippyResource } from '../../../support/common/HippyTypes';
import { HRNodeProps } from '../../dom_node/HRNodeProps';
import { NativeRenderContext } from '../../NativeRenderContext';
import { HRConvertUtil } from '../../utils/HRConvertUtil';
import HippyRenderBaseView from '../base/HippyRenderBaseView';
import { LogUtils } from '../../../support/utils/LogUtils';
import { ResourceDataHolder } from '../../../vfs/ResourceDataHolder';

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
  private static readonly RAW_IMAGE_PREFIX = "hpfile://"
  private readonly TAG = "HRImageView"
  cssSrc: string | null = null
  cssResizeMode: ImageFit = ImageFit.Cover
  cssBorderColor: number = 0
  cssBorderStyle: BorderStyle = BorderStyle.Solid
  cssBorderWidth: number = 0
  cssAlt: string = ""
  cssFillColor: ResourceColor = 0
  cssMargin: number = 0
  cssHeight: number = 0
  cssWidth: number = 0
  cssColorFilter: ColorFilter = [0]
  onCompleteCallback: ((event?: ImageLoadEvent) => void) | null = null
  eventLoadSuccess: HippyRenderCallback | null = null
  eventLoadResolution: HippyRenderCallback | null = null

  constructor(ctx: NativeRenderContext) {
    super(ctx)
    console.log(`hrimageview constructor`)
  }

  setResizeMode(pvalue: string) {
    console.log(`hrimageview setResizeMode`)
    switch (pvalue) {
      case 'contain':
        this.cssResizeMode = ImageFit.Contain
        break;
      case 'cover':
        this.cssResizeMode = ImageFit.Cover
        break;
      case 'center':
        this.cssResizeMode = ImageFit.Contain
        break;
      default:
        console.log(`unknown resizeMode : ${pvalue}`)
        break;
    }
  }

  setBorderStyle(pvalue: string) {
    console.log(`hrimageview setBorderStyle`)
    switch (pvalue) {
      case 'solid':
        this.cssBorderStyle = BorderStyle.Solid
        break;
      default:
        console.log(`unknown BorderStyle : ${pvalue}`)
        break;
    }
  }

  setProp(propKey: string, propValue: HippyAny | HippyRenderCallback): boolean {
    console.log(`hrimageview propKey is : ${propKey},propValue is : ${propValue}`)
    switch (propKey) {
      case 'src':
        this.cssSrc = propValue as string
        return true
      case 'resizeMode':
        {
          let pvalue = propValue as string
          this.setResizeMode(pvalue)
        }
        return true
      case 'borderStyle':
        {
          let pvalue = propValue as string
          this.setBorderStyle(pvalue)
        }
        return true
      case 'borderColor':
        {
          let borderColor = propValue as number
          this.cssOpacity = (borderColor >> 24) & 0xff
          this.cssBorderColor = borderColor & 0xffffff
          LogUtils.d(this.TAG, "hrimageview borderColor: cssOpacity " + this.cssOpacity  + " cssBorderColor: " + this.cssBorderColor)
        }
        return true
      case 'borderWidth':
        this.cssBorderWidth = propValue as number
        return true
      case 'defaultSource':
        {
          this.cssAlt = propValue as string
          let splitList = this.cssAlt.split("/")
          this.cssAlt = "vue2/assets/" + splitList[splitList.length - 1]
        }
        return true
      case 'tintColor':
        {
          let rgb = [0]
          let matrixColor = [0]
          let intColor = propValue as number
          rgb[0] = (intColor >> 24) & 0xff   //alpha
          rgb[1] = (intColor >> 16) & 0xff   //red
          rgb[2] = (intColor >> 8) & 0xff    //green
          rgb[3] = intColor & 0xff           //blue
          matrixColor[0]= Number((rgb[0]/0xff).toFixed(2))
          matrixColor[1]= Number((rgb[1]/0xff).toFixed(2))
          matrixColor[2]= Number((rgb[2]/0xff).toFixed(2))
          matrixColor[3]= Number((rgb[3]/0xff).toFixed(2))
          this.cssColorFilter = [
              matrixColor[1],  0,  0,  0,  0,
              0 , matrixColor[2] , 0,  0 , 0,
              0 , 0 , matrixColor[3],  0,  0,
              0 , 0 , 0,  matrixColor[0],  0]
        }
        return true
      case 'margin':
        this.cssMargin = propValue as number
        return true
      case 'height':
        this.cssHeight = propValue as number
        return true
      case 'width':
        this.cssWidth = propValue as number
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

  getImage(): HippyResource {
    let imageLoader = this.ctx.getNativeRender()!.getImageLoader()!
    if (this.cssSrc) {
      return imageLoader.fetchImageSync(this.cssSrc)
    }
    return ""
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
      .borderWidth(this.renderView.cssBorderWidth)
      .borderColor(this.renderView.cssBorderColor)
      .borderStyle(this.renderView.cssBorderStyle)
      .alt($rawfile(this.renderView.cssAlt))
      .colorFilter(this.renderView.cssColorFilter)
      .width(this.renderView.cssWidth)
      .height(this.renderView.cssHeight)

  }
}

// base props for all components
@Extend(Image)
function applyRenderViewBaseAttr($$: HippyRenderBaseView) {
  .backgroundColor($$.cssBackgroundColor)
  .position({x: $$.cssPositionX, y: $$.cssPositionY})
  .size({width:$$.cssWidth, height: $$.cssHeight})
  .opacity($$.cssOpacity)
  .clip($$.cssOverflow)
  .visibility(($$ as HippyRenderBaseView).cssVisibility) // must add as, otherwise the compiler has error
  .zIndex($$.cssZIndex)
  .border($$.cssBorder)
  .rotate($$.cssRotate)
  .scale($$.cssScale)
  .translate($$.cssTranslate)
  .onClick($$.eventClick)
}
