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

import ArrayList from '@ohos.util.ArrayList'
import HashMap from '@ohos.util.HashMap'
import { LogUtils } from '../support/utils/LogUtils'

const TAG = 'Serialization'

export enum OddballType {
  UNDEFINED,
  NULL,
  HOLE,
  NOTHING
}

export class Oddball {
  private type_: OddballType

  constructor(type: OddballType) {
    this.type_ = type
  }

  isUndefined(): boolean {
    return this.type_ === OddballType.UNDEFINED
  }

  isNull(): boolean {
    return this.type_ === OddballType.NULL
  }

  isHole(): boolean {
    return this.type_ === OddballType.HOLE
  }
}

export class SharedSerialization {
  static readonly HOLE = new Oddball(OddballType.HOLE)
  static readonly UNDEFINED = new Oddball(OddballType.UNDEFINED)
  static readonly NULL = new Oddball(OddballType.NULL)
  static readonly NOTHING = new Oddball(OddballType.NOTHING)

  printValue(value: any, spaceString: string, prefixString: string) {
    if(value instanceof ArrayList) {
      LogUtils.d(TAG, `${spaceString} ${prefixString} Array(length=${value.length})`)
      for(let i = 0; i < value.length; i++) {
        this.printValue(value[i], `${spaceString}  `, `[${i}]: `)
      }
    } else if (value instanceof HashMap) {
      LogUtils.d(TAG, `${spaceString} ${prefixString} Map(length=${value.length})`)
      value.forEach((value?: any, key?: any, map?: HashMap<any, any>) => {
        this.printValue(key, `${spaceString}  `, `k: `)
        this.printValue(value, `${spaceString}  `, `v: `)
      })
    } else if (value instanceof Date) {
      LogUtils.d(TAG, `${spaceString} ${prefixString} Date(mills=${value.getTime()})`)
    } else if (typeof value == 'string') {
      LogUtils.d(TAG, `${spaceString} ${prefixString} \'${value}\'`)
    } else if (typeof value == 'number') {
      LogUtils.d(TAG, `${spaceString} ${prefixString} ${value}`)
    } else if (typeof value == 'boolean') {
      LogUtils.d(TAG, `${spaceString} ${prefixString} ${value}`)
    } else if (typeof value == 'bigint') {
      LogUtils.d(TAG, `${spaceString} ${prefixString} bigint(${value})`)
    } else if (value == SharedSerialization.HOLE) {
      LogUtils.d(TAG, `${spaceString} ${prefixString} HOLE`)
    } else if (value == SharedSerialization.UNDEFINED) {
      LogUtils.d(TAG, `${spaceString} ${prefixString} UNDEFINED`)
    } else if (value == SharedSerialization.NULL) {
      LogUtils.d(TAG, `${spaceString} ${prefixString} NULL`)
    } else {
      LogUtils.d(TAG, `${spaceString} ${prefixString} NOTHING`)
    }
  }
}
