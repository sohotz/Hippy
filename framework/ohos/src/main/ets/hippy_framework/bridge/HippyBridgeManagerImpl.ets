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
import { HippyAny,
  HippyArray,
  HippyMap, HippyValue } from '../../support/common/HippyTypes';
import { ArgumentUtils } from '../../support/utils/ArgumentUtils';
import { DimensionsUtil } from '../../support/utils/DimensionsUtil';
import { I18nUtil } from '../../support/utils/I18nUtil';
import { LogUtils } from '../../support/utils/LogUtils';
import { MonitorGroupType, TimeMonitor } from '../../support/utils/TimeMonitor';
import { HippyThirdPartyAdapter } from '../adapter/thirdparty/HippyThirdPartyAdapter';
import { HippyJsException } from '../common/HippyJsException';
import { JsDriver } from '../connector/JsDriver';
import { NativeCallback } from '../connector/NativeCallback';
import { ModuleLoadStatus, V8InitParams } from '../HippyEngine';
import { HippyEngineContext } from '../HippyEngineContext';
import { TurboModuleManager } from '../modules/turbo/TurboModuleManager';
import { Serializer } from '../serialization/Serializer';
import { HippyBundleLoader } from './bundleloader/HippyBundleLoader';
import { BridgeCallback } from './HippyBridge';
import { HippyBridgeImpl } from './HippyBridgeImpl';
import { HippyBridgeManager } from './HippyBridgeManager';
import { HippyCallNativeParams } from './HippyCallNativeParams';
import { util } from '@kit.ArkTS';

export enum BridgeState {
  UNINITIALIZED = 0,
  INITIALIZED = 1,
  DESTROYED = 2,
}

export class HippyBridgeManagerImpl implements HippyBridgeManager, BridgeCallback {
  // Bridge 状态
  public static MSG_CODE_INIT_BRIDGE = 10;
  public static MSG_CODE_RUN_BUNDLE = 11;
  public static MSG_CODE_CALL_FUNCTION = 12;
  public static MSG_CODE_DESTROY_BRIDGE = 13;
  public static MSG_CODE_ON_BRIDGE_DESTROYED = 14;
  // Instance 状态
  public static FUNCTION_ACTION_LOAD_INSTANCE = 1;
  public static FUNCTION_ACTION_RESUME_INSTANCE = 2;
  public static FUNCTION_ACTION_PAUSE_INSTANCE = 3;
  public static FUNCTION_ACTION_DESTROY_INSTANCE = 4;
  public static FUNCTION_ACTION_CALLBACK = 5;
  public static FUNCTION_ACTION_CALL_JS_MODULE = 6;
  public static FUNCTION_ACTION_ON_WEBSOCKET_MESSAGE = 7;
  // Bridge状态定义
  public static BRIDGE_TYPE_SINGLE_THREAD = 2;
  public static BRIDGE_TYPE_NORMAL = 1;
  // 销毁状态
  public static DESTROY_CLOSE = 0;
  public static DESTROY_RELOAD = 1;
  public mContext: HippyEngineContext;
  public mCoreBundleLoader: HippyBundleLoader | null;
  public mEnableV8Serialization: boolean;
  public mIsDevModule: boolean;
  public mDebugServerHost: string;
  public mGroupId: number;
  public mThirdPartyAdapter: HippyThirdPartyAdapter | null;
  public mV8InitParams: V8InitParams | null;
  public mJsDriver: JsDriver;
  public mHippyBridge: HippyBridgeImpl;
  public mBridgeState: BridgeState = BridgeState.UNINITIALIZED;

  public constructor(
    context: HippyEngineContext,
    coreBundleLoader: HippyBundleLoader | null,
    enableV8Serialization: boolean,
    isDevModule: boolean,
    debugServerHost: string,
    groupId: number,
    thirdPartyAdapter: HippyThirdPartyAdapter | null,
    v8InitParams: V8InitParams | null,
    jsDriver: JsDriver) {

    this.mContext = context;
    this.mCoreBundleLoader = coreBundleLoader;
    this.mGroupId = groupId;
    this.mThirdPartyAdapter = thirdPartyAdapter;
    this.mEnableV8Serialization = enableV8Serialization;
    this.mIsDevModule = isDevModule;
    this.mDebugServerHost = debugServerHost;
    this.mJsDriver = jsDriver;
    this.mV8InitParams = v8InitParams;
    this.mHippyBridge = new HippyBridgeImpl(
      context,
      this,
      enableV8Serialization,
      isDevModule,
      debugServerHost,
      v8InitParams,
      jsDriver);
    if (enableV8Serialization) {
      // compatibleSerializer = new Serializer();
      // recommendSerializer = new com.tencent.mtt.hippy.serialization.recommend.Serializer();
    } else {
      // mStringBuilder = new StringBuilder(1024);
    }
  }

  runBundle(id: number, loader: HippyBundleLoader): void {
    loader.load(this.mHippyBridge, (result: number, reason: string) => {
      this.bundleLoaderCallback(result, reason);
    });
  }

