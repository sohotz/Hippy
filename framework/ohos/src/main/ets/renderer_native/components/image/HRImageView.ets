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
import { LogUtils } from '../../../support/utils/LogUtils';
import {HRDownloadUtils} from '../../utils/HRDownloadUtils'
import common from '@ohos.app.ability.common';


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

enum TintColorBlendMode {
  CLEAR,
  SRC,
  DST,
  SRC_OVER,
  DST_OVER,
  SRC_IN,
  DST_IN,
  SRC_OUT,
  DST_OUT,
  DST_ATOP = 10,
  XOR,
  ADD,
  MULTIPLY,
  SCREEN,
  OVERLAY,
  DARKEN,
  LIGHTEN,
  SRC_ATOP,
}

@Observed
export class HRImageView extends HippyRenderBaseView {
  private static readonly BASE64_IMAGE_PREFIX = "data:image"
  private static readonly RAW_IMAGE_PREFIX = "hpfile://"
  private static readonly INTERNET_IMAGE_PREFIX = "http"
  private readonly TAG = "HRImageView"
  private imageLoader:HRDownloadUtils = new HRDownloadUtils
  tempDir:string = getContext().tempDir

  cssSrc: string | null = null
  //cssBackgroundColor: number = 0
  //cssBackgroundColorArray: number[] = [0]
  cssResizeMode: ImageFit = ImageFit.Cover
  cssBorderColor: number = 0
  cssBorderStyle: BorderStyle = BorderStyle.Solid
  cssBorderWidth: number = 0
  cssAlt: string = ""
  cssFillColor: ResourceColor = 0
  cssMargin: number = 0
  cssHeight: number = 0
  cssWidth: number = 0
  cssTintColor: number[] = [0]  //RGBA
  cssColorFilter: ColorFilter = [0]
  cssTintColorBlendMode: TintColorBlendMode = TintColorBlendMode.DST
  cssPreTintColorBlendMode: TintColorBlendMode = TintColorBlendMode.DST
  onCompleteCallback: ((event?: ImageLoadEvent) => void) | null = null
  eventLoadSuccess: HippyRenderCallback | null = null
  eventLoadResolution: HippyRenderCallback | null = null

  constructor(ctx: NativeRenderContext) {
    super(ctx)
    LogUtils.d(this.TAG,"constructor")
  }

  setResizeMode(pvalue: string) {
    LogUtils.d(this.TAG,"setResizeMode")
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
    LogUtils.d(this.TAG,"setBorderStyle")
    switch (pvalue) {
      case 'solid':
        this.cssBorderStyle = BorderStyle.Solid
        break;
      default:
        console.log(`unknown BorderStyle : ${pvalue}`)
        break;
    }
  }

  private saturate(x:number):number {
      return Math.max(0.0, Math.min(1.0, x));
}

  private setTintColorBlendMode(propValue: HippyAny){
    LogUtils.d(this.TAG,"setTintColorBlendMode")
    this.cssPreTintColorBlendMode = this.cssTintColorBlendMode;
    let blendMode = propValue as number
    switch (blendMode){
      case 0:
        this.cssTintColorBlendMode = TintColorBlendMode.CLEAR
      break;
      case 1:
        this.cssTintColorBlendMode = TintColorBlendMode.SRC
        break;
      case 2:
        this.cssTintColorBlendMode = TintColorBlendMode.DST
        break;
      case 3:
        this.cssTintColorBlendMode = TintColorBlendMode.SRC_OVER
        break;
      case 4:
        this.cssTintColorBlendMode = TintColorBlendMode.DST_OVER
        break;
      case 5:
        this.cssTintColorBlendMode = TintColorBlendMode.SRC_IN
        break;
      case 6:
        this.cssTintColorBlendMode = TintColorBlendMode.DST_IN
        break;
      case 7:
        this.cssTintColorBlendMode = TintColorBlendMode.SRC_OUT
        break;
      case 8:
        this.cssTintColorBlendMode = TintColorBlendMode.DST_OUT
        break;
      case 10:
        this.cssTintColorBlendMode = TintColorBlendMode.DST_ATOP
        break;
      case 11:
        this.cssTintColorBlendMode = TintColorBlendMode.XOR
        break;
      case 12:
        this.cssTintColorBlendMode = TintColorBlendMode.ADD
        break;
      case 13:
        this.cssTintColorBlendMode = TintColorBlendMode.MULTIPLY
        break;
      case 14:
        this.cssTintColorBlendMode = TintColorBlendMode.SCREEN
        break;
      case 15:
        this.cssTintColorBlendMode = TintColorBlendMode.OVERLAY
        break;
      case 16:
        this.cssTintColorBlendMode = TintColorBlendMode.DARKEN
        break;
      case 17:
        this.cssTintColorBlendMode = TintColorBlendMode.LIGHTEN
        break
      case 9:
      default:
        this.cssTintColorBlendMode = TintColorBlendMode.SRC_ATOP
        break;
    }
  }

