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
import { HippyAny } from '../../support/common/HippyTypes';
import { DirectStringTable } from './DirectStringTable';
import { StringEncoding } from './StringEncoding';
import { StringLocation } from './StringLocation';

export class InternalizedStringTable extends DirectStringTable {
  // TODO(hot):

  lookup(byteData: Uint8Array,
         encoding: StringEncoding,
         location: StringLocation,
         relatedKey: HippyAny
  ): string {
    switch (location) {
      case StringLocation.OBJECT_KEY:
      case StringLocation.DENSE_ARRAY_KEY:
      case StringLocation.SPARSE_ARRAY_KEY:
      case StringLocation.MAP_KEY:
        return ''
      case StringLocation.OBJECT_VALUE:
      case StringLocation.DENSE_ARRAY_ITEM:
      case StringLocation.SPARSE_ARRAY_ITEM:
      case StringLocation.MAP_VALUE:
        return ''
      case StringLocation.ERROR_MESSAGE:
      case StringLocation.ERROR_STACK:
      case StringLocation.REGEXP:
      case StringLocation.SET_ITEM:
      case StringLocation.TOP_LEVEL:
        return super.lookup(byteData, encoding, location, relatedKey);
      case StringLocation.VOID:
        return ''
      default:
        return ''
    }
  }

  release() {
    super.release()
  }
}
