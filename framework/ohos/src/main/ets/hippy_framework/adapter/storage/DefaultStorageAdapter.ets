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
import { HippyStorageAdapter } from "./HippyStorageAdapter";
import relationalStore from '@ohos.data.relationalStore';
import { BusinessError } from '@ohos.base';
import { Context } from '@ohos.abilityAccessCtrl';
import { ValuesBucket } from '@ohos.data.ValuesBucket';
import hilog from '@ohos.hilog';
import { LogUtils } from '../../../support/utils/LogUtils';

const TAG = 'DefaultStorageAdapter'

const STORE_DB_NAME = 'HippyStorage.db'
const TABLE_STORAGE = 'hippy_engine_storage'
const COLUMN_KEY = 'storage_key'
const COLUMN_VALUE = 'storage_value'

export class DefaultStorageAdapter implements HippyStorageAdapter {
  private hippyStoreDb: relationalStore.RdbStore | undefined = undefined;

  constructor(context: Context) {
    const STORE_CONFIG: relationalStore.StoreConfig = {
      name: STORE_DB_NAME,
      securityLevel: relationalStore.SecurityLevel.S1
    };

    relationalStore.getRdbStore(context, STORE_CONFIG)
      .then(async (rdbStore: relationalStore.RdbStore) => {
        LogUtils.i(TAG, 'Get RdbStore successfully.')
        rdbStore.executeSql(`CREATE TABLE IF NOT EXISTS ${TABLE_STORAGE} (${COLUMN_KEY} TEXT PRIMARY KEY, ${COLUMN_VALUE} TEXT);`)
          .then(() => {
            this.hippyStoreDb = rdbStore;
          })
      })
      .catch((err: BusinessError) => {
        LogUtils.e(TAG, `Get RdbStore failed, code is ${err.code},message is ${err.message}`);
      });
  }

  multiGet(keys: string[]): Promise<[key: string, value: string][]> {
    return new Promise((resolve, reject) => {
      if (this.hippyStoreDb === undefined) {
        return reject("database is not ready");
      }

      if (keys === null || keys === undefined || keys.length === 0) {
        return resolve([]);
      }

      let predicates = new relationalStore.RdbPredicates(TABLE_STORAGE);
      predicates.in(COLUMN_KEY, keys);
      (this.hippyStoreDb as relationalStore.RdbStore).query(
        predicates,
        [COLUMN_KEY, COLUMN_VALUE],
        (err, resultSet) => {
          if (err) {
            LogUtils.e(TAG, `Query failed, code is ${err.code}, message is ${err.message}`);
            return reject(err);
          }
          LogUtils.i(TAG, `ResultSet column names: ${resultSet.columnNames}, column count: ${resultSet.columnCount}`);
          let results = new Array<[key: string, value: string]>(resultSet.rowCount);
          let i = 0;
          while (resultSet.goToNextRow()) {
            results[i] = [
              resultSet.getString(resultSet.getColumnIndex(COLUMN_KEY)),
              resultSet.getString(resultSet.getColumnIndex(COLUMN_VALUE))
            ];
            i++;
          }
          resultSet.close();
          return resolve(results);
        });
    });
  }

  multiSet(pairs: [key: string, value: string][]): Promise<void> {
    return new Promise((resolve, reject) => {
      if (this.hippyStoreDb === undefined) {
        return reject("database is not ready");
      }
      if (pairs === null || pairs === undefined || pairs.length === 0) {
        return resolve();
      }

      const valueBuckets = pairs.map<ValuesBucket>((pair) => {
        return { 'storage_key': pair[0], 'storage_value': pair[1] }
      });

      (this.hippyStoreDb as relationalStore.RdbStore).batchInsert(
        TABLE_STORAGE,
        valueBuckets,
        (err: BusinessError, rowId: number) => {
          if (err) {
            LogUtils.e(TAG, `Insert is failed, code is ${err.code}, message is ${err.message}`);
            return reject();
          }
          LogUtils.i(TAG, `Insert is successful, rowId = ${rowId}`);
          return resolve();
        })
    });
  }

  multiRemove(keys: string[]): Promise<void> {
    return new Promise((resolve, reject) => {
      if (this.hippyStoreDb === undefined) {
        reject("database is not ready");
        return;
      }

      if (keys === null || keys === undefined || keys.length === 0) {
        resolve();
        return;
      }

      let predicates = new relationalStore.RdbPredicates(TABLE_STORAGE);
      predicates.in(COLUMN_KEY, keys);
      if (this.hippyStoreDb != undefined) {
        (this.hippyStoreDb as relationalStore.RdbStore).delete(predicates, (err, rows) => {
          if (err) {
            LogUtils.e(TAG, `Delete failed, code is ${err.code}, message is ${err.message}`);
            return reject(err);
          }
          LogUtils.i(TAG, `Delete rows: ${rows}`);
          return resolve();
        })
      }
    });
  }

  getAllKeys(): Promise<string[]> {
    return new Promise((resolve, reject) => {
      if (this.hippyStoreDb === undefined) {
        reject("database is not ready");
        return;
      }

      let predicates = new relationalStore.RdbPredicates(TABLE_STORAGE);
      (this.hippyStoreDb as relationalStore.RdbStore).query(
        predicates,
        [COLUMN_KEY],
        (err, resultSet) => {
          if (err) {
            LogUtils.e(TAG, `Query failed, code is ${err.code}, message is ${err.message}`);
            return reject(err);
          }
          LogUtils.i(TAG, `ResultSet column names: ${resultSet.columnNames}, column count: ${resultSet.columnCount}`);
          let results = new Array<string>(resultSet.rowCount);
          let i = 0;
          while (resultSet.goToNextRow()) {
            results[i] = resultSet.getString(resultSet.getColumnIndex(COLUMN_KEY));
            i++;
          }
          resultSet.close();
          return resolve(results);
        })
    });
  }
}
