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
import {
  createHippyEngine,
  EngineInitParams,
  EngineInitStatus, HippyEngine, HippyRoot, ModuleLoadParams, ModuleLoadStatus
} from 'hippy'
import { HippyLibrary } from 'hippy/src/main/ets/hippy_library/HippyLibrary'
import HippyRenderBaseView
  from 'hippy/src/main/ets/renderer_native/components/base/HippyRenderBaseView'
import { HippyException } from 'hippy/src/main/ets/support/common/HippyException'
import { HippyValue } from 'hippy/src/main/ets/support/common/HippyTypes'
import { LogUtils } from 'hippy/src/main/ets/support/utils/LogUtils'
import { ExampleAPIProvider } from '../hippyExtend/ExampleAPIProvider'
import { ExampleCustomComponent, ExampleCustomComponentView } from '../hippyExtend/ExampleCustomComponentView'
import { DriverMode, InitParams } from './PageConfiguration'

@Entry
@Component
struct ExampleHippyPage {
  @StorageLink('libHippy') private libHippy: HippyLibrary | null = null
  @StorageLink('abilityContext') private abilityContext: Context | null = null
  @State exception: string = ""
  private hippyEngine: HippyEngine | null = null

  aboutToAppear(): void {
    let configParams = router.getParams() as InitParams

    let params = new EngineInitParams(this.libHippy!, this.abilityContext!)
    params.providers = new Array(new ExampleAPIProvider())

    params.coreJSAssetsPath = "react/vendor.ohos.js"
    if (configParams.driver === DriverMode.JS_VUE_2) {
      params.coreJSAssetsPath = "vue2/vendor.ohos.js"
    } else if (configParams.driver === DriverMode.JS_VUE_3) {
      params.coreJSAssetsPath = "vue3/vendor.ohos.js"
    }

    this.hippyEngine = createHippyEngine(params)
    this.hippyEngine.initEngine((statusCode: EngineInitStatus, msg: string) => {
      if (statusCode == EngineInitStatus.STATUS_OK) {
        let jsAssetPath = 'react/index.ohos.js'
        if (configParams.driver === DriverMode.JS_VUE_2) {
          jsAssetPath = "vue2/index.ohos.js"
        } else if (configParams.driver === DriverMode.JS_VUE_3) {
          jsAssetPath = "vue3/index.ohos.js"
        }

        let loadParams = new ModuleLoadParams(
          jsAssetPath,
            '',
            '',
            '',
            null,
            null,
            null,
          )
        loadParams.componentName = "Demo"
        loadParams.codeCacheTag = "Demo"
        loadParams.jsParams = new Map<string, HippyValue>()
        loadParams.jsParams.set(
          "msgFromNative",
          "Hi js developer, I come from ohos native code!"
        )
        this.hippyEngine?.loadModule(
          loadParams,
          (statusCode: ModuleLoadStatus, msg: string) => {

        })
      }
    })
  }

  aboutToDisappear(): void {

  }

  @Builder Title() {
    Row() {
      RelativeContainer() {
        Row() {
          Image($r('app.media.back_2x'))
            .width(9)
            .height(17)
            .margin({ left: 10})
        }
        .width(55)
        .height(54)
        .onClick(() => {
          router.back()
        })
        .id('btn_back')
        .justifyContent(FlexAlign.Center)

        Text("Demo")
          .fontColor(Color.Black)
          .fontWeight(500)
          .fontSize(17)
          .id('title')
          .alignRules({ middle: { anchor: "__container__", align: HorizontalAlign.Center},
            center: { anchor: "__container__", align: VerticalAlign.Center}})
      }
    }
    .height(54)
    .backgroundColor(Color.White)
    .width('100%')
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
