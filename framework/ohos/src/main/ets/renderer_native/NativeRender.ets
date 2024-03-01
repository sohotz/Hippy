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
import { HippyExecutor } from '../support/common/HippyExecutor';
import { VfsManager } from '../vfs/VfsManager';
import { ImageDecoderAdapter } from './components/image/ImageDecoderAdapter';
import { ImageLoaderAdapter } from './components/image/ImageLoaderAdapter';
import { FontAdapter } from './components/richtext/FontAdapter';
import { FrameworkProxy } from './FrameworkProxy'
import { HRManager } from './uimanager/HRManager';

export interface NativeRender {
  getRenderManager(): HRManager

  getBundlePath(): string

  getImageLoader(): ImageLoaderAdapter | null

  // TODO(eksea)
  // getImageDecoderAdapter(): ImageDecoderAdapter | null

  getVfsManager(): VfsManager | null

  // TODO(eksea)
  // getFontAdapter(): FontAdapter | null

  // TODO(eksea)
  // getBackgroundExecutor(): HippyExecutor | null

  getEngineId(): number

  onSizeChanged(rootId: number, width: number, height: number, oldWidth: number, oldHeight: number): void

  onSizeChanged2(rootId: number, nodeId: number, width: number, height: number, isSync: boolean): void

  updateDimension(width: number, height: number, shouldUseScreenDisplay: boolean, systemUiVisibilityChanged: boolean): void

}
