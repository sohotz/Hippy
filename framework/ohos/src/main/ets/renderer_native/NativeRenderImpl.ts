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

import ArrayList from '@ohos.util.ArrayList';
import TextMeasurer from "@ohos.measure"
import { NativeRenderContext } from './NativeRenderContext';
import { CreateMutation,
  DeleteMutation,
  Move2Mutation,
  MoveMutation,
  Mutation, MutationType,
  UpdateLayoutMutation,
  UpdateMutation } from './descriptor/Mutation';
import { VirtualNode } from './node/VirtualNode';
import { LogUtils } from '../support/utils/LogUtils';
import { PixelUtil } from '../support/utils/PixelUtil';

type TextMeasurerConfig = {
  textContent: string
  fontSize: number
  lineHeight: number
  fontWeight?: number
  maxWidth?: number
  numberOfLines: number
  letterSpacing?: number
}

declare function px2vp(px: number): number

export class NativeRenderImpl {
  private static readonly ROOT_VIEW_ID_INCREMENT = 10
  private static readonly INVALID_NODE_ID = -1

  private static sRootIdCounter = 0

  private ctx: NativeRenderContext
  private mutations: Mutation[] = []
  private virtualNodes: Map<number, VirtualNode>

  constructor() {
    NativeRenderImpl.sRootIdCounter += NativeRenderImpl.ROOT_VIEW_ID_INCREMENT
    let rootId = NativeRenderImpl.sRootIdCounter
    this.ctx = new NativeRenderContext(rootId)
    this.virtualNodes = new Map<number, VirtualNode>()
  }

  public getNativeRenderContext(): NativeRenderContext {
    return this.ctx
  }

  createVirtualNode(rootId: number, id: number, pid: number, index: number, props: any): VirtualNode {
    let node = new VirtualNode()
    node.rootId = rootId
    node.id = id
    node.pid = pid
    node.index = index
    node.props = props
    return node
  }

  addVirtualNode(id: number, node: VirtualNode) {
    this.virtualNodes.set(id, node)
  }

  removeVirtualNode(id: number) {
    this.virtualNodes.delete(id)
  }

  createNode(rootId: number, nodeList: ArrayList<any>) {
    for (let i = 0; i < nodeList.length; i++) {
      let node = nodeList[i]

      let vNode = this.createVirtualNode(rootId, node.get('id'), node.get('pId'), node.get('index'), node.get('props'))
      this.addVirtualNode(node.get('id'), vNode)

      let createMutation: CreateMutation = {
        type: MutationType.CREATE,
        descriptor: {
          type: node.get('name'),
          tag: node.get('id'),
          parentTag: node.get('pId'),
          childrenTags: [],
          index: node.get('index'),
          props: {
            backgroundColor: node.get('props').get('backgroundColor'),
            // TextView
            text: node.get('props').get('text'),
            color: node.get('props').get('color'),
            fontSize: node.get('props').get('fontSize'),
            lineHeight: node.get('props').get('lineHeight')
          },//node.get('props'), //{ top: 0, left: 0, width: 0, height: 0 },
          state: {},
          layoutMetrics: {
            frame: {
              origin: {
                x: 0,
                y: 0,
              },
              size: {
                width: 0,
                height: 0,
              }
            }
          }
        }
      }
      this.mutations.push(createMutation)
    }
  }

  updateNode(rootId: number, nodeList: ArrayList<any>) {
    for (let i = 0; i < nodeList.length; i++) {
      let node = nodeList[i]
      let updateMutation: UpdateMutation = {
        type: MutationType.UPDATE,
        descriptor: {
          type: node.get('name'),
          tag: node.get('id'),
          parentTag: node.get('pId'),
          childrenTags: [],
          index: node.get('index'),
          props: node.get('props'), //{ top: 0, left: 0, width: 0, height: 0 },
          state: {},
          layoutMetrics: {
            frame: {
              origin: {
                x: 0,
                y: 0,
              },
              size: {
                width: 0,
                height: 0,
              }
            }
          }
        }
      }
      this.mutations.push(updateMutation)
    }
  }

