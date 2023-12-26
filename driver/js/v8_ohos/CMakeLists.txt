#
# Tencent is pleased to support the open source community by making
# Hippy available.
#
# Copyright (C) 2022 THL A29 Limited, a Tencent company.
# All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
cmake_minimum_required(VERSION 3.14)

project(v8)

add_library(${PROJECT_NAME} INTERFACE)
target_include_directories(${PROJECT_NAME}
  INTERFACE "include"
  INTERFACE "include/v8")

target_compile_definitions(${PROJECT_NAME}
  INTERFACE "-DV8_IMMINENT_DEPRECATION_WARNINGS"
  INTERFACE "-DV8_DEPRECATION_WARNINGS"
  #INTERFACE "-DV8_COMPRESS_POINTERS"
  )

add_library(${PROJECT_NAME}_library SHARED IMPORTED)
set_property(TARGET ${PROJECT_NAME}_library PROPERTY
  IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/lib/libv8android.so")
target_link_libraries(${PROJECT_NAME}
  INTERFACE ${PROJECT_NAME}_library)
