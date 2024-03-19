cmake_minimum_required(VERSION 3.14)

project("hippy")

add_compile_options("-Wno-unused-command-line-argument")

set(HIPPY_CPP_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(HIPPY_ROOT_DIR "${HIPPY_CPP_DIR}/../../../../")
set(PROJECT_ROOT_DIR "${HIPPY_ROOT_DIR}/../../")

set(HIPPY_FOOTSTONE_CPP_DIR "${PROJECT_ROOT_DIR}/modules/footstone")
set(HIPPY_OH_NAPI_CPP_DIR "${PROJECT_ROOT_DIR}/modules/ohos/oh_napi")
set(HIPPY_DRIVER_JS_CPP_DIR "${PROJECT_ROOT_DIR}/driver/js")
set(HIPPY_DOM_CPP_DIR "${PROJECT_ROOT_DIR}/dom")
set(HIPPY_VFS_CPP_DIR "${PROJECT_ROOT_DIR}/modules/vfs")

add_library(hippy STATIC #SHARED
  "${HIPPY_CPP_DIR}/entry.cc"
)

target_include_directories(hippy PUBLIC
  "${HIPPY_CPP_DIR}"
)

add_subdirectory("${HIPPY_FOOTSTONE_CPP_DIR}" ./footstone)
add_subdirectory("${HIPPY_OH_NAPI_CPP_DIR}" ./oh_napi)
#add_subdirectory("${HIPPY_DRIVER_JS_CPP_DIR}" ./driver_js)
add_subdirectory("${HIPPY_DOM_CPP_DIR}" ./dom)
#add_subdirectory("${HIPPY_VFS_CPP_DIR}" ./vfs)
add_subdirectory("./connector")

target_link_libraries(hippy PUBLIC
  libace_napi.z.so
  libhilog_ndk.z.so
  librawfile.z.so
  libnative_drawing.so
  uv
  footstone
  oh_napi
  js_driver
  dom
  vfs
  connector
)

#set(hippy_compile_options
#  -fno-rtti
#  -fno-exceptions
#)

target_compile_options(hippy PUBLIC ${hippy_compile_options} -std=c++17)
