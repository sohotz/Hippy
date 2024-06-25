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
import { HippyAny, HippyValue } from '../common/HippyTypes'

export class ArgumentUtils {
  public static objectToJson(obj: HippyAny): string {
    if (obj == null) {
      return ""
    }

    let buider: String = new String()
    ArgumentUtils.objectToJsonImpl(obj, buider)
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

  private static objectToJsonImpl(obj: HippyAny, builder: String) {
    if (obj == null) {
      builder += "\"\""
      return
    }

    if (typeof obj == 'string') {
      if (obj.length == 0) {
        builder += "\"\""
      } else {
        ArgumentUtils.stringFormat(obj, builder)
      }
    } else if (typeof obj == 'number') {
      builder += Number.isNaN(obj) ? "0" : obj.toString()
    } else if (typeof obj == 'bigint') {
      builder += obj.toString()
    } else if (typeof obj == 'boolean') {
      builder += obj.toString()
    } else if (obj instanceof Array) {
      builder += "["
      let array = obj as Array<HippyAny>
      let length = array.length
      for (let i = 0; i < length; i++) {
        ArgumentUtils.objectToJsonImpl(array[i], builder)
        if (i != length - 1) {
          builder += ","
        }
      }
      builder += "]"
    } else if (obj instanceof Map) {
      builder += "{"
      let map = obj as Map<string, HippyAny>
      let length = map.size
      let count = 0
      map.forEach((value: HippyAny, key: string) => {
        builder += "\""
        builder += key
        builder += "\""
        builder += ":"
        ArgumentUtils.objectToJsonImpl(value, builder)
        ++count
        if (count < length) {
          builder += ","
        }
      })
      builder += "}"
    }
  }

  public static mapToRecord(map: Map<string, HippyAny>) {
    let record: Record<string, Object> = {};
    map.forEach((value, key) => {
      if (key != undefined && key != null) {
        if (value instanceof Map) {
          record[key] = ArgumentUtils.mapToRecord(value);
        } else {
          record[key] = value as Object;
        }
      }
    })
    return record;
  }

}
