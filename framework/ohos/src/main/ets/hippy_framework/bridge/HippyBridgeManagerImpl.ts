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
import { BinaryWriter } from '../../serialization/writer/BinaryWriter';
import { ArgumentUtils } from '../../support/utils/ArgumentUtils';
import { I18nUtil } from '../../support/utils/I18nUtil';
import { Serializer } from '../serialization/Serializer';
import { HippyBridgeManager } from './HippyBridgeManager';

export class HippyBridgeManagerImpl implements HippyBridgeManager {

  loadInstance(name: string, id: number, params: HashMap<string, any>) {
    // TODO(hot):
    let map = new HashMap<string, any>()
    map.set("name", name)
    map.set("id", id)
    map.set("params", params)

    let binaryWriter = new BinaryWriter()
    let serializer = new Serializer(binaryWriter)
    serializer.writeHeader()
    serializer.writeValue(params)
    let buffer = binaryWriter.chunked().buffer
  }

  getGlobalConfigs(): string {
    // TODO(hot):
    let globalParams = new HashMap<string, any>()
    let dimensionMap = new HashMap<string, any>()

    globalParams.set("Dimensions", dimensionMap)

    let packageName = ""
    let versionName = ""
    let pageUrl = ""
    let nightMode = false
    let extraDataMap = new HashMap<string, any>()

    let platformParams = new HashMap<string, any>()
    platformParams.set("OS", "ohos")
    platformParams.set("PackageName", (packageName == null) ? "" : packageName)
    platformParams.set("VersionName", (versionName == null) ? "" : versionName)
    platformParams.set("APILevel", 0)
    platformParams.set("NightMode", nightMode)
    platformParams.set("SDKVersion", "")

    let localization = new HashMap<string, any>()
    localization.set("language", I18nUtil.getLanguage())
    localization.set("country", I18nUtil.getCountry())
    localization.set("direction", I18nUtil.getLayoutDirection())
    platformParams.set("Localization", localization)

    globalParams.set("Platform", platformParams)

    // TODO(etkamo): isSupportDev

    let host = new HashMap<string, any>()
    host.set("url", (pageUrl == null) ? "" : pageUrl)
    host.set("appName", (packageName == null) ? "" : packageName)
    host.set("appVersion", (versionName == null) ? "" : versionName)
    host.set("nightMode", nightMode)
    host.set("extra", extraDataMap)
    globalParams.set("HostConfig", host)

    return ArgumentUtils.objectToJson(globalParams)
  }
}
