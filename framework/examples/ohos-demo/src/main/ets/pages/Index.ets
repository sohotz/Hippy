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

@Entry
@Component
struct Index {

  build() {
    Column() {
      Text("vue2")
        .backgroundColor(Color.White)
        .markAnchor({x: '50%', y: '50%'})
        .position({x: '50%', y: '30%'})
        .size({width:100, height: 40})
        .fontSize(16)
        .fontWeight(FontWeight.Bold)
        .textAlign(TextAlign.Center)
        .borderWidth(1)
        .onClick(() => {
          router.pushUrl({
            url: 'pages/ExampleHippyPage',
            params: {}
          })
        })
    }
  }

}
