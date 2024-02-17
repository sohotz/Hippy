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
import window from '@ohos.window';
import { HippyAbility, HippyEngine } from '../hippy_framework';
import { HippyException } from '../support/common/HippyException';
import { HippyRecord } from '../support/common/HippyTypes';
import HippyRenderBaseView from './components/base/HippyRenderBaseView';
import { FrameworkProxy } from './FrameworkProxy';
import { HippyRender, HippyRenderView } from './HippyRenderView';
import { IHippyRenderExceptionListener } from './IHippyRenderExceptionListener';
import { NativeRenderContext } from './NativeRenderContext';
import { HRConvertUtil } from './utils/HRConvertUtil';
import { HRSize } from './utils/HRTypes';

export class HippyRootView {
  ctx: NativeRenderContext
  renderView: HippyRenderView | null = null
  pagerName: string
  pagerData: HippyRecord
  renderExceptionListener: IHippyRenderExceptionListener | null = null
  private avoidArea: window.AvoidArea | null = null
  private renderViewDidCreateCallback: (() => void) | null = null
  private renderViewSize: HRSize = new HRSize(0, 0)
  private lazyRenderViewTasks: Array<() => void> = []

  constructor(ctx: NativeRenderContext,
              pagerName: string,
              pagerData: HippyRecord,
              renderExceptionListener: IHippyRenderExceptionListener
  ) {
    this.ctx = ctx
    this.pagerName = pagerName
    this.pagerData = pagerData
    this.renderExceptionListener = renderExceptionListener
  }

  private initRenderViewIfNeed() {
    if (!this.renderView && this.renderViewSize.width) {
      this.renderView = new HippyRenderView(this.ctx, this.pagerName, this.pagerData, this.renderViewSize, this.avoidArea)
      this.performAllRenderViewTasks()
    }
    this.fireDidCreateRenderViewCallbackIfNeed()
  }

  setAvoidArea(avoidArea: window.AvoidArea) {
    this.avoidArea = avoidArea
  }

  onViewSizeChanged(size: HRSize) {
    if (this.renderViewSize.width == size.width && this.renderViewSize.height == size.height) {
      return
    }
    this.renderViewSize = size
    let hasRenderView = this.renderView !== null
    this.initRenderViewIfNeed()
    if (hasRenderView && this.renderView) {
      this.renderView.onSizeChanged(size.width, size.height)
    }
  }

  setDidCreateRenderViewCallback(callback: () => void) {
    if (this.renderView) {
      callback()
    } else {
      this.renderViewDidCreateCallback = callback
    }
  }

  pageDidAppear() {
    this.performRenderViewTask(() => {
      if (this.renderView) {
        this.renderView.pageDidAppear()
      }
    })
  }

  pageDidDisappear() {
    this.performRenderViewTask(() => {
      if (this.renderView) {
        this.renderView.pageDidDisappear()
      }
    })
  }

  onDestroy() {

  }

  private performRenderViewTask(task: () => void) {
    if (this.renderView) {
      task()
    } else {
      this.lazyRenderViewTasks.push(task)
    }
  }

  private performAllRenderViewTasks() {
    for (let index = 0; index < this.lazyRenderViewTasks.length; index++) {
      const task = this.lazyRenderViewTasks[index];
      task()
    }
    this.lazyRenderViewTasks = []
  }

  private fireDidCreateRenderViewCallbackIfNeed() {
    if (this.renderViewDidCreateCallback && this.renderView) {
      this.renderViewDidCreateCallback()
      this.renderViewDidCreateCallback = null
    }
  }
}

class HippyRenderExceptionListener implements IHippyRenderExceptionListener {
  listener: ((exception: HippyException) => void) | null = null
  frameworkProxy: FrameworkProxy | null = null

  onRenderException(exception: HippyException): void {
    this.listener?.(exception)
    if (this.frameworkProxy) {
      this.frameworkProxy.handleNativeException(exception)
    }
  }
}

@Component
export struct HippyRoot {
  pagerName: string = ""
  pagerData: HippyRecord = {}
  initViewSize: HRSize | null = HRConvertUtil.screenSize()
  onRenderException: ((exception: HippyException) => void) | null = null
  private renderExceptionListener = new HippyRenderExceptionListener()
  @BuilderParam buildCustomRenderView: ($$: HippyRenderBaseView) => void
  @State showRenderView: boolean = false
  private rootView: HippyRootView | null = null

  // TODO(hot):
  @StorageLink('HippyAbility') private hippyAbility: HippyAbility | null = null
  public hippyEngine: HippyEngine | null = null

  aboutToAppear(): void {
    this.hippyEngine = this.getOrCreateHippyEngine()
    if (this.hippyEngine && this.hippyAbility) {
      this.hippyEngine.runJsBundle(this.hippyAbility.context.resourceManager)
    }

    if (!this.rootView && this.hippyEngine) {
      this.renderExceptionListener.listener = this.onRenderException
      this.renderExceptionListener.frameworkProxy = null // TODO(hot):
      this.rootView = new HippyRootView(this.hippyEngine.getNativeRenderContext(), this.pagerName, this.pagerData, this.renderExceptionListener)//, this.communicator, this.delegate, this.renderExceptionListener)
      if (this.initViewSize && this.rootView) {
        this.rootView.onViewSizeChanged(this.initViewSize)
      }

      this.rootView.setDidCreateRenderViewCallback(() => {
        this.showRenderView = true
      })

      window.getLastWindow(getContext(this), (error, data) => {
        if (this.rootView && data) {
          let avoidArea = data.getWindowAvoidArea(window.AvoidAreaType.TYPE_SYSTEM)
          this.rootView.setAvoidArea(avoidArea)
        }
      })
    }
  }

  aboutToDisappear(): void {
    if (this.rootView) {
      this.rootView.onDestroy()
    }
  }

  private getOrCreateHippyEngine(): HippyEngine | null {
    if (this.hippyAbility) {
      return this.hippyAbility.getHippyEngine()
    }
    return null
  }

  build() {
    Column() {
      if (this.showRenderView && this.rootView && this.rootView.renderView) {
        HippyRender({
          renderView: this.rootView.renderView,
          children: this.rootView.renderView.children,
          buildCustomRenderView: this.buildCustomRenderView
        })
          .width('100%')
          .height('100%')
          .onAppear(() => {
            console.log("did hippy render")
          })
      } else {

      }
    }
    .width('100%')
    .height('100%')
    .onAreaChange((oldValue: Area, newValue: Area) => {
      if (this.rootView) {
        this.rootView.onViewSizeChanged(new HRSize(newValue.width as number, newValue.height as number))
      }
    })
  }
}
