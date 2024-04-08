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
import { FetchResultCode } from '../http/HippyResourceLoader';
import fs from '@ohos.file.fs';
import { HippyHttpResponse } from '../http/HippyHttpResponse';

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

  public downloadFile(mDataHolder: ResourceDataHolder, mCallback: ProcessorCallback) {
    let filePath = this.ctx.filesDir + '/' + this.geFileName(mDataHolder.uri);
    let res = fs.accessSync(filePath);
    if (res) {
      let stat = fs.statSync(filePath);
      LogUtils.i(FILE_REQ_TAG, "get file info succeed, the size of file is " + stat.size);
      fs.unlinkSync(filePath);
      fs.stat(filePath).then((stat: fs.Stat) => {
        fs.unlink(filePath).then(() => {
          LogUtils.i(FILE_REQ_TAG, "remove file succeed");
        }).catch((err: HippyJsException) => {
          LogUtils.e(FILE_REQ_TAG, "remove file failed with error message: " + err.message);
        });
      }).catch((err: HippyJsException) => {
        LogUtils.e(FILE_REQ_TAG, "getFileInfo failed with error:" + err);
      });
    } else {
      LogUtils.i(FILE_REQ_TAG, "file not exists");
    }

    try {
      request.downloadFile(this.ctx, {
        url: mDataHolder.uri,
        filePath: filePath
      })
        .then((data) => {
          LogUtils.i(FILE_REQ_TAG, 'succ to request the download. result: ' + JSON.stringify(data.getTaskInfo()));
          data.getTaskInfo().then((downloadInfo: request.DownloadInfo) => {
            LogUtils.i(FILE_REQ_TAG, 'Succeeded in querying the download task')
            mDataHolder.resultCode = FetchResultCode.OK
            mDataHolder.addResponseHeaderProperty(HippyHttpResponse.HTTP_RESPONSE_STATUS_CODE,
              downloadInfo.status.toString());
            let filePath = downloadInfo.filePath
            let inputStream = fs.createStreamSync(filePath, "r+");
            inputStream.flushSync();
            if (downloadInfo.status != 0) {
              mDataHolder.errorMessage = downloadInfo.failedReason.toString()
            }
            mDataHolder.filePath = filePath;
            // mDataHolder.readResourceDataFromStream(inputStream);
            mCallback.onHandleCompleted();
            inputStream.close()
          }).catch((err: HippyJsException) => {
            LogUtils.e(FILE_REQ_TAG, `Failed to query the download task. message: ${err.message}`)
            mDataHolder.errorMessage = err.message
            mCallback.onHandleCompleted();
          });
          data.on('complete', () => {
            LogUtils.i(FILE_REQ_TAG, 'Download task completed.');
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
      LogUtils.e(FILE_REQ_TAG, `Failed to request the download. err: ${JSON.stringify(err)}`);
    }
  }

  private geFileName(url: string): string {
    // 找到最后一个斜杠的位置
    const lastIndex = url.lastIndexOf('/');
    // 如果找到了斜杠，则截取斜杠之后的部分作为图片名
    if (lastIndex !== -1) {
      return url.slice(lastIndex + 1);
    }
    // 如果没有找到斜杠，则整个URL可能就是图片名
    return url;
  }
}
