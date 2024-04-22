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
// import { HippyJsException } from 'ets/hippy_framework/common/HippyJsException';
import { HippyGlobalConfigs } from '../../hippy_framework/HippyGlobalConfigs';
import { OnReloadListener } from './DevExceptionDialog';
import { DevFloatButton } from './DevFloatButton';
import { DevServerCallBack } from './DevServerCallBack';
import { DevServerConfig } from './DevServerConfig';
import { DevServerHelper } from './DevServerHelper';
import { DevServerInterface } from './DevServerInterface';
import Stack from '@ohos.util.Stack';
import { HippyException } from '../common/HippyException';

export class DevServerImpl implements DevServerInterface, OnReloadListener {
  private static TAG = "DevServerImpl";
  private mServerConfig: DevServerConfig | null
  private mDebugMode: boolean
  private mDevButtonMaps: Map<number, DevFloatButton> = new Map();
  private mServerCallBack: DevServerCallBack | null = null
  private mFetchHelper: DevServerHelper
  private mDevButtonStack: Stack<DevFloatButton> = new Stack();

  constructor(configs: HippyGlobalConfigs, serverHost: string, bundleName: string,
              remoteServerUrl: string, debugMode: boolean) {
    this.mDebugMode = debugMode;
    this.mFetchHelper = new DevServerHelper(configs, serverHost, remoteServerUrl);
    if (this.mDebugMode) {
      this.mServerConfig = new DevServerConfig(serverHost, bundleName);
    } else this.mServerConfig = null
  }

  onReload(): void {
    this.reload()
  }

  reload(): void {
    if (this.mServerCallBack != null) {
      this.mServerCallBack.onDebugReLoad();
    }
  }

  createResourceUrl(resName: string): string {
    if (this.mDebugMode && this.mServerConfig != null) {
      return this.mFetchHelper.createBundleURL(this.mServerConfig.getServerHost(), resName,
        this.mServerConfig.enableRemoteDebug(), false, false);
    } else return "";
  }

  onLoadResourceFailed(url: string, errorMessage: string): void {
    // let exception = new HippyException(
    //   "Could not connect to development server." + "URL: " + url
    //     + "  try to :adb reverse tcp:38989 tcp:38989 , message : " + errorMessage);
    // if (this.mDevButtonStack.isEmpty()) {
    //   this.mServerCallBack?.onInitDevError(exception);
    // } else {
    //   this.handleException(exception);
    // }
  }

  createDebugUrl(host: string, componentName: string, debugClientId: string): string {
    return ""
  }

  setDevServerCallback(devServerCallback: DevServerCallBack): void {
    this.mServerCallBack = devServerCallback;
  }

  attachToHost(context: Context, rootId: number): void {

  }

  detachFromHost(context: Context, rootId: number): void {

  }

  handleException(throwable: HippyException): void {

  }

  getDevButton(rootId: number): View | null {
    if (this.mDevButtonMaps != null) {
      return this.mDevButtonMaps.get(rootId) as View;
    }
    return null;
  }
}
