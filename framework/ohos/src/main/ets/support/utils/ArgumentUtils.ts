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

export class ArgumentUtils {
  public static objectToJson(obj: any): string {
    if (obj == null) {
      return ""
    }

    let buider: String = new String()
    this.objectToJsonImpl(obj, buider)
    return buider.toString()
  }

  private static stringFormat(value: string, builder: String) {
    builder += "\""
    for (let i = 0, length = value.length; i < length; i++) {
      let c = value.charAt(i)
      switch (c) {
        case '"':
        case '\\':
        case '/': {
          builder += "\\"
          builder += c
          break
        }
        case '\t': {
          builder += "\\t"
          break
        }
        case '\b': {
          builder += "\\b"
          break
        }
        case '\n': {
          builder += "\\n"
          break
        }
        case '\r': {
          builder += "\\r"
          break
        }
        case '\f': {
          builder += "\\f"
          break
        }
        default: {
          if (c.charCodeAt(0) <= 0x1F) {
            builder += "\\u"
            builder += ("0000" + c.charCodeAt(0).toString(16)).slice(-4)
          } else {
            builder += c
          }
          break
        }
      }
    }
    builder += "\""
  }

  private static objectToJsonImpl(obj: any, builder: String) {
    if (obj == null) {
      builder += "\"\""
      return
    }

    if (typeof obj == 'string') {
      if (obj.length == 0) {
        builder += "\"\""
      } else {
        this.stringFormat(obj, builder)
      }
    } else if (typeof obj == 'number') {
      builder += Number.isNaN(obj) ? "0" : obj.toString()
    } else if (typeof obj == 'bigint') {
      builder += obj.toString()
    } else if (typeof obj == 'boolean') {
      builder += obj.toString()
    } else if (obj instanceof ArrayList) {
      builder += "["
      let array = obj as ArrayList<any>
      let length = array.length
      for (let i = 0; i < length; i++) {
        this.objectToJsonImpl(array[i], builder)
        if (i != length - 1) {
          builder += ","
        }
      }
      builder += "]"
    } else if (obj instanceof HashMap) {
      builder += "{"
      let map = obj as HashMap<any, any>
      let length = map.length
      let count = 0
      map.forEach((value?: any, key?: any, map?: HashMap<any, any>) => {
        builder += "\""
        builder += key
        builder += "\""
        builder += ":"
        this.objectToJsonImpl(value, builder)
        ++count
        if (count < length) {
          builder += ","
        }
      })
      builder += "}"
    }
  }


}
