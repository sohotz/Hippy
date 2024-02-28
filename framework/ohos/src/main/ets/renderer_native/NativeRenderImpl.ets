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
import TextMeasurer from "@ohos.measure"
import { HippyException } from '../support/common/HippyException'
import { HippyExecutor } from '../support/common/HippyExecutor'
import { HippyAny, HippyMap, HippyRenderCallback } from '../support/common/HippyTypes'
import { LogUtils } from '../support/utils/LogUtils'
import { VfsManager } from '../vfs/VfsManager'
import { ImageDecoderAdapter } from './components/image/ImageDecoderAdapter'
import { ImageLoader } from './components/image/ImageLoader'
import { ImageLoaderAdapter } from './components/image/ImageLoaderAdapter'
import { FontAdapter } from './components/richtext/FontAdapter'
import { FrameworkProxy } from './FrameworkProxy'
import { NativeRender } from './NativeRender'
import { NativeRenderContext } from './NativeRenderContext'
import { HRVirtualNode } from './node/HRVirtualNode'
import { HRManager } from './uimanager/HRManager'
import { HRCreateMutation,
  HRDeleteMutation,
  HRMove2Mutation,
  HRMoveMutation,
  HRMutation,
  HRUpdateLayoutMutation,
  HRUpdateMutation
} from './uimanager/HRMutation'
import { HRSize } from './utils/HRTypes'
import { HRValueUtils } from './utils/HRValueUtils'

export class TextMeasurerConfig {
  textContent: string | null = null
  fontSize: number | null = null
  lineHeight: number | null = null
  fontWeight: number | null = null
  maxWidth: number | null = null
  numberOfLines: number | null = null
  letterSpacing?: number | null = null
}

declare function px2vp(px: number): number

export interface NativeRenderEventHandler {
  onSize(rootId: number, width: number, height: number): void
  onSize2(rootId: number, nodeId: number, width: number, height: number, isSync: boolean): void
}

export class NativeRenderImpl implements NativeRender {
  private readonly TAG = "NativeRenderImpl"
  private readonly ROOT_VIEW_ID_INCREMENT = 10
  private readonly INVALID_NODE_ID = -1

  private static sRootIdCounter = 0

  private eventHandler: NativeRenderEventHandler

  private frameworkProxy: FrameworkProxy | null = null
  private imageLoader: ImageLoaderAdapter | null = null

  private ctx: NativeRenderContext
  private mutations: HRMutation[] = []
  private virtualNodes: Map<number, HRVirtualNode>

  private renderManager: HRManager

  constructor(eventHandler: NativeRenderEventHandler) {
    this.eventHandler = eventHandler
    NativeRenderImpl.sRootIdCounter += this.ROOT_VIEW_ID_INCREMENT
    let rootId = NativeRenderImpl.sRootIdCounter
    this.ctx = new NativeRenderContext(rootId, this)
    this.virtualNodes = new Map<number, HRVirtualNode>()

    this.renderManager = new HRManager(this.ctx)
  }

  setFrameworkProxy(proxy: FrameworkProxy) {
    this.frameworkProxy = proxy
  }

  getNativeRenderContext(): NativeRenderContext {
    return this.ctx
  }

  getRenderManager(): HRManager {
    return this.renderManager
  }

  getBundlePath(): string {
    return this.frameworkProxy ? this.frameworkProxy.getBundlePath() : ""
  }

  getImageLoader(): ImageLoaderAdapter | null{
    if (this.imageLoader == null && this.getVfsManager() != null) {
      this.imageLoader = new ImageLoader(this.getVfsManager() as VfsManager)
    }
    return this.imageLoader
  }

  getImageDecoderAdapter(): ImageDecoderAdapter | null {
    return this.frameworkProxy ? this.frameworkProxy.getImageDecoderAdapter() : null
  }

  getVfsManager(): VfsManager | null {
    return this.frameworkProxy ? this.frameworkProxy.getVfsManager() : null
  }

  getFontAdapter(): FontAdapter | null {
    return this.frameworkProxy ? this.frameworkProxy.getFontAdapter() : null
  }

  getBackgroundExecutor(): HippyExecutor | null {
    return this.frameworkProxy ? this.frameworkProxy.getBackgroundExecutor() : null
  }

  getEngineId(): number {
    return this.frameworkProxy ? this.frameworkProxy.getEngineId() : -1
  }

  onSizeChanged(rootId: number, width: number, height: number, oldWidth: number, oldHeight: number): void {
    if (this.frameworkProxy != null) {
      this.frameworkProxy.onSizeChanged(rootId, width, height, oldWidth, oldHeight)
    }
    this.eventHandler.onSize(rootId, width, height)
  }

  onSizeChanged2(rootId: number, nodeId: number, width: number, height: number, isSync: boolean): void {
    this.eventHandler.onSize2(rootId, nodeId, width, height, isSync)
  }

  updateDimension(width: number, height: number, shouldUseScreenDisplay: boolean, systemUiVisibilityChanged: boolean) {
    if (this.frameworkProxy != null) {
      this.frameworkProxy.updateDimension(width, height, shouldUseScreenDisplay, systemUiVisibilityChanged)
    }
  }

  handleRenderException(exception: HippyException) {
    exception = HippyException.safeBuild(exception)
    LogUtils.e(this.TAG, `handleRenderException: ${exception.message}`)
    if (this.frameworkProxy) {
      this.frameworkProxy.handleNativeException(exception)
    }
  }

  createVirtualNode(rootId: number, id: number, pid: number, index: number, props: HippyMap): HRVirtualNode {
    let node = new HRVirtualNode()
    node.rootId = rootId
    node.id = id
    node.pid = pid
    node.index = index
    node.props = props
    return node
  }

