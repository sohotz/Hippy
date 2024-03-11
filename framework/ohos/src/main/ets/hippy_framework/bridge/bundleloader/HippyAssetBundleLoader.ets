/* Tencent is pleased to support the open source community by making Hippy available.
 * Copyright (C) 2018 THL A29 Limited, a Tencent company. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
import { TextUtils } from '../../../support/utils/TextUtils';
import { NativeCallback } from '../../connector/NativeCallback';
import { HippyBridge } from '../HippyBridge';
import { HippyBundleLoader } from './HippyBundleLoader';


export class HippyAssetBundleLoader implements HippyBundleLoader {

  private ASSETS_STR = "assets://";
  private mContext: Context;

  private mAssetPath = '';

  private mCanUseCodeCache = false;

  private mCodeCacheTag = '';



  public constructor(
      context: Context,
      assetName: string,
      canUseCodeCache: boolean,
      codeCacheTag: string) {
        this.mContext = context;
        this.mAssetPath = assetName;
        this.mCanUseCodeCache = canUseCodeCache;
        this.mCodeCacheTag = codeCacheTag;
  }

  public setCodeCache(canUseCodeCache: boolean, codeCacheTag: string) {
    this.mCanUseCodeCache = canUseCodeCache;
    this.mCodeCacheTag = codeCacheTag;
  }

  public load(bridge: HippyBridge, callback: NativeCallback) {
    if (TextUtils.isEmpty(this.mAssetPath)) {
      return;
    }

    // AssetManager assetManager = mContext.getAssets();
    // String uri = mAssetPath;
    // if (!mAssetPath.startsWith(URI_SCHEME_ASSETS)) {
    //   if (mAssetPath.startsWith("/")) {
    //     uri = URI_SCHEME_ASSETS + mAssetPath;
    //   } else {
    //     uri = URI_SCHEME_ASSETS + "/" + mAssetPath;
    //   }
    // }
    //
    // boolean ret = bridge
    //     .runScriptFromUri(uri, assetManager, mCanUseCodeCache, mCodeCacheTag, callback);
    // LogUtils.d("HippyAssetBundleLoader", "load: ret" + ret);
  }

  public getPath(): string {
    if (this.mAssetPath != null && !this.mAssetPath.startsWith(this.ASSETS_STR)) {
      return this.ASSETS_STR + this.mAssetPath;
    } else {
      return this.mAssetPath;
    }
  }

  public getRawPath(): string {
    return this.mAssetPath;
  }

  public getBundleUniKey(): string {
    return this.getPath();
  }

  public canUseCodeCache(): boolean {
    return this.mCanUseCodeCache;
  }

  public getCodeCacheTag(): string {
    return this.mCodeCacheTag;
  }

}
