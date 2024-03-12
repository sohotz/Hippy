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

import HashMap from '@ohos.util.HashMap';
import { HippyLibrary } from '../hippy_library/HippyLibrary';
import { Processor } from './Processor';
import { ResourceDataHolder, FetchResourceCallback, RequestFrom } from './ResourceDataHolder';
import { ProcessorCallback } from './ProcessorCallback';

class ProcessorCallbackImpl implements ProcessorCallback {
  onHandleCompleted = () => { }

  goNext = () => { }
}

export class VfsManager {
  private libHippy: HippyLibrary
  public instanceId: number = 0

  private processorChain: Array<Processor>

  constructor(libHippy: HippyLibrary) {
    this.libHippy = libHippy
    this.instanceId = this.createVfs();
    this.processorChain = new Array;
  }

  createVfs(): number {
    return this.libHippy?.VfsManager_CreateVfs()
  }

  destroyVfs() {
    this.libHippy?.VfsManager_DestroyVfs(this.instanceId)
  }

  addProcessor(index: number, processor: Processor) {
    if (index < this.processorChain.length) {
      this.processorChain.splice(index, 0, processor)
    }
  }

  addProcessorAtFirst(processor: Processor) {
    this.processorChain.splice(0, 0, processor)
  }

  addProcessorAtLast(processor: Processor) {
    this.processorChain.push(processor)
  }

  onFetchResourceStart() {}

  fetchResourceSync(uri: string, requestHeaders: HashMap<string, string> | undefined,
                    requestParams: HashMap<string, string> | undefined) : ResourceDataHolder {
    this.onFetchResourceStart()
    const holder = this.fetchResourceSyncImpl(uri, requestHeaders, requestParams, RequestFrom.LOCAL)
    this.onFetchResourceEnd(holder)
    return holder
  }

  fetchResourceSyncImpl(uri: string, requestHeaders: HashMap<string, string> | undefined,
                        requestParams: HashMap<string, string> | undefined, from: RequestFrom) : ResourceDataHolder {
    let holder = new ResourceDataHolder(uri, requestHeaders, requestParams, null, from, -1)
    this.traverseForward(holder, true)
    return holder
  }

  fetchResourceAsync(uri: string, requestHeaders:HashMap<string, string> | undefined,
                     requestParams:HashMap<string, string> | undefined, callback: FetchResourceCallback ) {
    this.onFetchResourceStart()
    this.fetchResourceAsyncImpl(uri, requestHeaders, requestParams, callback, RequestFrom.LOCAL, -1)
  }

  fetchResourceAsyncImpl(uri: string, requestHeaders: HashMap<string, string> | undefined,
                         requestParams: HashMap<string, string> | undefined, callback: FetchResourceCallback,
                         from: RequestFrom, nativeRequestId: number) {
    let holder = new ResourceDataHolder(uri, requestHeaders, requestParams, callback, from, nativeRequestId)
    this.traverseForward(holder, false)
  }

  traverseForward(holder: ResourceDataHolder, isSync: boolean) {
    let index = holder.index + 1
    if (index < this.processorChain.length) {
      holder.index = index
      const processor = this.processorChain[index]
      if (isSync) {
        let goBack = processor.handleRequestSync(holder)
        if (goBack) {
          this.traverseGoBack(holder, true)
        } else {
          this.traverseForward(holder, true)
        }
      } else {
        const callback = new ProcessorCallbackImpl()
        callback.goNext = () => {
          this.traverseForward(holder, false)
        }
        callback.onHandleCompleted = () => {
          this.traverseGoBack(holder, false)
        }
        processor.handleRequestAsync(holder, callback)
      }
    } else if (isSync) {
      if (holder.requestFrom == RequestFrom.LOCAL) {
        this.doNativeTraversalsSync(holder)
      } else if (holder.requestFrom == RequestFrom.NATIVE) {
        this.traverseGoBack(holder, true)
      }
    } else {
      if (holder.requestFrom == RequestFrom.LOCAL) {
        this.performNativeTraversals(holder)
      } else if (holder.requestFrom == RequestFrom.NATIVE) {
        this.traverseGoBack(holder, true)
      }
    }
  }

  traverseGoBack(holder: ResourceDataHolder, isSync: boolean) {
    let index = holder.index - 1
    if (index >= 0 && index < this.processorChain.length) {
      holder.index = index
      const processor = this.processorChain[index]
      if (isSync) {
        processor.handleRequestSync(holder)
        this.traverseGoBack(holder, true)
      } else {
        let callback = new ProcessorCallbackImpl()
        callback.onHandleCompleted = () => {
          this.traverseGoBack(holder, false)
        }
        processor.handleRequestAsync(holder, callback)
      }
    } else if (!isSync) {
      if (holder.requestFrom == RequestFrom.LOCAL) {
        this.onFetchResourceEnd(holder)
      } else if (holder.requestFrom == RequestFrom.NATIVE) {
        this.onTraversalsEndAsync(holder)
      }
    }
  }

  onFetchResourceEnd(holder: ResourceDataHolder) {
    holder.callback?.onFetchCompleted(holder)
  }

  onTraversalsEndAsync(holder: ResourceDataHolder) {
    //TODO(eksea)
  }

  doNativeTraversalsSync(holder: ResourceDataHolder) {
    //TODO(eksea)
  }

  performNativeTraversals(holder: ResourceDataHolder) {
    //TODO(eksea)
  }
}
