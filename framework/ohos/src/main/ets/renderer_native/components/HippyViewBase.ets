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

import { ColorSegments, Descriptor } from '../descriptor/DescriptorBase';
import { convertColorSegmentsToString,
  convertMatrixArrayToMatrix4,
  TransformMatrix } from '../descriptor/DescriptorUtils';
import { NodeProps } from '../dom_node/NodeProps';
import { NativeGestureDispatcher } from '../NativeGestureDispatcher';
import { NativeRenderContext } from '../NativeRenderContext';

export type PointerEvents = "auto" | "none" | "box-none" | "box-only"

export class BorderRadii {
  topLeft?: number
  topRight?: number
  bottomLeft?: number
  bottomRight?: number
}

export class BorderColors {
  left?: ColorSegments
  top?: ColorSegments
  right?: ColorSegments
  bottom?: ColorSegments
}

export class BorderWidths {
  left?: number
  top?: number
  right?: number
  bottom?: number
}

export class ShadowOffset {
  width: number = 0
  height: number = 0
}

export class LayoutProps {
  top: number = 0
  left: number = 0
  width: number = 0
  height: number = 0
}

export class ViewBaseProps extends LayoutProps {
  borderStyle?: string
  backgroundColor?: number//ColorSegments
  borderWidth?: BorderWidths
  borderColor?: BorderColors
  borderLeftColor?: ColorSegments
  borderRightColor?: ColorSegments
  borderBottomColor?: ColorSegments
  borderRadius?: BorderRadii
  opacity?: number
  transform?: TransformMatrix
  overflow?: number
  zIndex?: number
  pointerEvents?: PointerEvents
  shadowColor?: ColorSegments
  shadowOffset?: ShadowOffset
  shadowOpacity?: number
  shadowRadius?: number
}

export type ViewBaseDescriptor = Descriptor<string, ViewBaseProps>

// TODO(hot): DynamicProps

function convertViewBaseDescriptor(descriptor: ViewBaseDescriptor): ViewBaseDescriptor {
  return descriptor as ViewBaseDescriptor
  // return {
  //   type: descriptor.type,
  //   tag: descriptor.tag,
  //   parentTag: descriptor.parentTag,
  //   childrenTags: descriptor.childrenTags,
  //   props: descriptor.props,
  //   state: descriptor.state,
  //   layoutMetrics: descriptor.layoutMetrics
  // }
}

@Component
export struct HippyViewBase {
  ctx!: NativeRenderContext
  tag!: number
  resetPosition: boolean = false
  @BuilderParam renderChildren: () => void
  @State private descriptor: ViewBaseDescriptor = {} as ViewBaseDescriptor
  private unregisterDescriptorChangesListener?: () => void = undefined

  aboutToAppear() {
    let descriptor = this.ctx.descriptorRegistry.getDescriptor<ViewBaseDescriptor>(this.tag)
    this.setDescriptor(descriptor)
    this.unregisterDescriptorChangesListener = this.ctx.descriptorRegistry.subscribeToDescriptorChanges(this.tag,
      (newDescriptor) => {
        const viewBaseDescriptor = (newDescriptor as ViewBaseDescriptor)
        this.setDescriptor(viewBaseDescriptor)
      }
    )
  }

  aboutToDisappear() {
    this.unregisterDescriptorChangesListener?.()
  }

  private setDescriptor(descriptor: ViewBaseDescriptor) {
    this.descriptor = convertViewBaseDescriptor(descriptor)
  }

  getHitTestMode(): HitTestMode {
    switch (this.descriptor.props.pointerEvents) {
      case "none":
        return HitTestMode.None
      case "box-none":
        return HitTestMode.None
      case "box-only":
        return HitTestMode.Block
      default:
        return HitTestMode.Default
    }
  }

  getShadow(): ShadowOptions | undefined {
    const shadowRadius = this.descriptor.props.shadowRadius
    if (shadowRadius === undefined || shadowRadius === 0) {
      return undefined
    }

    const props = this.descriptor.props
    const colorSegments = props.shadowColor ?? [0, 0, 0, 1]
    colorSegments[3] *= (props.shadowOpacity ?? 1)
    return {
      radius: shadowRadius,
      color: convertColorSegmentsToString(colorSegments),
      offsetX: props.shadowOffset?.width,
      offsetY: props.shadowOffset?.height,
    }
  }

  getBorderStyle(): BorderStyle{
    switch (this.descriptor.props.borderStyle) {
      case "dashed":
        return BorderStyle.Dashed
      case "dotted":
        return BorderStyle.Dotted
      default:
        return BorderStyle.Solid
    }
  }

  handleTouch(event: TouchEvent) {
    if (event.touches.length > 0) {
      let eventName = ""
      let touch = event.touches[0]
      switch (touch.type) {
        case TouchType.Down:
          eventName = NodeProps.ON_TOUCH_DOWN
          break;
        case TouchType.Up:
          eventName = NodeProps.ON_TOUCH_END
          break;
        case TouchType.Move:
          eventName = NodeProps.ON_TOUCH_MOVE
          break;
        case TouchType.Cancel:
          eventName = NodeProps.ON_TOUCH_CANCEL
          break;
        default:
          break;
      }
      NativeGestureDispatcher.handleTouchEvent(this.ctx, this.tag, touch.windowX, touch.windowY, eventName)
    }
  }

  handleClick(event: ClickEvent) {
    NativeGestureDispatcher.handleClickEvent(this.ctx, this.tag, NodeProps.ON_CLICK)
  }

  build() {
    Stack() {
      this.renderChildren()
    }
    .width(this.descriptor.layoutMetrics.frame.size.width)
    .height(this.descriptor.layoutMetrics.frame.size.height)
    .backgroundColor(this.descriptor.props.backgroundColor)//convertColorSegmentsToString(this.descriptor.props.backgroundColor))
    .position({
      y: (this.resetPosition ? 0 : this.descriptor.layoutMetrics.frame.origin.y),
      x: (this.resetPosition ? 0 : this.descriptor.layoutMetrics.frame.origin.x)
    })
    .borderWidth(2)//this.descriptor.props.borderWidth)
    // .borderColor({
    //   left: convertColorSegmentsToString(this.descriptor.props.borderColor!.left),
    //   top: convertColorSegmentsToString(this.descriptor.props.borderColor!.top),
    //   right: convertColorSegmentsToString(this.descriptor.props.borderColor!.right),
    //   bottom: convertColorSegmentsToString(this.descriptor.props.borderColor!.bottom),
    // })
    // .borderStyle(this.getBorderStyle())
    // .borderRadius(this.descriptor.props.borderRadius)
    // .opacity(this.descriptor.props.opacity)
    // .transform(this.descriptor.props.transform != undefined ? convertMatrixArrayToMatrix4(this.descriptor.props.transform) : undefined)
    // .clip((this.descriptor.props.overflow === 1 || this.descriptor.props.overflow === 2) ? true : false)
    // .hitTestBehavior(this.getHitTestMode())
    // .zIndex(this.descriptor.props.zIndex)
    // .shadow(this.getShadow())
    .onTouch((event: TouchEvent) => {
      //this.handleTouch(event)
    })
    .onClick((event: ClickEvent) => {
      this.handleClick(event)
    })
  }
}