  bundleLoaderCallback = (result: number, reason: string) => {
    LogUtils.i('hippy bundleLoaderCallback', result + reason)
    if (this.mContext) {
      if (result === 0) {
        this.mContext.onLoadModuleCompleted(ModuleLoadStatus.STATUS_OK, 'load module success')
      } else {
        this.mContext.onLoadModuleCompleted(ModuleLoadStatus.STATUS_ERR_RUN_BUNDLE, 'load module error. loader.load failed. check the file!!')
      }
    }
  }

  resumeInstance(id: number): void {
    let map = new Map<string, HippyAny>()
    map.set("id", id)
    const buffer = this.getSerializerBuffer(map);
    this.mHippyBridge.callFunction(HippyBridgeManagerImpl.FUNCTION_ACTION_RESUME_INSTANCE, this.callFunctionCallback, buffer, "");
  }

  pauseInstance(id: number): void {
    let map = new Map<string, HippyAny>()
    map.set("id", id)
    const buffer = this.getSerializerBuffer(map);
    this.mHippyBridge.callFunction(HippyBridgeManagerImpl.FUNCTION_ACTION_PAUSE_INSTANCE, this.callFunctionCallback, buffer, "");
  }

  destroyInstance(id: number): void {
    let map = new Map<string, HippyAny>()
    map.set("id", id)
    const buffer = this.getSerializerBuffer(map);
    this.mHippyBridge.callFunction(HippyBridgeManagerImpl.FUNCTION_ACTION_DESTROY_INSTANCE, this.callFunctionCallback, buffer, "");
  }

  getSerializerBuffer(params: Map<string, HippyAny>) {
    if (this.mEnableV8Serialization) {
      let binaryWriter = new BinaryWriter()
      let serializer = new Serializer(binaryWriter)
      serializer.writeHeader()
      serializer.writeValue(params)
      let buffer = binaryWriter.chunked()
      return buffer
    } else {
      let str = JSON.stringify(ArgumentUtils.mapToRecord(params))
      // let str = ArgumentUtils.objectToJson(params)
      let encoder = new util.TextEncoder("utf-8")
      let uint8Array = encoder.encodeInto(str)
      return uint8Array
    }
  }

  getSerializerBufferBinary(params: Map<string, HippyAny>) {
    let binaryWriter = new BinaryWriter()
    let serializer = new Serializer(binaryWriter)
    serializer.writeHeader()
    serializer.writeValue(params)
    let buffer = binaryWriter.chunked()
    return buffer;
  }

  execCallback(params: Map<string, HippyAny>): void {
    if (this.mBridgeState == BridgeState.INITIALIZED) {
      const buffer = this.getSerializerBuffer(params);

      let callId: string = params.get("callId") as string
      if (!callId) {
        callId = ""
      }
      this.mHippyBridge.callFunction(HippyBridgeManagerImpl.FUNCTION_ACTION_CALLBACK, this.callFunctionCallback, buffer, callId);
    }
  }

  public onBridgeDestroyed(isReload: boolean) {
    this.mContext.onBridgeDestroyed(isReload);
  }

  public callFunctionCallback = (result: number, reason: string) => {
    LogUtils.i('hippy execCallback', result + reason);
  }

  destroyBridge(isReload: boolean): void {
    this.mHippyBridge.destroy((result: number, reason: string) => {
      LogUtils.i('hippy destroyBridge', result + reason);
      this.onBridgeDestroyed(isReload);
    }, isReload);
    this.destroy();
  }

  destroy(): void {
    this.mBridgeState = BridgeState.DESTROYED;
  }

  callJavaScriptModule(moduleName: string, methodName: string, params: HippyMap): void {
    if (this.mBridgeState == BridgeState.INITIALIZED) {
      let map = new Map<string, HippyAny>()
      map.set("moduleName", moduleName)
      map.set("methodName", methodName)
      map.set("params", params)

      const buffer = this.getSerializerBuffer(map);
      this.mHippyBridge.callFunction(HippyBridgeManagerImpl.FUNCTION_ACTION_CALL_JS_MODULE, this.callJavaScriptModuleCallback, buffer, "");
    }
  }

  public callJavaScriptModuleCallback = (result: number, reason: string) => {
    LogUtils.i('hippy callJavaScriptModuleCallback', result + reason)
  }

  getThirdPartyAdapter(): HippyThirdPartyAdapter | null {
    return this.mThirdPartyAdapter;
  }

  callNatives(moduleName: string, moduleFunc: string, callId: string, params: ArrayBuffer): void {
    if (this.mContext != null) {
      const moduleManager = this.mContext.getModuleManager();
      if (moduleManager != null) {
        LogUtils.i('hippy', 'HippyBridgeManagerImpl callNatives params, '
          + moduleName + ' - '
          + moduleFunc + ' - '
          + callId + ' - '
          + params);
        const callNativeParams = new HippyCallNativeParams(moduleName, moduleFunc, callId, params);
        moduleManager.callNatives(callNativeParams);
      }
    }
  }

