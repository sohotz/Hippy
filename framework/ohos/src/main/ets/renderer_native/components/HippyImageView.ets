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

import { NativeRenderContext } from '../NativeRenderContext';
import { HippyViewBase, ViewBaseProps } from './HippyViewBase';
import { ColorSegments, Descriptor } from '../descriptor/DescriptorBase';
import { getTintColorMatrix } from '../descriptor/DescriptorUtils';

export class ImageProps extends ViewBaseProps {
  uri?: string
  resizeMode?: number
  tintColor?: ColorSegments
  blurRadius?: number
}

export type ImageDescriptor = Descriptor<"Image", ImageProps>

@Component
export struct HippyImageView {
  ctx!: NativeRenderContext
  tag!: number
  @State descriptor: ImageDescriptor = {} as ImageDescriptor
  defaultColorFilter: number[] = [
    1, 0, 0, 0, 0,
    0, 1, 0, 0, 0,
    0, 0, 1, 0, 0,
    0, 0, 0, 1, 0,
  ]
  private unregisterDescriptorChangesListener?: () => void = undefined

  aboutToAppear() {
    this.descriptor = this.ctx.descriptorRegistry.getDescriptor<ImageDescriptor>(this.tag)
    this.onLoadStart()
    this.unregisterDescriptorChangesListener = this.ctx.descriptorRegistry.subscribeToDescriptorChanges(this.tag,
      (newDescriptor) => {
        const newImageDescriptor = (newDescriptor as ImageDescriptor)
        if(newImageDescriptor.props.uri !== this.descriptor.props.uri) {
          this.onLoadStart()
        }
        this.descriptor = newImageDescriptor
      }
    )
  }

  aboutToDisappear() {
    this.unregisterDescriptorChangesListener?.()
  }

  onLoadStart() {
    // TODO(hot):
  }

  getSource(): Resource | string | undefined {
    const uri = this.descriptor.props.uri
    if (uri && uri.startsWith("asset://")) {
      return $rawfile(uri.replace("asset://", "assets/"))
    }

    // TODO(hot):
    // const filePath: string = null
    // return filePath ?? uri

    return uri
  }

  getResizeMode(resizeMode: number): ImageFit {
    switch(resizeMode) {
      case 0:
        return ImageFit.Cover;
      case 1:
        return ImageFit.Contain;
      case 2:
        return ImageFit.Fill;
      case 3:
      case 4:
        return ImageFit.None;
      default:
        return ImageFit.Cover;
    }
  }

  getColorFilter(): number[] {
    if (!this.descriptor.props.tintColor || this.descriptor.props.tintColor.every((element) => element === 0)){
      return this.defaultColorFilter;
    }
    return getTintColorMatrix(this.descriptor.props.tintColor)
  }

  build() {
    HippyViewBase({
      ctx: this.ctx,
      tag: this.tag,
    }) {
      Image(this.getSource())
        .interpolation(ImageInterpolation.High)
        .width("100%")
        .height("100%")
        //.objectFit(this.getResizeMode(this.descriptor.props.resizeMode))
        .objectRepeat(this.descriptor.props.resizeMode === 4 ? ImageRepeat.XY : ImageRepeat.NoRepeat )
        .colorFilter(this.getColorFilter())
        .blur(this.descriptor.props.blurRadius)
    }
  }
}
