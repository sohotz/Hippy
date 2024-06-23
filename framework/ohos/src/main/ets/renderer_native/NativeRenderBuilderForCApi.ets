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
import { HippyMap } from '../support/common/HippyTypes'
import HippyRenderBaseView from './components/base/HippyRenderBaseView'
import { HRWeb, HRWebView } from './components/web/HRWebView'

export interface CreateRenderViewParamsForCApi {
  rootTag: number
  tag: number
  viewName: string
}

export interface UpdatePropsParamsForCApi {
  rootTag: number
  tag: number
  props: ArrayBuffer | HippyMap
  deleteProps: Array<string>
}

export interface UpdateEventListenerParamsForCApi {
  rootTag: number
  tag: number
  props: ArrayBuffer | HippyMap
}

export interface SetRenderViewFrameParamsForCApi {
  rootTag: number
  tag: number
  left: number
  top: number
  width: number
  height: number
}

@Builder
export function buildHippyRenderViewForCApi($$: HippyRenderBaseView) {
  if ($$ instanceof HRWebView) {
    // TODO(hot):
    HRWeb({
      renderView: $$ as HRWebView,
      children: $$.children,
    })
  } else {
    if ($$.ctx.wrappedCustomRenderViewBuilder) {
      $$.ctx.wrappedCustomRenderViewBuilder.builder($$)
    }
  }
}
