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

import { Descriptor, Tag } from './DescriptorBase'

export enum MutationType {
  CREATE   = 1,
  UPDATE   = 2,
  MOVE     = 4,
  MOVE2    = 8,
  DELETE   = 16,
  UPDATE_LAYOUT = 32,
}

export class MutationBase {
  type: MutationType
}

export class CreateMutation extends MutationBase {
  descriptor: Descriptor
  constructor() {
    super()
    this.type = MutationType.CREATE
  }
}

export class UpdateMutation extends MutationBase {
  descriptor: Descriptor
  constructor() {
    super()
    this.type = MutationType.UPDATE
  }
}

export class MoveMutation extends MutationBase {
  pid: number
  descriptor: Descriptor
  constructor() {
    super()
    this.type = MutationType.MOVE
  }
}

export class Move2Mutation extends MutationBase {
  id: number
  to_pid: number
  from_pid: number
  index: number
  constructor() {
    super()
    this.type = MutationType.MOVE2
  }
}

export class DeleteMutation extends MutationBase {
  id: number
  constructor() {
    super()
    this.type = MutationType.DELETE
  }
}

export class UpdateLayoutMutation extends MutationBase {
  id: number
  left: number
  top: number
  width: number
  height: number
  constructor() {
    super()
    this.type = MutationType.UPDATE_LAYOUT
  }
}

export type Mutation =
  | CreateMutation
  | UpdateMutation
  | MoveMutation
  | Move2Mutation
  | DeleteMutation
  | UpdateLayoutMutation