  addVirtualNode(id: number, node: HRVirtualNode) {
    this.virtualNodes.set(id, node)
  }

  removeVirtualNode(id: number) {
    this.virtualNodes.delete(id)
  }

  createNode(rootId: number, nodeList: Array<HippyAny>) {
    for (let i = 0; i < nodeList.length; i++) {
      let node = nodeList[i] as Map<string, HippyAny>

      let vNode = this.createVirtualNode(
        rootId,
        node.get('id') as number,
        node.get('pId') as number,
        node.get('index') as number,
        node.get('props') as HippyMap)
      this.addVirtualNode(node.get('id') as number, vNode)

      let m: HRCreateMutation = new HRCreateMutation()
      m.viewName = node.get('name') as string
      m.tag = node.get('id') as number
      m.parentTag = node.get('pId') as number
      m.index = node.get('index') as number
      m.props = node.get('props') as HippyMap
      this.mutations.push(m)
    }
  }

  updateNode(rootId: number, nodeList: Array<HippyAny>) {
    for (let i = 0; i < nodeList.length; i++) {
      let node = nodeList[i] as Map<string, HippyAny>
      let m: HRUpdateMutation = new HRUpdateMutation()
      m.viewName = node.get('name') as string
      m.tag = node.get('id') as number
      m.parentTag = node.get('pId') as number
      m.index = node.get('index') as number
      m.props = node.get('props') as HippyMap
      this.mutations.push(m)
    }
  }

  moveNode(rootId: number, pid: number, nodeList: Array<HippyAny>) {
    for (let i = 0; i < nodeList.length; i++) {
      let node = nodeList[i] as Map<string, HippyAny>
      let m: HRMoveMutation = new HRMoveMutation()
      m.tag = node.get('id') as number
      m.parentTag = node.get('pId') as number
      m.index = node.get('index') as number
      this.mutations.push(m)
    }
  }

  moveNode2(rootId: number, id_array: Array<number>, to_pid: number, from_pid: number, index: number) {
    for (let i = 0; i < id_array.length; i++) {
      let id = id_array[i]
      let m: HRMove2Mutation = new HRMove2Mutation()
      m.tag = id
      m.toParentTag = to_pid
      m.fromParentTag = from_pid
      m.index = index
      this.mutations.push(m)
    }
  }

  deleteNode(rootId: number, id_array: Array<number>) {
    for (let i = 0; i < id_array.length; i++) {
      let id = id_array[i]
      let m: HRDeleteMutation = new HRDeleteMutation()
      m.tag = id
      this.mutations.push(m)
    }
  }

  updateLayout(rootId: number, nodeList: Array<HippyAny>) {
    for (let i = 0; i < nodeList.length; i++) {
      let node = nodeList[i] as Map<string, HippyAny>
      let m: HRUpdateLayoutMutation = new HRUpdateLayoutMutation()
      m.tag = node.get('id') as number
      m.left = px2vp(node.get('left') as number)
      m.top = px2vp(node.get('top') as number)
      m.width = px2vp(node.get('width') as number)
      m.height = px2vp(node.get('height') as number)
      this.mutations.push(m)
    }
  }

  endBatch(rootId: number) {
    this.renderManager.applyMutations(this.mutations)
    this.mutations.splice(0, this.mutations.length)
  }

  updateEventListener(rootId: number, eventList: Array<HippyAny>) {
    for (let i = 0; i < eventList.length; i++) {
      let event = eventList[i] as Map<string, HippyAny>
      if (!(event instanceof Map)) {
        throw new HippyException(this.TAG + ": updateEventListener: invalid event object")
      }
      let nodeId = HRValueUtils.getNumber2(event.get('id'), this.INVALID_NODE_ID)
      let eventProps = HRValueUtils.getMap(event.get('props'))
      if (nodeId < 0 || !eventProps) {
        throw new HippyException(this.TAG + ": updateEventListener: invalid id or props")
      }


    }
  }

  callUIFunction(nodeId: number, functionName: string, params: Array<HippyAny>, callback: HippyRenderCallback | null) {
    LogUtils.d(this.TAG,  "callUIFunction: id " + nodeId + ", functionName " + functionName + ", params" + params)
    this.renderManager.callViewMethod(nodeId, functionName, params, callback)
  }

  measure(rootId: number, nodeId: number, width: number, widthMode: number, height: number, heightMode: number): bigint {
    let vNode = this.virtualNodes.get(nodeId)
    if (vNode && vNode.props) {
      let config: TextMeasurerConfig = new TextMeasurerConfig();
      config.textContent = vNode.props.get('text') as string
      config.fontSize = vNode.props.get('fontSize') as number
      config.lineHeight = vNode.props.get('lineHeight') as number
      config.numberOfLines = 1
      let result = this.measureText(config)
      return (BigInt(result.width) << BigInt(32)) | BigInt(result.height)
    }
    return BigInt(0)
  }

  private measureText(config: TextMeasurerConfig): HRSize {
    let textSize = TextMeasurer.measureTextSize({
      textContent: config.textContent as string,
      fontSize: config.fontSize,
      lineHeight: config.lineHeight as number,
      fontWeight: config.fontWeight,
      maxLines: config.numberOfLines,
      letterSpacing: config.letterSpacing
    });

    if (!config.maxWidth || px2vp(textSize.width as number) < config.maxWidth) {
      return new HRSize(Number(textSize.width), Number(textSize.height))
    }

    textSize = TextMeasurer.measureTextSize({
      textContent: config.textContent as string,
      fontSize: config.fontSize,
      lineHeight: config.lineHeight,
      fontWeight: config.fontWeight,
      constraintWidth: config.maxWidth,
      maxLines: config.numberOfLines,
      letterSpacing: config.letterSpacing
    })
    return new HRSize(Number(textSize.width), Number(textSize.height))
  }
}