  reportException(message: string, stackTrace: string): void {
    if (this.mContext != null) {
      this.mContext.handleException(new HippyJsException(message, stackTrace));
    }
  }

  async initBridge(initBridgeCallback: NativeCallback): Promise<void> {
    this.mHippyBridge.initJSBridge(await this.getGlobalConfigsString(), (result: number, reason: string) => {
      this.initJSBridgeCallback(result, reason, initBridgeCallback);
    }, this.mGroupId);
  }

  initJSBridgeCallback(result: number, reason: string, initBridgeCallback: NativeCallback): void {
    LogUtils.i('hippy', 'jsDriver initialize callback, instanceId: %{public}d' + this.mJsDriver.instanceId);
    if (result != 0) {
      LogUtils.e('hippy', 'jsDriver initialize error ' + reason);
    }

    if (this.mContext != null) {
      this.mContext.onRuntimeInitialized();
    }
    // TODO
    if (this.enableTurbo()) {
      const mTurboModuleManager = new TurboModuleManager(this.mContext);
      // TODO runtimeID
      mTurboModuleManager.install(0);
    }
    // if (mThirdPartyAdapter != null) {
    //   mThirdPartyAdapter.onRuntimeInit(runtimeId);
    // }
    if (this.mCoreBundleLoader != null) {
      const timeMonitor = this.mContext.getMonitor();
      timeMonitor.startPoint(MonitorGroupType.ENGINE_INITIALIZE,
        TimeMonitor.MONITOR_POINT_LOAD_COMMON_JS);
      this.mCoreBundleLoader
        .load(this.mHippyBridge, (result: number, reason: string) => {
          this.bundleVendorLoaderCallback(result, reason, initBridgeCallback);
        });
    } else {
      this.mBridgeState = BridgeState.INITIALIZED;
      initBridgeCallback(1, '');
    }
  }

  public bundleVendorLoaderCallback = (result: number, reason: string, initBridgeCallback: NativeCallback) => {
    if (this.mBridgeState == BridgeState.DESTROYED) {
      return;
    }
    if (result == 0) {
      this.mBridgeState = BridgeState.INITIALIZED;
    } else {
      throw new Error("load coreJsBundle failed, check your core jsBundle:" + reason);
    }
    const timeMonitor = this.mContext.getMonitor();
    timeMonitor.startPoint(
      MonitorGroupType.ENGINE_INITIALIZE,
      TimeMonitor.MONITOR_POINT_NOTIFY_ENGINE_INITIALIZED);
    initBridgeCallback(result, reason);
  }

  loadInstance(name: string, id: number, params: HippyMap) {
    const timeMonitor = this.mContext.getMonitor()
    timeMonitor.startPoint(MonitorGroupType.LOAD_INSTANCE, TimeMonitor.MONITOR_POINT_LOAD_INSTANCE)

    let map = new Map<string, HippyAny>()
    map.set("name", name)
    map.set("id", id)
    map.set("params", params)

    const buffer = this.getSerializerBufferBinary(map);
    this.mHippyBridge.callFunction(HippyBridgeManagerImpl.FUNCTION_ACTION_LOAD_INSTANCE, this.callFunctionCallBack, buffer, "")
  }

  callFunctionCallBack = (result: number, reason: string) => {
    LogUtils.i('hippy callFunctionCallBack', result + reason)
  }

  async getGlobalConfigsString(): Promise<string> {
    let context = this.mContext.getGlobalConfigs().getContext();
    if (context == null) return ""
    let globalParams = new Map<string, HippyAny>()
    let dimensionMap = await DimensionsUtil.getDimensions(-1, -1, context, true);
    if (this.mContext.getGlobalConfigs() != null
      && this.mContext.getGlobalConfigs().getDeviceAdapter() != null) {
      this.mContext.getGlobalConfigs()
        .getDeviceAdapter()?.reviseDimensionIfNeed(context, dimensionMap, false,
        false);
    }
    DimensionsUtil.convertDimensionsToDp(dimensionMap);
    globalParams.set("Dimensions", dimensionMap)

    let packageName = ""
    let versionName = ""
    let pageUrl = ""
    let nightMode = false
    let extraDataMap = new Map<string, HippyValue>()

    // TODO support ThirdAdapter inject

    let platformParams = new Map<string, HippyAny>()
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

    let host = new Map<string, HippyAny>()
    host.set("url", (pageUrl == null) ? "" : pageUrl)
    host.set("appName", (packageName == null) ? "" : packageName)
    host.set("appVersion", (versionName == null) ? "" : versionName)
    host.set("nightMode", nightMode)
    host.set("extra", extraDataMap)
    globalParams.set("HostConfig", host)

    return JSON.stringify(ArgumentUtils.mapToRecord(globalParams));
  }

  private enableTurbo(): boolean {
    return this.mContext.getGlobalConfigs() != null && this.mContext.getGlobalConfigs()
      .enableTurbo();
  }
}