  private setColorFilterMatrix(){
    LogUtils.d(this.TAG,"setColorFilterMatrix")
    if (this.cssTintColor.length != 1) {
      let matrixColor = [0] //RGBA
      matrixColor[0]= Number((this.cssTintColor[0]/0xff).toFixed(2))
      matrixColor[1]= Number((this.cssTintColor[1]/0xff).toFixed(2))
      matrixColor[2]= Number((this.cssTintColor[2]/0xff).toFixed(2))
      matrixColor[3]= Number((this.cssTintColor[3]/0xff).toFixed(2))
      switch (this.cssTintColorBlendMode){
        case TintColorBlendMode.CLEAR:
          this.cssColorFilter = [
            0,  0,  0,  0,  0,
            0,  0,  0,  0,  0,
            0,  0,  0,  0,  0,
            0,  0,  0,  0,  0]
        break
        case TintColorBlendMode.SRC:
          this.cssColorFilter = [
            1,  0,  0,  0,  0,
            0,  1,  0,  0,  0,
            0,  0,  1,  0,  0,
            0,  0,  0,  1,  0]
          break
        case TintColorBlendMode.DST:
          this.cssColorFilter = [
            matrixColor[0], 0, 0, 0, 0,
            0, matrixColor[1], 0, 0, 0,
            0, 0, matrixColor[2], 0, 0,
            0, 0, 0, matrixColor[3], 0]
          break
        case TintColorBlendMode.SRC_OVER:
          this.cssColorFilter = [
            1,  0,  0,  0-matrixColor[0],  matrixColor[0],
            0,  1,  0,  0-matrixColor[1],  matrixColor[1],
            0,  0,  1,  0-matrixColor[2],  matrixColor[2],
            0,  0,  0,  1-matrixColor[3],  matrixColor[3]]
          break
        case TintColorBlendMode.DST_OVER:
          this.cssColorFilter = [
            1-matrixColor[3],  0,  0,  0,  matrixColor[0],
            0,  1-matrixColor[3],  0,  0,  matrixColor[1],
            0,  0,  1-matrixColor[3],  0,  matrixColor[2],
            0,  0,  0,  1-matrixColor[3],  matrixColor[3]]
          break
        case TintColorBlendMode.SRC_IN:
          this.cssColorFilter = [
            matrixColor[3],  0,  0,  0,  0,
            0, matrixColor[3] , 0,  0,  0,
            0, 0, matrixColor[3],  0,  0,
            0, 0, 0,  matrixColor[3],  0]
          break
        case TintColorBlendMode.DST_IN:
          this.cssColorFilter = [
            0,  0,  0,  matrixColor[0],  0,
            0,  0,  0,  matrixColor[1],  0,
            0,  0,  0,  matrixColor[2],  0,
            0,  0,  0,  matrixColor[3],  0]
          break
        case TintColorBlendMode.SRC_OUT:
          this.cssColorFilter = [
            1-matrixColor[3], 0, 0, 0, 0,
            0, 1-matrixColor[3], 0, 0, 0,
            0, 0,  1-matrixColor[3], 0,  0,
            0, 0, 0, 1-matrixColor[3],  0]
          break
        case TintColorBlendMode.DST_OUT:
          this.cssColorFilter = [
            0,  0,  0,  0-matrixColor[0],  matrixColor[0],
            0,  0,  0,  0-matrixColor[1], matrixColor[1],
            0,  0,  0,  0-matrixColor[2],  matrixColor[2],
            0,  0,  0,  0-matrixColor[3],  matrixColor[3]]
          break
        case TintColorBlendMode.DST_ATOP:
          this.cssColorFilter = [
            1-matrixColor[0],  0,  0,  matrixColor[0],  0,
            0,  1-matrixColor[1],  0,  matrixColor[1],  0,
            0,  0,  1-matrixColor[2],  matrixColor[2],  0,
            0,  0,  0,  1,  0]
          break
        case TintColorBlendMode.XOR:
          this.cssColorFilter = [
            1-matrixColor[3],  0,  0,  0-matrixColor[0],  matrixColor[0],
            0,  1-matrixColor[3],  0,  0-matrixColor[1],  matrixColor[1],
            0,  0,  1-matrixColor[3],  0-matrixColor[2],  matrixColor[2],
            0,  0,  0,  1-2*matrixColor[3],  matrixColor[3]]
        case TintColorBlendMode.MULTIPLY:
          this.cssColorFilter = [
            matrixColor[0],  0,  0,  0,  0,
            0,  matrixColor[1],  0,  0,  0,
            0,  0,  matrixColor[2],  0,  0,
            0,  0,  0,  matrixColor[3],  0]
          break
        case TintColorBlendMode.SCREEN:
          this.cssColorFilter = [
            1-matrixColor[0],  0,  0,  0,  matrixColor[0],
            0,  1-matrixColor[1],  0,  0,  matrixColor[1],
            0,  0,  1-matrixColor[2],  0,  matrixColor[2],
            0,  0,  0,  1-matrixColor[3],  matrixColor[3]]
          break
        case TintColorBlendMode.ADD://TODO
        case TintColorBlendMode.OVERLAY://TODO
          this.cssColorFilter = [
            0,  0,  0,  0,  0,
            0,  0,  0,  0,  0,
            0,  0,  0,  0,  0,
            0,  0,  0,  0,  0]
          break
        case TintColorBlendMode.DARKEN://TODO
          {
            this.cssColorFilter = [
              0,  0,  0,  0,  0,
              0,  0,  0,  0,  0,
              0,  0,  0,  0,  0,
              0,  0,  0,  0,  0]
          }
          break
        case TintColorBlendMode.LIGHTEN://TODO
          this.cssColorFilter = [
            0,  0,  0,  0,  0,
            0,  0,  0,  0,  0,
            0,  0,  0,  0,  0,
            0,  0,  0,  0,  0]
          break
        case TintColorBlendMode.SRC_ATOP:
          this.cssColorFilter = [
            matrixColor[3],  0,  0,  0-matrixColor[0],  matrixColor[0],
            0,  matrixColor[3],  0,  0-matrixColor[1],  matrixColor[1],
            0,  0,  matrixColor[3],  0-matrixColor[2],  matrixColor[2],
            0,  0,  0,  0,  matrixColor[3]]
          break
        default:
          break
      }
    }
  }