  moveNode(rootId: number, pid: number, nodeList: ArrayList<any>) {
    for (let i = 0; i < nodeList.length; i++) {
      let node = nodeList[i]
      let moveMutation: MoveMutation = {
        type: MutationType.MOVE,
        pid: pid,
        descriptor: {
          type: node.get('name'),
          tag: node.get('id'),
          parentTag: node.get('pId'),
          childrenTags: [],
          index: node.get('index'),
          props: { top: 0, left: 0, width: 0, height: 0 },
          state: {},
          layoutMetrics: {
            frame: {
              origin: {
                x: 0,
                y: 0,
              },
              size: {
                width: 0,
                height: 0,
              }
            }
          }
        }
      }
      this.mutations.push(moveMutation)
    }
  }

  moveNode2(rootId: number, id_array: Array<number>, to_pid: number, from_pid: number, index: number) {
    for (let i = 0; i < id_array.length; i++) {
      let id = id_array[i]
      let move2Mutation: Move2Mutation = {
        type: MutationType.MOVE2,
        id: id,
        to_pid: to_pid,
        from_pid: from_pid,
        index: index
      }
      this.mutations.push(move2Mutation)
    }
  }

  deleteNode(rootId: number, id_array: Array<number>) {
    for (let i = 0; i < id_array.length; i++) {
      let id = id_array[i]
      let deleteMutation: DeleteMutation = {
        type: MutationType.DELETE,
        id: id,
      }
      this.mutations.push(deleteMutation)
    }
  }

  updateLayout(rootId: number, nodeList: ArrayList<any>) {
    for (let i = 0; i < nodeList.length; i++) {
      let node = nodeList[i]
      let updateMutation: UpdateLayoutMutation = {
        type: MutationType.UPDATE_LAYOUT,
        id: node.get('id'),
        left: px2vp(node.get('left')),
        top: px2vp(node.get('top')),
        width: px2vp(node.get('width')),
        height: px2vp(node.get('height')),
      }
      this.mutations.push(updateMutation)
    }
  }

  endBatch(rootId: number) {
    this.ctx.descriptorRegistry.applyMutations(this.mutations)
    this.mutations.splice(0, this.mutations.length)
  }

  updateEventListener(rootId: number, eventList: ArrayList<any>) {

  }

  callUIFunction(rootId: number, nodeId: number, callbackId: number, functionName: string, eventList: ArrayList<any>) {

  }

  measure(rootId: number, nodeId: number, width: number, widthMode: number, height: number, heightMode: number): bigint {
    let vNode = this.virtualNodes.get(nodeId)
    let config: TextMeasurerConfig = {
      textContent: vNode.props.get('text'),
      fontSize: vNode.props.get('fontSize'),
      lineHeight: vNode.props.get('lineHeight'),
      numberOfLines: 1,
    }
    let result = this.measureText(config)
    return (BigInt(result.width) << BigInt(32)) | BigInt(result.height)
  }

  private measureText(config: TextMeasurerConfig){
    let textSize = TextMeasurer.measureTextSize({
      textContent: config.textContent,
      fontSize: config.fontSize,
      lineHeight: config.lineHeight,
      fontWeight: config.fontWeight,
      maxLines: config.numberOfLines,
      letterSpacing: config.letterSpacing
    });

    if (px2vp(textSize.width as number) < config.maxWidth) {
      return textSize as {width: number, height: number};
    }

    return TextMeasurer.measureTextSize({
      textContent: config.textContent,
      fontSize: config.fontSize,
      lineHeight: config.lineHeight,
      fontWeight: config.fontWeight,
      constraintWidth: config.maxWidth,
      maxLines: config.numberOfLines,
      letterSpacing: config.letterSpacing
    }) as {width: number, height: number};
  }
}
