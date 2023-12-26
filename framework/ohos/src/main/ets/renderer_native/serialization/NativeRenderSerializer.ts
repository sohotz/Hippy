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
import ArrayList from '@ohos.util.ArrayList';
import { PrimitiveValueSerializer } from '../../serialization/PrimitiveValueSerializer';
import { SharedSerialization } from '../../serialization/SharedSerialization';
import { BinaryWriter } from '../../serialization/writer/BinaryWriter';
import { NativeRenderSerializationTag } from './NativeRenderSerializationTag';

const TAG = 'NativeRenderSerializer'

export class NativeRenderSerializer extends PrimitiveValueSerializer {
  constructor(writer: BinaryWriter) {
    super(writer, 13)
  }

  writeValue(object: any): boolean {
    if (object == null) {
      object = SharedSerialization.NULL
    }
    if (super.writeValue(object)) {
      return true
    }
    if (object instanceof HashMap) {
      this.assignId(object)
      if (object.length > 0) {
        let keys = object.keys()
        for (let key of keys) {
          if(typeof key == 'string') {
            this.writeObject(object)
          } else {
            this.writeMap(object)
          }
          break
        }
      } else {
        this.writeObject(object)
      }
    } else if (object instanceof ArrayList) {
      this.assignId(object)
      this.writeList(object)
    } else {
      // TODO(hot):
      throw TAG + ": Unsupported object data type, object=" + object
    }
    return true
  }

  writeMap(map: HashMap<any, any>) {
    this.writeTag(NativeRenderSerializationTag.BEGIN_MAP)
    let count = 0
    map.forEach((value?: any, key?: any, map?: HashMap<any, any>) => {
      count++
      this.writeValue(key)
      this.writeValue(value)
    })
    this.writeTag(NativeRenderSerializationTag.END_MAP)
    this.writer_.putVarint(2 * count)
  }

  writeObject(map: HashMap<string, any>) {
    this.writeTag(NativeRenderSerializationTag.BEGIN_OBJECT)
    map.forEach((value?: any, key?: string, map?: HashMap<string, any>) => {
      if (key == null) {
        this.writeString("null")
      } else {
        this.writeString(key)
      }
      this.writeValue(value)
    })
    this.writeTag(NativeRenderSerializationTag.END_OBJECT)
    this.writer_.putVarint(map.length)
  }

  writeList(list: ArrayList<any>) {
    let length = list.length
    this.writeTag(NativeRenderSerializationTag.BEGIN_DENSE_ARRAY)
    this.writer_.putVarint(length)
    for(let i = 0; i < length; i++) {
      this.writeValue(list[i])
    }
    this.writeTag(NativeRenderSerializationTag.END_DENSE_ARRAY)
    this.writer_.putVarint(0)
    this.writer_.putVarint(length)
  }
}
