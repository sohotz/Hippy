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
import { router } from '@kit.ArkUI'

//dimens.xml
const page_index_item_margin:number = 16
const page_item_default_width:number = 164
const page_item_default_height:number = 239
const page_item_add_image_width:number = 38
const page_item_add_image_height:number = 38
const page_item_attribute_prompt_height:number = 23

@Entry
@Component
struct PageManagement {
  @State pageitems:number[] = []

  build() {
    Column() {
      Row() {
        RelativeContainer() {
          Row() {
            Image($r('app.media.back_2x'))
              .width(9)
              .height(17)
              .margin({ left: 10})
          }
          .width(55)
          .height(54)
          .onClick(() => {
            router.back()
          })
          .id('btn_back')
          .justifyContent(FlexAlign.Center)

          Text("Page Management")
            .fontColor(Color.Black)
            .fontWeight(500)
            .fontSize(17)
            .id('title')
            .alignRules({ middle: { anchor: "__container__", align: HorizontalAlign.Center},
              center: { anchor: "__container__", align: VerticalAlign.Center}})
        }
      }
      .height(54)
      .backgroundColor(Color.White)
      .width('100%')

      //getPageIndexItemWidth ===> margin | grid | margin | grid margin
      Grid() {
        //generateBitmapFromView <====> .ets
        ForEach(this.pageitems, ()=>{
          GridItem() { //TODO: no android:elevation="3dp"
            Column() {
              Row() {
                Image($r('app.media.startIcon'))
                  .width(page_item_add_image_width)
                  .height(page_item_add_image_height)
                  .onClick(()=>{
                    router.pushUrl({
                      url: 'pages/PageConfiguration',
                      params: {}
                    })
                  })
              }
              .width('100%')
              .height('calc(100% - 23vp)')  //page_item_attribute_prompt_height
              .backgroundColor(Color.White)
              .justifyContent(FlexAlign.Center)

              Row() {
                Row() {
                  Image($r('app.media.delete_page_2x'))
                    .width(24)
                    .height(24)
                    //.margin({ top: 2, right: 3 }) //container's width 30,height 28 calc margin
                }
                .width(30)
                .height(28)
                .onClick(()=>{
                  this.pageitems.pop()
                })
              }
              .width('100%')
              .position({x:0, y:0})
              .justifyContent(FlexAlign.End)

              Row() {
                Image($r('app.media.page_item_tips_4x'))
                  .width(12)
                  .height(12)

                Text('Reactive')
                  .margin({left:4})
                  .fontColor('#4A5970')
                  .fontWeight(400)
                  .fontSize(12)
              }
              .height(17)
              .margin({top:6})
            }
          }
          .width('100%')
          .aspectRatio(page_item_default_width/page_item_default_height)
        })

        //add page item
        GridItem() { //TODO: no android:elevation="3dp"
          Column() {
            Row() {
              Image($r('app.media.add_page_2x'))
                .width(page_item_add_image_width)
                .height(page_item_add_image_height)
                .onClick(()=>{
                  router.pushUrl({
                    url: 'pages/PageConfiguration',
                    params: {}
                  })
                })
            }
            .width('100%')
            .height('calc(100% - 23vp)')  //page_item_attribute_prompt_height
            .backgroundColor(Color.White)
            .justifyContent(FlexAlign.Center)

            Row() {
              Image($r('app.media.page_item_add_4x'))
                .width(12)
                .height(12)

              Text($r('app.string.page_add_item_tips_text'))
                .margin({left:4})
                .fontColor('#4A5970')
                .fontWeight(400)
                .fontSize(12)
            }
            .height(17)
            .margin({top:6})
          }
        }
        .width('100%')
        .aspectRatio(page_item_default_width/page_item_default_height)
      }
      .columnsTemplate('1fr 1fr')
      .columnsGap(page_index_item_margin)
      .rowsGap(page_index_item_margin)
      //TODO:marginTop is not same as android
      .margin({left:page_index_item_margin, right:page_index_item_margin, top:page_index_item_margin})
    }
    .width('100%')
    .height('100%')
    .backgroundColor($r('app.color.home_background'))
    //.expandSafeArea()
  }
}
