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
import { LogUtils } from '../../support/utils/LogUtils';
import { BinaryReader } from '../../serialization/reader/BinaryReader';
import { StringLocation } from '../../serialization/string/StringLocation';
import { StringTable } from '../../serialization/string/StringTable';
import { PrimitiveValueDeserializer } from '../../serialization/PrimitiveValueDeserializer';
import { PrimitiveSerializationTag } from '../../serialization/PrimitiveSerializationTag';
import { NativeRenderSerializationTag } from './NativeRenderSerializationTag';
import { SharedSerialization } from '../../serialization/SharedSerialization';

const TAG = 'NativeRenderDeserializer'

export class NativeRenderDeserializer extends PrimitiveValueDeserializer {
  constructor(reader: BinaryReader, stringTable: StringTable) {
    super(reader, stringTable)
  }

  getSupportedVersion(): number {
    return -1
  }

  readValue3(tag: number, location: StringLocation, relatedKey: any): any {
    let object = super.readValue3(tag, location, relatedKey)
    if (object != SharedSerialization.NOTHING) {
      return object
    }

    switch (tag) {
      case NativeRenderSerializationTag.TRUE_OBJECT:
        return this.readBooleanObject(true)
      case NativeRenderSerializationTag.FALSE_OBJECT:
        return this.readBooleanObject(false)
      case NativeRenderSerializationTag.NUMBER_OBJECT:
        return this.readNumberObject()
      case NativeRenderSerializationTag.BIG_INT_OBJECT:
        return this.readBigIntObject()
      case NativeRenderSerializationTag.STRING_OBJECT:
        return this.readStringObject(location, relatedKey)
      case NativeRenderSerializationTag.BEGIN_OBJECT:
        return this.readObject()
      case NativeRenderSerializationTag.BEGIN_MAP:
        return this.readMap()
      case NativeRenderSerializationTag.BEGIN_DENSE_ARRAY:
        return this.readDenseArray()
      case NativeRenderSerializationTag.BEGIN_SPARSE_JS_ARRAY:
        return this.readSparseArray()
      default:
        // TODO(hot):
        throw 'error'
    }
  }

  readBooleanObject(value: boolean): any {
    return this.assignId(value)
  }

  readNumberObject(): number {
    return this.assignId(this.reader_.getDouble())
  }

  readBigIntObject(): bigint {
    return this.assignId(this.readBigInt())
  }

  readStringObject(location: StringLocation, relatedKey: any): string {
    return this.assignId(this.readString(location, relatedKey))
  }

  readObject(): HashMap<string, any> {
    let map = new HashMap<string, any>()
    this.assignId(map)
    let read = this.readObjectProperties(map)
    let expected = this.reader_.getVarint()
    if (read != expected) {
      // TODO(hot):
      throw 'error'
    }
    return map;
  }

  readObjectProperties(map: HashMap<string, any>): number {
    let keyLocation = StringLocation.OBJECT_KEY
    let valueLocation = StringLocation.OBJECT_VALUE

    let tag: number = 0
    let count: number = 0
    while ((tag = this.readTag()) != NativeRenderSerializationTag.END_OBJECT) {
      count++
      let key = this.readValue3(tag, keyLocation, null)
      let value = this.readValue2(valueLocation, key)
      if (typeof key == 'string' || typeof key == 'number') {
        map.set(key.toString(), value)
      } else {
        // TODO(hot):
        throw TAG + ": readObjectProperties: Object key is not of String nor Number type"
      }
    }
    return count
  }

  readMap(): HashMap<any, any> {
    let map: HashMap<any, any> = new HashMap<any, any>()
    this.assignId(map)
    let tag: number = 0
    let read: number = 0
    while ((tag = this.readTag()) != NativeRenderSerializationTag.END_MAP) {
      read++
      let key = this.readValue3(tag, StringLocation.MAP_KEY, null)
      let value = this.readValue2(StringLocation.MAP_VALUE, key)
      map.set(key, value)
    }
    let expected = this.reader_.getVarint()
    if (2 * read != expected) {
      // TODO(hot):
      throw TAG + ": readMap: unexpected number of entries"
    }
    return map
  }

  readDenseArray(): ArrayList<any> {
    let totalLength = this.reader_.getVarint()
    if (totalLength < 0) {
    // TODO(hot):
      throw 'error'
    }
    let array: ArrayList<any> = new ArrayList<any>()
    this.assignId(array)
    for (let i = 0; i < totalLength; i++) {
      let tag = this.readTag()
      if (tag != PrimitiveSerializationTag.THE_HOLE) {
        array.add(this.readValue3(tag, StringLocation.DENSE_ARRAY_ITEM, i))
      }
    }
    let propsLength = this.readArrayProperties()
    let expected = this.reader_.getVarint()
    if (propsLength != expected) {
      // TODO(hot):
      throw TAG + ": readDenseArray: unexpected number of properties"
    }
    expected = this.reader_.getVarint()
    if (totalLength != expected) {
      // TODO(hot):
      throw TAG + ": readDenseArray: length ambiguity"
    }
    return array
  }

  readArrayProperties(): number {
    let keyLocation = StringLocation.DENSE_ARRAY_KEY
    let valueLocation = StringLocation.DENSE_ARRAY_ITEM

    let tag: number = 0
    let count: number = 0
    while ((tag = this.readTag()) != NativeRenderSerializationTag.END_DENSE_ARRAY) {
      count++
      let key = this.readValue3(tag, keyLocation, null)
      let value = this.readValue2(valueLocation, key)
      LogUtils.d(TAG, "readArrayProperties: key" + key + ", value=" + value)
    }
    return count
  }

  readSparseArray(): ArrayList<any> {
    let length = this.reader_.getVarint()
    let array: ArrayList<any> = new ArrayList<any>()
    this.assignId(array)
    let tag: number = 0
    let read: number = 0
    while ((tag = this.readTag()) != NativeRenderSerializationTag.END_SPARSE_JS_ARRAY) {
      read++
      let key = this.readValue3(tag, StringLocation.SPARSE_ARRAY_KEY, null)
      let value = this.readValue2(StringLocation.SPARSE_ARRAY_ITEM, key)
      let index = -1
      if (typeof key == 'number') {
        index = Math.trunc(key.valueOf())
      } else if (typeof key == 'string') {
        try {
          index = parseInt(key.valueOf())
        } catch (e) {
          // ignore not parsable string
        }
      }
      if (index >= 0) {
        let spaceNeeded = (index + 1) - array.length
        if (spaceNeeded == 1) { // Fast path, item are ordered in general ECMAScript(VM) implementation
          array.add(value)
        } else { // Slow path, universal
          for (let i = 0; i < spaceNeeded; i++) {
            array.add(null)
          }
          array[index] = value
        }
      }
    }
    let expected = this.reader_.getVarint()
    if (read != expected) {
      // TODO(hot):
      throw "unexpected number of properties"
    }
    let length2 = this.reader_.getVarint()
    if (length != length2) {
      // TODO(hot):
      throw "length ambiguity"
    }
    return array
  }
}
