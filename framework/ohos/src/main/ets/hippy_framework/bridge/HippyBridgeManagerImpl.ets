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
import { BinaryWriter } from '../../serialization/writer/BinaryWriter';
import { HippyAny, HippyMap, HippyValue } from '../../support/common/HippyTypes';
import { ArgumentUtils } from '../../support/utils/ArgumentUtils';
import { I18nUtil } from '../../support/utils/I18nUtil';
import { HippyThirdPartyAdapter } from '../adapter/thirdparty/HippyThirdPartyAdapter';
import { Serializer } from '../serialization/Serializer';
import { HippyBundleLoader } from './bundleloader/HippyBundleLoader';
import { HippyBridgeManager } from './HippyBridgeManager';

export class HippyBridgeManagerImpl implements HippyBridgeManager {
  initBridge(): void {
    throw new Error('Method not implemented.');
  }

  runBundle(id: number, loader: HippyBundleLoader): void {
    throw new Error('Method not implemented.');
  }

  resumeInstance(id: number): void {
    throw new Error('Method not implemented.');
  }

  pauseInstance(id: number): void {
    throw new Error('Method not implemented.');
  }

  destroyInstance(id: number): void {
    throw new Error('Method not implemented.');
  }

  execCallback(params: HippyMap): void {
    throw new Error('Method not implemented.');
  }

  destroyBridge(isReload: boolean): void {
    throw new Error('Method not implemented.');
  }

  destroy(): void {
    throw new Error('Method not implemented.');
  }

  callJavaScriptModule(mName: string, name: string, params: HippyMap): void {
    throw new Error('Method not implemented.');
  }

  getThirdPartyAdapter(): HippyThirdPartyAdapter {
    throw new Error('Method not implemented.');
  }


  loadInstance(name: string, id: number, params: HippyMap) {
    // TODO(hot):
    let map = new Map<string, HippyAny>()
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
    let globalParams = new Map<string, HippyValue | Map<string, HippyValue> | Map<string, HippyValue | Map<string, HippyValue>>>()
    let dimensionMap = new Map<string, HippyValue>()

    globalParams.set("Dimensions", dimensionMap)

    let packageName = ""
    let versionName = ""
    let pageUrl = ""
    let nightMode = false
    let extraDataMap = new Map<string, HippyValue>()

    let platformParams = new Map<string, HippyValue | Map<string, HippyValue>>()
    platformParams.set("OS", "ohos")
    platformParams.set("PackageName", (packageName == null) ? "" : packageName)
    platformParams.set("VersionName", (versionName == null) ? "" : versionName)
    platformParams.set("APILevel", 0)
    platformParams.set("NightMode", nightMode)
    platformParams.set("SDKVersion", "")

    let localization = new Map<string, HippyValue>()
    localization.set("language", I18nUtil.getLanguage())
    localization.set("country", I18nUtil.getCountry())
    localization.set("direction", I18nUtil.getLayoutDirection())
    platformParams.set("Localization", localization)

    globalParams.set("Platform", platformParams)

    // TODO(etkamo): isSupportDev

    let host = new Map<string, HippyValue | Map<string, HippyValue>>()
    host.set("url", (pageUrl == null) ? "" : pageUrl)
    host.set("appName", (packageName == null) ? "" : packageName)
    host.set("appVersion", (versionName == null) ? "" : versionName)
    host.set("nightMode", nightMode)
    host.set("extra", extraDataMap)
    globalParams.set("HostConfig", host)

    return ArgumentUtils.objectToJson(globalParams)
  }
}
