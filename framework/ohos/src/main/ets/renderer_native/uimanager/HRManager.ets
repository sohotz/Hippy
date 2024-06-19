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
import { HippyRenderView } from '../HippyRenderView'
import { HRVirtualNodeManager } from '../node/HRVirtualNodeManager'
import { HRViewManager } from './HRViewManager'

export class HRManager {
  private readonly TAG = "HRManager"
  private viewManagerMap = new Map<number, HRViewManager>()
  private virtualNodeManagerMap = new Map<number, HRVirtualNodeManager>()

  initViewManager(rootView: HippyRenderView) {
    let rootId = rootView.tag
    let viewManager = new HRViewManager(rootView)
    this.addViewManager(rootId, viewManager)
    let nodeManager = new HRVirtualNodeManager()
    this.addVirtualNodeManager(rootId, nodeManager)
    rootView.ctx.getNativeRender().attachToRootView(rootId)
  }

  destroyViewManager(rootId: number) {
    let viewManager = this.getViewManager(rootId)
    if (viewManager) {
      viewManager.getRootRenderView()?.ctx.getNativeRender().destroyRoot(rootId)
    }
    this.removeViewManager(rootId)
    this.removeVirtualNodeManager(rootId)
  }

  addViewManager(rootId: number, viewManager: HRViewManager) {
    this.viewManagerMap.set(rootId, viewManager)
  }

  removeViewManager(rootId: number) {
    this.viewManagerMap.delete(rootId)
  }

  getViewManager(rootId: number) {
    return this.viewManagerMap.get(rootId)
  }

  addVirtualNodeManager(rootId: number, nodeManager: HRVirtualNodeManager) {
    this.virtualNodeManagerMap.set(rootId, nodeManager)
  }

  removeVirtualNodeManager(rootId: number) {
    this.virtualNodeManagerMap.delete(rootId)
  }

  getVirtualNodeManager(rootId: number) {
    return this.virtualNodeManagerMap.get(rootId)
  }
}
