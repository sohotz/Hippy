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
import HippyRenderBaseView, {
  HippyObservedArray,
  HIPPY_COMPONENT_KEY_PREFIX
}  from 'hippy/src/main/ets/renderer_native/components/base/HippyRenderBaseView'
import { HippyCustomComponentView } from 'hippy/src/main/ets/renderer_native/components/custom/HippyCustomComponentView'
import { buildHippyRenderView } from 'hippy/src/main/ets/renderer_native/HippyRenderViewBuilder'
import { NativeRenderContext } from 'hippy/src/main/ets/renderer_native/NativeRenderContext'
import { HRPadding, HRRect } from 'hippy/src/main/ets/renderer_native/utils/HRTypes'
import { HippyAny, HippyRenderCallback } from 'hippy/src/main/ets/support/common/HippyTypes'

@Observed
export class ExampleMenuView extends HippyRenderBaseView {
  constructor(ctx: NativeRenderContext) {
    super(ctx)
  }

}

@Component
export struct ExampleMenuComponent {
  @ObjectLink renderView: ExampleMenuView

  aboutToAppear() {

  }

  aboutToDisappear() {

  }

  build() {
    Stack() {
      Text("This is a menu component.")
    }
  }
}

@Builder
export function buildExampleMenuView($$: HippyRenderBaseView) {
  if ($$ instanceof ExampleMenuView) {
    ExampleMenuComponent({ renderView: $$ as ExampleMenuView })
  }
}
