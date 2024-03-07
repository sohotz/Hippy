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

import { HippyHttpAdapter } from './HippyHttpAdapter';
import { ResourceDataHolder } from '../../../vfs/ResourceDataHolder';
import { ResourceLoader } from '../../../vfs/ResourceLoader';
import { ProcessorCallback } from '../../../vfs/ProcessorCallback';
import { UrlUtils } from '../../../support/utils/UrlUtils';
import { HippyEngineContext } from '../../HippyEngineContext';
import { http } from '@kit.NetworkKit';

export enum FetchResultCode {
  OK,
  ERR_OPEN_LOCAL_FILE,
  ERR_UNKNOWN_SCHEME,
  ERR_REMOTE_REQUEST_FAILED,
}

export class HippyResourceLoader implements ResourceLoader {
  private static readonly RAWFILE_PREFIX = "hpfile://";
  private static readonly ASSETS_IMAGE_PREFIX = "assets://"
  private readonly engineContext: HippyEngineContext

  constructor(engineContext: HippyEngineContext) {
    this.engineContext = engineContext
  }

  fetchResourceAsync(holder: ResourceDataHolder, callback: ProcessorCallback) {
    if (UrlUtils.isWebUrl(holder.uri)) {
      let httpAdapter = this.engineContext.getGlobalConfigs().getHttpAdapter()
      if (httpAdapter) {
        httpAdapter.fetch(holder, http.RequestMethod.GET, this.engineContext.getNativeParams(), callback)
      }
    }
  }

  fetchResourceSync(holder: ResourceDataHolder): boolean {
    if (UrlUtils.isWebUrl(holder.uri)) {
      // TODO(eksea)
      holder.resource = holder.uri
    } else if (holder.uri.startsWith(HippyResourceLoader.RAWFILE_PREFIX)) {
      let realUri = "vue2/assets/" + holder.uri.substring(HippyResourceLoader.RAWFILE_PREFIX.length)
      holder.resource = $rawfile(realUri)
    }
    else if (holder.uri.startsWith(HippyResourceLoader.ASSETS_IMAGE_PREFIX)) {
      holder.resource = $rawfile(holder.uri.substring(HippyResourceLoader.ASSETS_IMAGE_PREFIX.length))
    } else {
      holder.resource = holder.uri
    }
    return true
  }
}
