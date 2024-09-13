/*
 *
 * Tencent is pleased to support the open source community by making
 * Hippy available.
 *
 * Copyright (C) 2019 THL A29 Limited, a Tencent company.
 * All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#pragma once

#include <string>
#include <native_drawing/drawing_font_collection.h>
#include <native_drawing/drawing_text_declaration.h>
#include <native_drawing/drawing_text_typography.h>
#include <native_drawing/drawing_register_font.h>

class OhFontCollection {
public:
  OhFontCollection();
  ~OhFontCollection();
  
  OH_Drawing_FontCollection *GetDrawingFontCollection() { return fontCollection_; }
  
  void RegisterFont(const std::string &familyName, const std::string &familySrc);
  
private:
  OH_Drawing_FontCollection *fontCollection_ = nullptr;
};
