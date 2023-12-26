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

import { DescriptorRegistry } from './descriptor/DescriptorRegistry';

export class NativeRenderContext {
  private rootId: number
  private instanceId: number
  descriptorRegistry: DescriptorRegistry

  constructor(rootId: number) {
    this.rootId = rootId
    this.instanceId = -1
    this.descriptorRegistry = new DescriptorRegistry(rootId, {})
  }

  setInstanceId(instanceId: number) {
    this.instanceId = instanceId
  }

  getRootId() {
    return this.rootId
  }

  getInstanceId() {
    return this.instanceId
  }
}
