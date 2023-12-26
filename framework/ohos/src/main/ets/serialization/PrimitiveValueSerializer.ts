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
import { PrimitiveSerializationTag } from './PrimitiveSerializationTag';
import { SharedSerialization } from './SharedSerialization';
import { BinaryWriter } from './writer/BinaryWriter';

export class PrimitiveValueSerializer extends SharedSerialization {
  writer_: BinaryWriter

  version_: number = 0
  nextId_: number = 0

  objectMap_: HashMap<any, number> = new HashMap<any, number>()

  readonly MAX_UINT32_VALUE: number = 4294967295
  readonly MIN_UINT32_VALUE: number = 0
  readonly SSO_SMALL_STRING_MAX_LENGTH: number = 32
  readonly ISO_8859_1_MAX_CHAR: number = 0xff

  constructor(writer: BinaryWriter, version: number) {
    super()
    this.writer_ = writer
    this.version_ = version
  }

  setWriter(writer: BinaryWriter) {
    this.writer_ = writer
  }

  getWriter(): BinaryWriter {
    return this.writer_
  }

  reset() {
    this.objectMap_.clear()
    this.nextId_ = 0
  }

  writeHeader() {
    this.writeTag(PrimitiveSerializationTag.VERSION)
    this.writer_.putVarint(this.version_)
  }

  writeTag(tag: number) {
    this.writer_.putByte(tag)
  }

  writeValue(value: Object): boolean {
    if (typeof value == 'string') {
      this.writeString(value)
    } else if (typeof value == 'number') {
      let longValue = Math.trunc(value)
      if (value == longValue) {
        if (longValue <= this.MAX_UINT32_VALUE && longValue >= this.MIN_UINT32_VALUE) {
          this.writeTag(PrimitiveSerializationTag.UINT32)
          this.writer_.putVarint(longValue)
        } else {
          this.writeTag(PrimitiveSerializationTag.DOUBLE)
          this.writer_.putDouble(value)
        }
      } else {
        this.writeTag(PrimitiveSerializationTag.DOUBLE)
        this.writer_.putDouble(value)
      }
    } else if (typeof value == 'boolean') {
      if (value) {
        this.writeTag(PrimitiveSerializationTag.TRUE)
      } else {
        this.writeTag(PrimitiveSerializationTag.FALSE)
      }
    } else if (value == SharedSerialization.HOLE) {
      this.writeTag(PrimitiveSerializationTag.THE_HOLE)
    } else if (value == SharedSerialization.UNDEFINED) {
      this.writeTag(PrimitiveSerializationTag.UNDEFINED)
    } else if (value == SharedSerialization.NULL) {
      this.writeTag(PrimitiveSerializationTag.NULL)
    } else {
      let id = this.objectMap_.get(value)
      if (id != null) {
        this.writeTag(PrimitiveSerializationTag.OBJECT_REFERENCE)
        this.writer_.putVarint(id)
      } else if (value instanceof Date) {
        this.assignId(value)
        this.writeDate(value)
      } else {
        return false
      }
    }
    return true
  }

  writeString(value: string) {
    let length = value.length
    let isOneByteString = true
    for (let i = 0; i < length; i++) {
      if (value.charCodeAt(i) >= this.ISO_8859_1_MAX_CHAR) {
        isOneByteString = false;
        break;
      }
    }

    if (isOneByteString) {
      // region one byte string, commonly path
      this.writeTag(PrimitiveSerializationTag.ONE_BYTE_STRING)
      this.writer_.putVarint(length)
      for (let i = 0; i < length; i++) {
        this.writer_.putByte(value.charCodeAt(i))
      }
    } else {
      // region two byte string, universal path
      this.writeTag(PrimitiveSerializationTag.TWO_BYTE_STRING)
      this.writer_.putVarint(length * 2)
      for (let i = 0; i < length; i++) {
        this.writer_.putChar(value.charCodeAt(i))
      }
    }
  }

  writeDate(date: Date) {
    this.writeTag(PrimitiveSerializationTag.DATE)
    this.writer_.putDouble(date.getTime())
  }

  protected assignId(object: any) {
    this.objectMap_.set(object, this.nextId_++)
  }

  getVersion(): number {
    return this.version_
  }
}