  setProp(propKey: string, propValue: HippyAny | HippyRenderCallback): boolean {
    console.log(`propKey is : ${propKey},propValue is : ${propValue}`)
    switch (propKey) {
      case 'src':
        {
          let src = propValue as string
          if (src?.startsWith(HRImageView.INTERNET_IMAGE_PREFIX)) {
            let splitList = src.split("/")
            let filePath = this.tempDir +"/" + splitList[splitList.length - 1]
            let failCallback = (err: number) => {
              console.error(`HRImageView:Failed to download the task. Code: ${err}`);
            };
            let progressCallback = (receivedSize: number, totalSize: number) => {
              console.info("HRImageView:download receivedSize:" + receivedSize + " totalSize:" + totalSize);
            };
            let completeCallback = () => {
              console.info('HRImageView:Download task completed.');
            };
            this.imageLoader.setCbFail(failCallback)
            this.imageLoader.setCbComplete(completeCallback)
            this.imageLoader.setCbProgress(progressCallback)
            this.imageLoader.downloadTask(src,filePath)
          }
          this.cssSrc = propValue as string
        }
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
          LogUtils.d(this.TAG, "borderColor: cssOpacity " + this.cssOpacity  + " cssBorderColor: " + this.cssBorderColor)
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
      case 'tintColor'://ARGB
        {
          let rgb = [0]
          let matrixColor = [0]
          let intColor = propValue as number
          this.cssTintColor[0] = (intColor >> 16) & 0xff   //red
          this.cssTintColor[1] = (intColor >> 8) & 0xff    //green
          this.cssTintColor[2] = intColor & 0xff           //blue
          this.cssTintColor[3] = (intColor >> 24) & 0xff   //alpha
          this.setColorFilterMatrix()
        }
        return true
      case 'tintColorBlendMode':
        let blendMode = propValue as number
        this.setTintColorBlendMode(blendMode)
        if (this.cssTintColorBlendMode != this.cssPreTintColorBlendMode) {
          this.setColorFilterMatrix()
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
      // case 'backgroundColor':
      //   this.cssBackgroundColor = propValue as number
      //   this.cssBackgroundColorArray[0] = (this.cssBackgroundColor >> 16) & 0xff   //red
      //   this.cssBackgroundColorArray[1] = (this.cssBackgroundColor >> 8) & 0xff    //green
      //   this.cssBackgroundColorArray[2] = this.cssBackgroundColor & 0xff           //blue
      //   this.cssBackgroundColorArray[3] = (this.cssBackgroundColor >> 24) & 0xff   //alpha
      //   return true
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

  getImage() {
    LogUtils.d(this.TAG,"getImage")
    if (this.cssSrc?.startsWith(HRImageView.BASE64_IMAGE_PREFIX)) {
      return this.cssSrc
    } else if (this.cssSrc?.startsWith(HRImageView.RAW_IMAGE_PREFIX)){
      let splitList = this.cssSrc.split("/")
      this.cssSrc = "vue2/assets/" + splitList[splitList.length - 1]
      LogUtils.d(this.TAG, "getImage: cssSrc= " + this.cssSrc)
      this.cssAlt = this.cssSrc
      return $rawfile(this.cssSrc)
    }
    return this.cssSrc
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
      // .onComplete(() => {
      //   console.log(`hrimageview setTintColorBlendMode`)
      // })
      // .onError(() => {
      //   console.log(`hrimageview setTintColorBlendMode`)
      // })
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
