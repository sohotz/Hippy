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
import { EngineInitStatus, HippyEngine, HippyRoot, ModuleLoadParams, ModuleLoadStatus
} from 'hippy'
import HippyRenderBaseView
  from 'hippy/src/main/ets/renderer_native/components/base/HippyRenderBaseView'
import { HippyException } from 'hippy/src/main/ets/support/common/HippyException'
import { ExampleCustomComponent, ExampleCustomComponentView } from '../components/ExampleCustomComponentView'

@Builder
function buildCustomRenderView($$: HippyRenderBaseView) {
  if ($$ instanceof ExampleCustomComponentView) {
    ExampleCustomComponent({ renderView: $$ as ExampleCustomComponentView, children: $$.children, buildRenderView: buildCustomRenderView })
  }
}

@Entry
@Component
struct Index {
  @StorageLink('HippyEngine') private hippyEngine: HippyEngine | null = null
  @State exception: string = ""

  aboutToAppear(): void {
    this.hippyEngine?.initEngine((statusCode: EngineInitStatus, msg: string) => {
      if (statusCode == EngineInitStatus.STATUS_OK) {
        this.hippyEngine?.loadModule(new ModuleLoadParams(), (statusCode: ModuleLoadStatus, msg: string) => {

        })
      }
    })
  }

  aboutToDisappear(): void {

  }

  build() {
    Stack() {
      HippyRoot({
        hippyEngine: this.hippyEngine,
        pagerName: 'demo',
        pagerData: {},
        buildCustomRenderView: buildCustomRenderView,
        onRenderException: (exception: HippyException) => {
          this.exception = `${exception.message}\n${exception.stack}`
        },
      })
      if (this.exception) {
        Text(this.exception)
      }
    }
    .width('100%')
    .height('100%')
    .expandSafeArea([SafeAreaType.KEYBOARD])
  }
}
