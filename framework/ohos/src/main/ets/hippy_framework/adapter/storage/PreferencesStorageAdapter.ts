import { HippyStorageAdapter } from './HippyStorageAdapter'
import dataPreferences from '@ohos.data.preferences';
import { Context } from '@ohos.abilityAccessCtrl';
import { BusinessError } from '@ohos.base';

const PREFERENCES_NAME = 'HippyStorage'

export class DefaultStorageAdapter implements HippyStorageAdapter {
  constructor(protected context: Context) {

  }

  multiGet(keys: string[]): Promise<[key: string, value: string][]> {
    return new Promise((resolve, reject) => {
      this.getHippyPreferences()
        .then((preferences) => {
          const result = new Array<[key: string, value: string]>(keys.length);
          keys.forEach((key, index) => {
            result[index] = [key, preferences.getSync(key, '') as string];
          });
          return resolve(result);
        })
        .catch((err: BusinessError) => reject(err))
    });
  }

  multiSet(pairs: [key: string, value: string][]): Promise<void> {
    return new Promise((resolve, reject) => {
      this.getHippyPreferences()
        .then((preferences) => {
          pairs.forEach((kv) => preferences.putSync(kv[0], kv[0]));
          return preferences.flush()
        })
        .then(() => resolve())
        .catch((err: BusinessError) => reject(err))
    });
  }

  multiRemove(keys: string[]): Promise<void> {
    return new Promise((resolve, reject) => {
      this.getHippyPreferences()
        .then((preferences) => {
          keys.forEach((key, _) => {
            preferences.deleteSync(key);
          });
          preferences.flush()
        })
        .then(() => resolve())
        .catch((err: BusinessError) => reject(err))
    });
  }

  getAllKeys(): Promise<string[]> {
    return new Promise((resolve, reject) => {
      this.getHippyPreferences()
        .then((preferences) => preferences.getAll())
        .then((value: Object) => {
          let allKeys = this.getObjKeys(value);
          return resolve(allKeys);
        })
        .catch((err: BusinessError) => reject(err))
    });
  }

  getHippyPreferences(): Promise<dataPreferences.Preferences> {
    return dataPreferences.getPreferences(this.context, PREFERENCES_NAME);
  }

  getObjKeys(obj: Object): string[] {
    let keys = Object.keys(obj);
    return keys;
  }
}
