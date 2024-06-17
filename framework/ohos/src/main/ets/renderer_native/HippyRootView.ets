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
import { HippyEngine } from '../hippy_framework';
import { HippyException } from '../support/common/HippyException';
import { HippyRecord } from '../support/common/HippyTypes';
import { LogUtils } from '../support/utils/LogUtils';
import HippyRenderBaseView from './components/base/HippyRenderBaseView';
import { HRRenderViewCreator } from './components/HippyRenderRegisterMap';
import { FrameworkProxy } from './FrameworkProxy';
import { HippyRender, HippyRenderView } from './HippyRenderView';
import { WrappedHippyCustomRenderViewBuilder } from './HippyRenderViewBuilder';
import { IHippyRenderExceptionListener } from './IHippyRenderExceptionListener';
import { NativeRender } from './NativeRender';
import { NativeRenderContext } from './NativeRenderContext';
import { NativeRenderProvider } from './NativeRenderProvider';
import { HRConvertUtil } from './utils/HRConvertUtil';
import { HRSize } from './utils/HRTypes';

export class HippyRootView {
  renderView: HippyRenderView | null = null
  pagerName: string
  pagerData: HippyRecord
  renderExceptionListener: IHippyRenderExceptionListener | null = null
  private ctx: NativeRenderContext
  private avoidArea: window.AvoidArea | null = null
  private renderViewDidCreateCallback: (() => void) | null = null
  private renderViewSize: HRSize = new HRSize(0, 0)
  private lazyRenderViewTasks: Array<() => void> = []

  constructor(instanceId: number,
              rootId: number,
              customRenderViewCreatorMap: Map<string, HRRenderViewCreator> | null,
              nativeRender: NativeRender,
              wrappedCustomRenderViewBuilder: WrappedHippyCustomRenderViewBuilder | null,
              pagerName: string,
              pagerData: HippyRecord,
              renderExceptionListener: IHippyRenderExceptionListener
  ) {
    this.ctx = new NativeRenderContext(instanceId, rootId, customRenderViewCreatorMap, nativeRender, wrappedCustomRenderViewBuilder)
    this.pagerName = pagerName
    this.pagerData = pagerData
    this.renderExceptionListener = renderExceptionListener
  }

  setRenderExceptionListener(listener: IHippyRenderExceptionListener) {
    this.renderExceptionListener = listener
    if (this.renderView) {
      this.renderView.setRenderExceptionListener(listener)
    }
  }

  private initRenderViewIfNeed() {
    if (!this.renderView && this.renderViewSize.width) {
      this.renderView = new HippyRenderView(this.ctx, this.pagerName, this.pagerData, this.renderViewSize, this.avoidArea, this.renderExceptionListener)
      this.performAllRenderViewTasks()

      // TODO(hot): for c api
      //this.ctx.getNativeRender().attachToRootView(this.renderView.tag)
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
  nativeRender: NativeRender | null = null

  onRenderException(exception: HippyException): void {
    exception = HippyException.safeBuild(exception)
    this.listener?.(exception)
    if (this.nativeRender) {
      this.nativeRender.handleRenderException(exception)
    }
  }
}

@Component
export struct HippyRoot {
  hippyEngine: HippyEngine | null = null
  pagerName: string = ""
  pagerData: HippyRecord = {}
  initViewSize: HRSize = HRConvertUtil.screenSize()
  onRenderException: ((exception: HippyException) => void) | null = null
  private readonly ROOT_VIEW_ID_INCREMENT = 10
  private static sRootIdCounter = 0
  private rootId: number = -1
  private renderInstanceId: number = -1
  private renderExceptionListener = new HippyRenderExceptionListener()
  private wrappedCustomRenderViewBuilder: WrappedHippyCustomRenderViewBuilder | null = null
  @State showRenderView: boolean = false
  private rootView: HippyRootView | null = null
  private enableArkCApi: boolean = false
  onRootViewCreated: ((rootId: number) => void) | null = null

  aboutToAppear(): void {
    if (!this.hippyEngine || !this.hippyEngine.getNativeRenderProvider()) {
      this.onRenderException?.(new HippyException("Error, hippyEngine is null."))
      return
    }

    this.enableArkCApi = this.hippyEngine.getHippyEngineContext()!.getGlobalConfigs().enableArkCApi()
    if (this.enableArkCApi) {
      let libHippy = this.hippyEngine.getHippyEngineContext()!.getLibHippy()
      // TODO(hot):
    }

    if (!this.rootView) {
      this.renderExceptionListener.listener = this.onRenderException
      this.renderExceptionListener.nativeRender = this.hippyEngine.getNativeRenderProvider()!.getNativeRenderImpl()

      HippyRoot.sRootIdCounter += this.ROOT_VIEW_ID_INCREMENT
      this.rootId = HippyRoot.sRootIdCounter
      this.renderInstanceId = this.hippyEngine.getNativeRenderProvider()!.getInstanceId()
      let nativeRender = this.hippyEngine.getNativeRenderProvider()!.getNativeRenderImpl()
      let customRenderViewCreatorMap = nativeRender.getCustomRenderViewCreatorMap()
      this.rootView = new HippyRootView(this.renderInstanceId, this.rootId, customRenderViewCreatorMap, nativeRender,
                                        this.wrappedCustomRenderViewBuilder,
                                        this.pagerName, this.pagerData, this.renderExceptionListener)
      if (this.initViewSize && this.rootView) {
        this.rootView.onViewSizeChanged(this.initViewSize)
      }
      if (this.onRootViewCreated) {
        this.onRootViewCreated(this.rootId)
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
      if (this.hippyEngine) {
        let libHippy = this.hippyEngine.getHippyEngineContext()!.getLibHippy()
        libHippy.NativeRenderProvider_DestroyRoot(this.renderInstanceId, this.rootId)
      }
    }
  }

  build() {
    Column() {
      if (this.enableArkCApi) {
        XComponent({
          id: this.renderInstanceId + "_" + this.rootId,
          type: "node",
          libraryname: 'hippy'
        })
          .width("100%")
          .height("100%")
      } else {
        if (this.showRenderView && this.rootView && this.rootView.renderView) {
          HippyRender({
            renderView: this.rootView.renderView,
            children: this.rootView.renderView.children,
          })
            .width('100%')
            .height('100%')
            .onAppear(() => {
              LogUtils.d("hippy", "did hippy render")
            })
        }
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
