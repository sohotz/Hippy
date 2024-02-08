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
import UIAbility from '@ohos.app.ability.UIAbility';
import libHippy from 'libhippy_app.so'
import { HippyEngine } from './HippyEngine'
import { HippyEngineImpl } from './HippyEngineImpl'
import Want from '@ohos.app.ability.Want';
import AbilityConstant from '@ohos.app.ability.AbilityConstant';

export abstract class HippyAbility extends UIAbility {
  protected storage: LocalStorage | null = null
  protected hippyEngine: HippyEngine | null = null

  onCreate(want: Want, param: AbilityConstant.LaunchParam) {
    AppStorage.setOrCreate("HippyAbility", this)
  }

  public getHippyEngine(): HippyEngine {
    if (!this.hippyEngine) {
      this.hippyEngine = new HippyEngineImpl(libHippy, this.context.resourceManager)
      // TODO(hot):
      AppStorage.setOrCreate("HRManager", this.hippyEngine.getNativeRenderProvider().getNativeRenderImpl().getRenderManager())
      AppStorage.setOrCreate("HippyEngine", this.hippyEngine)
    }
    return this.hippyEngine
  }
}
