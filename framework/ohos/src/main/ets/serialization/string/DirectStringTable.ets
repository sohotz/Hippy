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
import util from '@ohos.util';
import { HippyAny } from '../../support/common/HippyTypes';
import { StringEncoding } from './StringEncoding';
import { StringLocation } from './StringLocation';
import { StringTable } from './StringTable';

export class DirectStringTable implements StringTable {

  private static sIsoDecoder: util.TextDecoder | null = null
  private static sUtf16Decoder: util.TextDecoder | null = null
  private static sUtf8Decoder: util.TextDecoder | null = null

  static getIsoDecoder(): util.TextDecoder {
    if (!DirectStringTable.sIsoDecoder) {
      DirectStringTable.sIsoDecoder = util.TextDecoder.create("iso-8859-1")
    }
    return DirectStringTable.sIsoDecoder
  }

  static getUtf16Decoder(): util.TextDecoder {
    if (!DirectStringTable.sUtf16Decoder) {
      DirectStringTable.sUtf16Decoder = util.TextDecoder.create("utf-16le")
    }
    return DirectStringTable.sUtf16Decoder
  }

  static getUtf8Decoder(): util.TextDecoder {
    if (!DirectStringTable.sUtf8Decoder) {
      DirectStringTable.sUtf8Decoder = util.TextDecoder.create("utf-8")
    }
    return DirectStringTable.sUtf8Decoder
  }

  lookup(byteData: Uint8Array,
         encoding: StringEncoding,
         location: StringLocation,
         relatedKey: HippyAny
  ): string {
    if (location == StringLocation.VOID) {
      return ''
    }

    return this.lookupImpl(byteData, encoding)
  }

  lookupImpl(byteData: Uint8Array,
             encoding: StringEncoding,
  ): string {
    if (encoding == StringEncoding.LATIN) {
      // need iso-8859-1
      return DirectStringTable.getIsoDecoder().decode(byteData) // 没用 decodeWithStream 的原因：静态变量多次调用 decodeWithStream 会乱码
    } else if (encoding == StringEncoding.UTF16LE) {
      // utf16-le
      return DirectStringTable.getUtf16Decoder().decode(byteData)
    } else {
      // utf8
      return DirectStringTable.getUtf8Decoder().decode(byteData)
    }
  }

  release() {

  }
}
