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
  HippyRenderCallback } from '../../../support/common/HippyTypes';
import { HRNodeProps } from '../../dom_node/HRNodeProps';
import { NativeRenderContext } from '../../NativeRenderContext';
import { HRConvertUtil } from '../../utils/HRConvertUtil';
import HippyRenderBaseView, { HIPPY_COMPONENT_KEY_PREFIX } from '../base/HippyRenderBaseView';
import { LogUtils } from '../../../support/utils/LogUtils';
import {HRDownloadUtils} from '../../utils/HRDownloadUtils'
import { HREventUtils } from '../../utils/HREventUtils';

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
  private imageLoader: HRDownloadUtils = new HRDownloadUtils
  tempDir:string = getContext().tempDir

  cssSrc: string | null = null
  cssAlt: string = ""
  cssResizeMode: ImageFit = ImageFit.Cover
  cssTintColor: number[] = [0] // RGBA
  cssColorFilter: ColorFilter = [0]
  cssTintColorBlendMode: TintColorBlendMode = TintColorBlendMode.DST
  cssPreTintColorBlendMode: TintColorBlendMode = TintColorBlendMode.DST
  onCompleteCallback: ((event?: ImageLoadEvent) => void) | null = null
  onErrorCallback: ((error: ImageError) => void) | null = null

  constructor(ctx: NativeRenderContext) {
    super(ctx)
    LogUtils.d(this.TAG, "constructor")
    this.initCallbacks()
  }

  private setResizeMode(pvalue: string) {
    LogUtils.d(this.TAG, "setResizeMode")
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
      case 'origin':
        this.cssResizeMode = ImageFit.None
        break;
      case 'repeat':
        this.cssResizeMode = ImageFit.Contain
        break;
      case 'fitXY':
        this.cssResizeMode = ImageFit.Fill
        break;
      default:
        console.log(`unknown resizeMode : ${pvalue}`)
        this.cssResizeMode = ImageFit.Fill
        break;
    }
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
      default:
        break
    }
    return super.setProp(propKey, propValue as HippyAny)
  }

  private initCallbacks() {
    if (!this.onCompleteCallback) {
      this.onCompleteCallback = (event) => {
        if (event?.loadingStatus !== 1) {
          return
        }
        // send onLoad event
        HREventUtils.sendComponentEvent(this.ctx, this.tag, HREventUtils.EVENT_IMAGE_ON_LOAD, null)
        let params = new Map<string, HippyAny>()
        params.set("success", 1)
        let imageSize = new Map<string, number>()
        imageSize.set("width", event?.width)
        imageSize.set("height", event?.height)
        params.set("image", imageSize)
        // send onLoadEnd event
        HREventUtils.sendComponentEvent(this.ctx, this.tag, HREventUtils.EVENT_IMAGE_LOAD_END, params as HippyAny)
      }
    }
    if (!this.onErrorCallback) {
      this.onErrorCallback = (error: ImageError) => {
        LogUtils.d(this.TAG, `onErrorCallback, w: ${error.componentWidth}, h: ${error.componentHeight}, msg: ${error.message}`)
        // send onError event
        HREventUtils.sendComponentEvent(this.ctx, this.tag, HREventUtils.EVENT_IMAGE_LOAD_ERROR, null)
        let params = new Map<string, number>()
        params.set("success", 0)
        // send onLoadEnd event
        HREventUtils.sendComponentEvent(this.ctx, this.tag, HREventUtils.EVENT_IMAGE_LOAD_END, params)
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
      .alt($rawfile(this.renderView.cssAlt))
      .objectFit(this.renderView.cssResizeMode)
      .colorFilter(this.renderView.cssColorFilter)
      .onComplete(this.renderView.onCompleteCallback)
      .onError(this.renderView.onErrorCallback)
  }
}

// base props for all components
@Extend(Image)
function applyRenderViewBaseAttr($$: HippyRenderBaseView) {
  .key(HIPPY_COMPONENT_KEY_PREFIX + $$.tag)
  .backgroundColor($$.cssBackgroundColor)
  .position({x: $$.cssPositionX, y: $$.cssPositionY})
  .size({width:$$.cssWidth, height: $$.cssHeight})
  .opacity($$.cssOpacity)
  .clip($$.cssOverflow)
  .visibility(($$ as HippyRenderBaseView).cssVisibility) // must add as, otherwise the compiler has error
  .zIndex($$.cssZIndex)
  .accessibilityText($$.cssAccessibilityLabel)
  .focusable($$.cssFocusable)
  .border($$.cssBorder)
  .shadow($$.cssShadow)
  .linearGradient($$.cssLinearGradient)
  .backgroundImage($$.cssBackgroundImage)
  .backgroundImageSize($$.cssBackgroundImageSize)
  .backgroundImagePosition($$.cssBackgroundImagePosition)
  .transform($$.cssMatrix)
  .rotate($$.cssRotate)
  .scale($$.cssScale)
  .translate($$.cssTranslate)
  .hitTestBehavior($$.hitTestBehavior)
  .onClick($$.eventClick)
}
