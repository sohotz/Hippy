project(hippy)
cmake_minimum_required(VERSION 3.14)

set(DEMO_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../../")
set(PROJECT_ROOT_DIR "${DEMO_ROOT_DIR}/../../../")
set(HIPPY_ROOT_DIR "${PROJECT_ROOT_DIR}/framework/ohos/")
set(HIPPY_IMPL_CPP_DIR "${HIPPY_ROOT_DIR}/src/main/cpp/impl")

add_subdirectory("${HIPPY_IMPL_CPP_DIR}" ./hippy_impl)

add_library(hippy SHARED

)

target_link_libraries(hippy PUBLIC hippy_impl)
