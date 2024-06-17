# 对于需要源码依赖 Hippy c++ 代码的特别需求，需要做2点：
# 1 集成 Hippy c++ 源码，生成使用方自己的 so，类似下面配置。
# 2 Hippy 内部不再编译 hippy so，注释掉 Hippy/framework/ohos/src/main/cpp/CMakeLists.txt 里内容。

#[[
project(hippy_user)
cmake_minimum_required(VERSION 3.14)

set(DEMO_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../../")
set(PROJECT_ROOT_DIR "${DEMO_ROOT_DIR}/../../../")
set(HIPPY_ROOT_DIR "${PROJECT_ROOT_DIR}/framework/ohos/")
set(HIPPY_IMPL_CPP_DIR "${HIPPY_ROOT_DIR}/src/main/cpp/impl")

add_subdirectory("${HIPPY_IMPL_CPP_DIR}" ./hippy_impl)

add_library(hippy_user SHARED

)

target_link_libraries(hippy_user PUBLIC hippy_impl)
]]
