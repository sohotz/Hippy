//
// Created on 2024/6/7.
//
// Node APIs are not fully supported. To solve the compilation error of the interface cannot be found,
// please include "napi/native_api.h".

#include <regex>
#include "renderer/utils/hr_url_utils.h"

namespace hippy {
inline namespace render {
inline namespace native {

bool HRUrlUtils::isWebUrl(std::string url) {
  std::regex regex("^https?://.+");
  return std::regex_search(url, regex);
}

}
}
}