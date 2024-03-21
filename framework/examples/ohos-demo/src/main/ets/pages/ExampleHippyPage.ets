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
import { router } from '@kit.ArkUI'
import { EngineInitStatus, HippyEngine, HippyRoot, ModuleLoadParams, ModuleLoadStatus
} from 'hippy'
import HippyRenderBaseView
  from 'hippy/src/main/ets/renderer_native/components/base/HippyRenderBaseView'
import { HippyException } from 'hippy/src/main/ets/support/common/HippyException'
import { ExampleCustomComponent, ExampleCustomComponentView } from '../hippyExtend/ExampleCustomComponentView'

@Entry
@Component
struct ExampleHippyPage {
  @StorageLink('HippyEngine') private hippyEngine: HippyEngine | null = null
  @State exception: string = ""

  aboutToAppear(): void {
    this.hippyEngine?.initEngine((statusCode: EngineInitStatus, msg: string) => {
      if (statusCode == EngineInitStatus.STATUS_OK) {
        this.hippyEngine?.loadModule(
          new ModuleLoadParams(
            'vue2/index.android.js',
            '',
            '',
            '',
            null,
            null,
            null,
          ),
          (statusCode: ModuleLoadStatus, msg: string) => {

        })
      }
    })
  }

  aboutToDisappear(): void {

  }

  @Builder Title() {
    Row() {
      Image($r('app.media.ic_back'))
        .width(24)
        .height(24)
        .margin({
          left: 24,
          right: 16
        })
        .onClick(() => {
          router.back()
        })
      Text("Demo")
        .fontColor(0xFF000000)
        .fontSize(20)
    }
    .width('100%')
    .height(56)
  }

  @Builder
  buildCustomRenderView($$: HippyRenderBaseView) {
    if ($$ instanceof ExampleCustomComponentView) {
      ExampleCustomComponent({ renderView: $$ as ExampleCustomComponentView, children: $$.children })
    }
  }

  build() {
    Column() {
      this.Title()
      HippyRoot({
        hippyEngine: this.hippyEngine,
        pagerName: 'demo',
        pagerData: {},
        buildCustomRenderView: this.buildCustomRenderView,
        onRenderException: (exception: HippyException) => {
          this.exception = `${exception.message}\n${exception.stack}`
        },
      })
        .layoutWeight(1)
      if (this.exception) {
        Text(this.exception)
      }
    }
    .width('100%')
    .height('100%')
    .expandSafeArea([SafeAreaType.KEYBOARD])
  }
}
