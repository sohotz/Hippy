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

export class DevFloatButton{
  private mWidth: number;

  constructor(context: Context) {
    super();
    this.mWidth = this.dip2px(context) as number;
    this.buildBackground();
    // setFocusable(true);
  }

  private dip2px(context: Context): number {
    let displayClass: display.Display = display.getDefaultDisplaySync();
    return displayClass.densityPixels
    // DisplayMetrics displayMetrics = context.getResources().getDisplayMetrics();
    // return TypedValue.applyDimension(TypedValue.COMPLEX_UNIT_DIP, SIZE, displayMetrics);
  }

  private buildBackground() {
    this.buildDrawableState();
  }

  /**
   * 构建背景Drawble
   */
  private buildDrawableState() {

    //   let mNormalState = new int[];
    // let mFocusedSate = new int[]{android.R.attr.state_focused, android.R.attr.state_enabled};

    //默认文字和背景颜色
    // let mBgNormalColor = Color.parseColor("#ddd9d9");
    // let mBgFocusedColor = Color.GREEN;
    //创建状态管理器
    // StateListDrawable drawable = new StateListDrawable();
    // int radius = mWidth / 2;
    // float[] outRect = new float[]{radius, radius, radius, radius, radius, radius, radius,
    // radius};
    // RoundRectShape rectShape = new RoundRectShape(outRect, null, null);

    //创建圆弧形状
    //创建drawable
    // let pressedDrawable = new ShapeDrawable(rectShape);
    //设置我们按钮背景的颜色
    // pressedDrawable.getPaint().setColor(mBgFocusedColor);
    //添加到状态管理里面
    // drawable.addState(mFocusedSate, pressedDrawable);

    // ShapeDrawable normalDrawable = new ShapeDrawable(rectShape);
    // normalDrawable.getPaint().setColor(mBgNormalColor);
    // drawable.addState(mNormalState, normalDrawable);
    //设置我们的背景，就是xml里面的selector
    // this.setBackgroundDrawable(drawable);

    // Looper.getMainLooper().myQueue().addIdleHandler(new MessageQueue.IdleHandler() {
    //             @Override
    //             public boolean queueIdle() {
    //                 boolean result = requestFocusFromTouch();
    //                 LogUtils.d("requestFocus", "requestFocusFromTouch result:" + result);
    //                 if (!result) {
    //                     result = requestFocus();
    //                     LogUtils.d("requestFocus", "requestFocus result:" + result);
    //                 }
    // return false;
    // }
    // });
  }
}
