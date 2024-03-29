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
import { ProcessorCallback } from '../../../vfs/ProcessorCallback';
import { ResourceDataHolder } from '../../../vfs/ResourceDataHolder';
import { HippyHttpResponse } from '../http/HippyHttpResponse';
import fs from '@ohos.file.fs';
import { FetchResultCode } from '../http/HippyResourceLoader';

const FILE_REQ_TAG = "FileRequestAdapter"

export class FileRequestAdapter implements HippyFileAdapter {
  private ctx: Context;

  constructor(context: Context) {
    this.ctx = context;
  }

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
        request.uploadFile(this.ctx, uploadConfig).then((data) => {
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

  public downloadFile(mDataHolder: ResourceDataHolder, mCallback: ProcessorCallback): Promise<request.DownloadTask> {
    let downloadConfig: request.DownloadConfig = {
      url: mDataHolder.uri,
      header: mDataHolder.requestHeaders ?? '',
    };
    return new Promise<request.DownloadTask>((resolve, reject) => {
      try {
        request.downloadFile(this.ctx, downloadConfig)
          .then((data) => {
            LogUtils.i(FILE_REQ_TAG, 'succ to request the download. result: ' + JSON.stringify(data));
            data.on('complete', () => {
              LogUtils.i(FILE_REQ_TAG, 'Download task completed.');
              data.getTaskInfo().then((downloadInfo: request.DownloadInfo) => {
                LogUtils.i(FILE_REQ_TAG, 'Succeeded in querying the download task')
                mDataHolder.resultCode = downloadInfo.status == 0 ? FetchResultCode.OK : downloadInfo.status
                mDataHolder.addResponseHeaderProperty(HippyHttpResponse.HTTP_RESPONSE_STATUS_CODE,
                  downloadInfo.status.toString());
                let filePath = downloadInfo.filePath + downloadInfo.fileName;
                let inputStream = fs.createStreamSync(filePath, "r+");
                if (downloadInfo.status != 0 || inputStream == null) {
                  mDataHolder.errorMessage = downloadInfo.failedReason.toString()
                }
                mDataHolder.readResourceDataFromStream(inputStream);
                mCallback.onHandleCompleted();
              }).catch((err: HippyJsException) => {
                LogUtils.e(FILE_REQ_TAG, `Failed to query the download task. message: ${err.message}`)
                mDataHolder.errorMessage = err.message
                mCallback.onHandleCompleted();
              });
            })

            data.on('fail', (err) => {
              LogUtils.e(FILE_REQ_TAG, `Failed to query the download task. message: ${err}`)
              mDataHolder.errorMessage = err.toString()
              mCallback.onHandleCompleted();
            });
          })
          .catch((err: HippyJsException) => {
            mDataHolder.resultCode = FetchResultCode.ERR_REMOTE_REQUEST_FAILED
            mDataHolder.errorMessage = err.message;
            mCallback.onHandleCompleted();
            LogUtils.e(FILE_REQ_TAG, 'Failed to request the download. Cause: ' + JSON.stringify(err));
          })
      } catch (err) {
        LogUtils.e(FILE_REQ_TAG, 'err.code : ' + err.code + ', err.message : ' + err.message);
      }
    })
  }
}
