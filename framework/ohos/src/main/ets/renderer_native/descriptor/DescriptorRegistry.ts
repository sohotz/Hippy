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
import { LogUtils } from '../../support/utils/LogUtils';

import { Descriptor, Tag } from './DescriptorBase';
import { CreateMutation, DeleteMutation, Mutation, MutationType,
  UpdateLayoutMutation,
  UpdateMutation } from './Mutation';

interface NodeData {
  type: string
  tag: number
  props: {}
  state: {}
  children: NodeData[]
}

type RootDescriptor = Descriptor<"RootView", any>

export class DescriptorRegistry {
  private rootId: number
  private descriptorByTag: Map<Tag, Descriptor>
  private descriptorListenersSetByTag: Map<Tag, Set<(descriptor: Descriptor) => void>> = new Map()

  constructor(rootId: number, descriptorByTag: Record<Tag, Descriptor>) {
    this.rootId = rootId
    this.descriptorByTag = new Map()
    for (const tag in descriptorByTag) {
      this.descriptorByTag.set(parseInt(tag), descriptorByTag[tag])
    }
  }

  buildNodeDataTree(node: Descriptor<string, {}>): NodeData {
    let ret = {
      type: node.type,
      tag: node.tag,
      props: node.props,
      state: node.state,
      children: []
    }
    if (node.childrenTags.length === 0) {
      return ret;
    }
    for(const tag of node.childrenTags) {
      ret.children.push(this.buildNodeDataTree(this.descriptorByTag.get(tag)))
    }
    return ret;
  }

  printNodeDataTree() {
    const retStr = JSON.stringify(this.buildNodeDataTree(this.descriptorByTag.get(this.rootId)))
    LogUtils.d('[Hippy]', `in printTree, retStr length: ${retStr.length}`)
    let start = 0
    const section = 800
    LogUtils.d('[Hippy]', `in printTree, section num: ${retStr.length / section}`)
    while (start <= retStr.length) {
      if (start + section < retStr.length) {
        LogUtils.d('[Hippy]', `in printTree, ${retStr.slice(start, start + section)}`)
      } else {
        LogUtils.d('[Hippy]', `in printTree, ${retStr.slice(start, retStr.length)}`)
      }
      start += section
    }
  }

  public getDescriptor<TDescriptor extends Descriptor>(tag: Tag): TDescriptor {
    return this.descriptorByTag.get(tag) as TDescriptor
  }

  public subscribeToDescriptorChanges(
    tag: Tag,
    listener: (descriptor: Descriptor) => void,
  ) {
    if (!this.descriptorListenersSetByTag.has(tag)) {
      this.descriptorListenersSetByTag.set(tag, new Set())
    }
    this.descriptorListenersSetByTag.get(tag)!.add(listener)
    return () => {
      this.removeDescriptorChangesListener(tag, listener)
    };
  }

  private removeDescriptorChangesListener(
    tag: Tag,
    listener: (descriptor: Descriptor) => void,
  ) {
    const callbacksSet = this.descriptorListenersSetByTag.get(tag)
    callbacksSet?.delete(listener)
    if (callbacksSet?.size === 0) {
      this.descriptorListenersSetByTag.delete(tag)
    }
  }

  public applyMutations(mutations: Mutation[]) {
    const updatedComponents = mutations.flatMap(
      mutation => this.applyMutation(mutation),
    );

    // debug
    this.printNodeDataTree()

    const uniqueUpdated = [...new Set(updatedComponents)]
    uniqueUpdated.forEach(tag => {
      const updatedDescriptor = this.getDescriptor(tag)
      if (!updatedDescriptor) {
        return
      }
      this.descriptorListenersSetByTag.get(tag)?.forEach(cb => {
        cb(updatedDescriptor)
      });
    });

    // TODO(hot):
  }

  private applyMutation(mutation: Mutation): Tag[] {
    if (mutation.type === MutationType.CREATE) {
      let theMutation = mutation as CreateMutation
      this.descriptorByTag.set(theMutation.descriptor.tag, theMutation.descriptor)
      this.descriptorByTag.get(theMutation.descriptor.parentTag).childrenTags.splice(
        theMutation.descriptor.index,
        0,
        theMutation.descriptor.tag,
      )
      return [theMutation.descriptor.tag, theMutation.descriptor.parentTag]
    } else if (mutation.type === MutationType.UPDATE) {
      let theMutation = mutation as UpdateMutation
      const currentDescriptor = this.descriptorByTag.get(theMutation.descriptor.tag)
      this.descriptorByTag.set(theMutation.descriptor.tag, {
        ...currentDescriptor,
        ...theMutation.descriptor,
        props: { ...currentDescriptor.props, ...theMutation.descriptor.props }
      })
      return [theMutation.descriptor.tag]
    } else if (mutation.type === MutationType.MOVE) {
      // TODO(hot):
      return []
    } else if (mutation.type === MutationType.MOVE2) {
      // TODO(hot):
      return []
    } else if (mutation.type === MutationType.DELETE) {
      let theMutation = mutation as DeleteMutation
      const currentDescriptor = this.descriptorByTag.get(theMutation.id)
      const index = currentDescriptor.index
      const tags = this.descriptorByTag.get(currentDescriptor.parentTag).childrenTags
      if (index >= 0 && index < tags.length && tags[index] == currentDescriptor.tag) {
        tags.splice(index, 1)
      } else if (tags.length > 0) {
        for (let i = 0; i < tags.length; i++) {
          if (tags[i] == currentDescriptor.tag) {
            tags.splice(i, 1)
            break;
          }
        }
      }
      this.deleteDescriptor(currentDescriptor)
      return [currentDescriptor.parentTag]
    } else if (mutation.type === MutationType.UPDATE_LAYOUT) {
      let theMutation = mutation as UpdateLayoutMutation
      const currentDescriptor = this.descriptorByTag.get(theMutation.id)
      this.descriptorByTag.set(theMutation.id, {
        ...currentDescriptor,
        layoutMetrics: {
          frame: {
            origin: {
              x: theMutation.left,
              y: theMutation.top,
            },
            size: {
              width: theMutation.width,
              height: theMutation.height,
            }
          }
        }
      })
      return [theMutation.id]
    }
    return []
  }

  private deleteDescriptor(descriptor: Descriptor) {
    for (let i = 0; i < descriptor.childrenTags.length; i++) {
      const childDescriptor = this.descriptorByTag.get(descriptor.childrenTags[i])
      this.deleteDescriptor(childDescriptor)
    }
    this.descriptorByTag.delete(descriptor.tag)
  }

  public createRootDescriptor(tag: Tag) {
    const rootDescriptor: RootDescriptor = {
      type: 'RootView',
      tag: this.rootId,
      childrenTags: [],
      index: 0,
      props: { top: 0, left: 0, width: 0, height: 0 },
      state: {},
      layoutMetrics: {
        frame: {
          origin: {
            x: 0,
            y: 0,
          },
          size: {
            width: 0,
            height: 0,
          }
        }
      }
    }
    this.descriptorByTag.set(tag, rootDescriptor)
  }

  public deleteRootDescriptor(tag: Tag) {
    if (this.getDescriptor(tag).type !== "RootView") {
      return
    }
    this.descriptorByTag.delete(tag)
  }
}
