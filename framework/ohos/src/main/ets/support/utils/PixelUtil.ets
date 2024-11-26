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
declare function vp2px(vp: number): number

// export class DisplayMetrics {
//   public densityScale: number
//   public ohosFontSizeScale: number
//   public density: number
//
//   constructor() {
//     this.densityScale = 1.0
//     this.ohosFontSizeScale = 1.0
//     this.density = vp2px(1.0)
//   }
// }

export class PixelUtil {

  // public static setDisplayMetrics(metrics: DisplayMetrics): void {
  //
  // }

  public static setDensityScale(densityScale: number): void {
    PixelUtil.densityScale = densityScale
    PixelUtil.density = vp2px(1.0) / PixelUtil.ohosFontSizeScale / PixelUtil.densityScale
  }

  public static setOhosFontSizeScale(ohosFontSizeScale: number): void {
    PixelUtil.ohosFontSizeScale = ohosFontSizeScale
    PixelUtil.density = vp2px(1.0) / PixelUtil.ohosFontSizeScale / PixelUtil.densityScale
  }

  public static px2dp(value: number): number {
    return value / PixelUtil.getDensity();
  }

  public static getDensity(): number {
    return PixelUtil.density
  }

  private static densityScale: number = 1.0
  private static ohosFontSizeScale: number = 1.0
  private static density: number = vp2px(1.0) /// 2
}
