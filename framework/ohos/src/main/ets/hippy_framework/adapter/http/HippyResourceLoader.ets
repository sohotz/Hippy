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
import { ResourceDataHolder } from 'ets/vfs/ResourceDataHolder';
import { ResourceLoader } from '../../../vfs/ResourceLoader';
import { ProcessorCallback } from './ProcessorCallback';

export enum FetchResultCode {
  OK,
  ERR_OPEN_LOCAL_FILE,
  ERR_UNKNOWN_SCHEME,
  ERR_REMOTE_REQUEST_FAILED,
}

export class HippyResourceLoader implements ResourceLoader {
  constructor() {
  }

  fetchResourceAsync(holder: ResourceDataHolder, callback: ProcessorCallback) {
  }

  fetchResourceSync(holder: ResourceDataHolder): boolean {
    return true
  }
}
