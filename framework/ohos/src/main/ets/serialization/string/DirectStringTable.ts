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
import { StringEncoding } from './StringEncoding';
import { StringLocation } from './StringLocation';
import { StringTable } from './StringTable';

export class DirectStringTable implements StringTable {
  lookup(byteData: Uint8Array,
         encoding: StringEncoding,
         location: StringLocation,
         relatedKey: any
  ): string {
    if (location == StringLocation.VOID) {
      return ''
    }

    if (encoding == StringEncoding.LATIN) {
      // need iso-8859-1
      return util.TextDecoder.create("iso-8859-1").decodeWithStream(byteData)
    } else if (encoding == StringEncoding.UTF16LE) {
      // utf16-le
      return util.TextDecoder.create("utf-16le").decodeWithStream(byteData)
    } else {
      // utf8
      return util.TextDecoder.create("utf-8").decodeWithStream(byteData)
    }
  }

  release() {

  }
}
