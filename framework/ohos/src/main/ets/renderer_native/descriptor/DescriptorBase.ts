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

/** 0-1 */
export type NormalizedScalar = number;

/** RGBA */
export type ColorSegments = [
  NormalizedScalar,
  NormalizedScalar,
  NormalizedScalar,
  NormalizedScalar,
];

export type ColorValue = number;

export type Tag = number;

export type LayoutMetrics = {
  frame: {
    origin: {
      x: number;
      y: number;
    };
    size: {
      width: number;
      height: number;
    };
  };
};

export class Descriptor<TType = string, TProps extends Object = Object, TState = {}> {
  type: TType
  tag: Tag
  parentTag?: Tag
  childrenTags: Tag[]
  index: number
  props: TProps
  state: TState

  layoutMetrics: LayoutMetrics
};
