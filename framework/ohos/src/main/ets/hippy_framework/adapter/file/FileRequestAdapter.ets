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
import { HippyFileAdapter } from './HippyFileAdapter';
import request from '@ohos.request';
import { HippyJsException } from '../../common/HippyJsException';
import { LogUtils } from '../../../support/utils/LogUtils';

const FILE_REQ_TAG = "FileRequestAdapter"

export class FileRequestAdapter implements HippyFileAdapter {
  public uploadFile(reqUrl: string, reqHaeder: object, reqMethod: string, reqFiles: request.File[], reqData: request.RequestData[]): Promise<request.UploadTask> {
    let uploadConfig: request.UploadConfig = {
      url: reqUrl,
      header: reqHaeder,
      method: reqMethod,
      files: reqFiles,
      data: reqData,
    };
    return new Promise<request.UploadTask>(() => {
      try {
        request.uploadFile(globalThis.abilityContext, uploadConfig).then((data) => {
          LogUtils.i(FILE_REQ_TAG, 'succ to request the upload. result: ' + JSON.stringify(data));
          return data
        }).catch((err: HippyJsException) => {
          LogUtils.e(FILE_REQ_TAG, 'Failed to request the upload. Cause: ' + JSON.stringify(err));
        });
      } catch (err) {
        LogUtils.e(FILE_REQ_TAG, 'err.code : ' + err.code + ', err.message : ' + err.message);
      }
    })
  }

  public downloadFile(url: string, header: object, path: string, networkType: number): Promise<request.DownloadTask> {
    let downloadConfig: request.DownloadConfig = {
      url: url,
      header: header,
      filePath: path,
      networkType: networkType
    };
    return new Promise<request.DownloadTask>((resolve, reject) => {
      try {
        request.downloadFile(globalThis.abilityContext, downloadConfig)
          .then((data) => {
            LogUtils.i(FILE_REQ_TAG, 'succ to request the download. result: ' + JSON.stringify(data));
            return data;
          })
          .catch((err: HippyJsException) => {
            LogUtils.e(FILE_REQ_TAG, 'Failed to request the download. Cause: ' + JSON.stringify(err));
          })
      } catch (err) {
        LogUtils.e(FILE_REQ_TAG, 'err.code : ' + err.code + ', err.message : ' + err.message);
      }
    })
  }
}
