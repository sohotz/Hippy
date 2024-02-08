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
import { BinaryReader } from '../serialization/reader/BinaryReader'
import { NativeRenderImpl } from './NativeRenderImpl';
import { NativeRenderDeserializer } from './serialization/NativeRenderDeserializer'
import { BinaryWriter } from '../serialization/writer/BinaryWriter';
import { NativeRenderSerializer } from './serialization/NativeRenderSerializer';
import { NativeRenderProviderManager } from './NativeRenderProviderManager';
import { HREventType } from './utils/HREventUtils';
import { LogUtils } from '../support/utils/LogUtils';
import { HippyLibrary } from '../hippy_library/HippyLibrary';
import { HippyAny } from '../support/common/HippyTypes';

export class NativeRenderProvider {
  private readonly TAG = "NativeRenderProvider"
  private readonly EVENT_PREFIX = "on"

  private libHippy: HippyLibrary
  private renderImpl: NativeRenderImpl
  private instanceId: number = 0

  constructor(libHippy: HippyLibrary) {
    this.libHippy = libHippy
    this.renderImpl = new NativeRenderImpl()
  }

  setInstanceId(instanceId: number) {
    this.instanceId = instanceId
    this.renderImpl.getNativeRenderContext().setInstanceId(instanceId)
    NativeRenderProviderManager.addRenderProvider(instanceId, this)
  }

  getNativeRenderImpl(): NativeRenderImpl {
    return this.renderImpl
  }

  bytesToArgument(buffer: ArrayBuffer): Array<HippyAny> {
    // TODO(hot):
    let binaryReader = new BinaryReader(new Uint8Array(buffer))
    let deserializer = new NativeRenderDeserializer(binaryReader, null)
    deserializer.readHeader()
    let paramsObj = deserializer.readValue() as Array<HippyAny>
    // TODO(hot):
    //deserializer.printValue(paramsObj, '', '')
    return (paramsObj instanceof Array) ? paramsObj : new Array<HippyAny>()
  }

  argumentToBytes(params: HippyAny): ArrayBuffer {
    // TODO(hot):
    let binaryWriter = new BinaryWriter()
    let serializer = new NativeRenderSerializer(binaryWriter)
    serializer.writeHeader()
    serializer.writeValue(params)
    return binaryWriter.chunked().buffer
  }

  createNode(rootId: number, buffer: ArrayBuffer) {
    let nodeList = this.bytesToArgument(buffer)
    this.renderImpl.createNode(rootId, nodeList)
  }

  updateNode(rootId: number, buffer: ArrayBuffer) {
    let nodeList = this.bytesToArgument(buffer)
    this.renderImpl.updateNode(rootId, nodeList)
  }

  moveNode(rootId: number, pid: number, buffer: ArrayBuffer) {
    let nodeList = this.bytesToArgument(buffer)
    this.renderImpl.moveNode(rootId, pid, nodeList)
  }

  moveNode2(rootId: number, id_array: Array<number>, to_pid: number, from_pid: number, index: number) {
    this.renderImpl.moveNode2(rootId, id_array, to_pid, from_pid, index)
  }

  deleteNode(rootId: number, id_array: Array<number>) {
    this.renderImpl.deleteNode(rootId, id_array)
  }

  updateLayout(rootId: number, buffer: ArrayBuffer) {
    let nodeList = this.bytesToArgument(buffer)
    this.renderImpl.updateLayout(rootId, nodeList)
  }

  endBatch(rootId: number) {
    this.renderImpl.endBatch(rootId)
  }

  updateEventListener(rootId: number, buffer: ArrayBuffer) {
    let eventList = this.bytesToArgument(buffer)
    this.renderImpl.updateEventListener(rootId, eventList)
  }

  callUIFunction(rootId: number, nodeId: number, callbackId: number, functionName: string, buffer: ArrayBuffer) {
    let params = this.bytesToArgument(buffer)
    // If callbackId equal to 0 mean this call does not need to callback.
    this.renderImpl.callUIFunction(nodeId, functionName, params, (callbackId == 0) ? null : (result: HippyAny) => {
      this.doCallBack(1, callbackId, functionName, rootId, nodeId, result)
    })
  }

  measure(rootId: number, nodeId: number, width: number, widthMode: number, height: number, heightMode: number): bigint {
    return this.renderImpl.measure(rootId, nodeId, width, widthMode, height, heightMode)
  }

  onSizeChanged(rootId: number, width: number, height: number) {
    this.libHippy?.NativeRenderImpl_UpdateRootSize(this.instanceId, rootId, width, height)
  }

  onSizeChanged2(rootId: number, nodeId: number, width: number, height: number, isSync: boolean) {
    this.libHippy?.NativeRenderImpl_UpdateNodeSize(this.instanceId, rootId, nodeId, width, height, isSync)
  }

  dispatchEvent(rootId: number, nodeId: number, eventName: string, params: HippyAny, useCapture: boolean, useBubble: boolean, eventType: HREventType) {
    // Because the native(C++) DOM use lowercase names, convert to lowercase here
    let lowerCaseEventName = eventName.toLowerCase()
    // Compatible with events prefixed with on in old version
    if (lowerCaseEventName.startsWith(this.EVENT_PREFIX)) {
        lowerCaseEventName = lowerCaseEventName.substring(this.EVENT_PREFIX.length)
    }
    // TODO(hot): to checkRegisteredEvent
    if (eventType != HREventType.GESTURE && false) {
        return;
    }
    LogUtils.d(this.TAG, "dispatchEvent: id " + nodeId + ", eventName " + eventName
               + ", eventType " + eventType + ", params " + params)

    let buffer: ArrayBuffer | null = null
    if (params != null) {
      try {
        buffer = this.argumentToBytes(params)
      } catch (e) {

      }
    }
    this.libHippy?.NativeRenderImpl_OnReceivedEvent(this.instanceId, rootId, nodeId, lowerCaseEventName, buffer, useCapture, useBubble)
  }

  doCallBack(result: number, callbackId: number, functionName: string, rootId: number, nodeId: number, params: HippyAny) {
    let buffer: ArrayBuffer | null = null
    if (params != null) {
      try {
        buffer = this.argumentToBytes(params)
      } catch (e) {

      }
    }
    this.libHippy?.NativeRenderImpl_DoCallBack(this.instanceId, result, functionName, rootId, nodeId, callbackId, buffer)
  }
}
