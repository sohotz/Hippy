//
// Created on 2024/6/7.
//
// Node APIs are not fully supported. To solve the compilation error of the interface cannot be found,
// please include "napi/native_api.h".

#pragma once

#include <string>

namespace hippy {
inline namespace render {
inline namespace native {

class HRUrlUtils {
public:
  static bool isWebUrl(std::string url);
  static std::string convertAssetImageUrl(const std::string &assetUrl);
};

} // namespace native
} // namespace render
} // namespace hippy