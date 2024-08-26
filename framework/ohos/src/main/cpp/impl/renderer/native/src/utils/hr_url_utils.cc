//
// Created on 2024/6/7.
//
// Node APIs are not fully supported. To solve the compilation error of the interface cannot be found,
// please include "napi/native_api.h".

#include <filemanagement/file_uri/oh_file_uri.h>
#include <regex>
#include "renderer/utils/hr_url_utils.h"
#include "footstone/logging.h"

namespace hippy {
inline namespace render {
inline namespace native {

char *HRUrlUtils::uriPrefix_ = nullptr;

bool HRUrlUtils::isWebUrl(std::string url) {
  std::regex regex("^https?://.+");
  return std::regex_search(url, regex);
}

std::string HRUrlUtils::convertAssetImageUrl(bool isRawfile, const std::string &resModuleName, const std::string &assetUrl) {
  const std::string assetPrefix = "asset:/";
  if (assetUrl.find(assetPrefix) == 0) {
    if (isRawfile) {
      std::string resourceStr = std::string("resource://RAWFILE/");
      resourceStr += assetUrl.substr(assetPrefix.size());
      return resourceStr;
    } else {
      if(uriPrefix_ == nullptr) {
        FileManagement_ErrCode err = OH_FileUri_GetUriFromPath("", 0, &uriPrefix_);
        if(err != FileManagement_ErrCode::ERR_OK) {
          return assetUrl;
        }
      }
      std::string resourceStr = std::string(uriPrefix_)
        + "/data/storage/el1/bundle/" + resModuleName + "/resources/resfile/"
        + assetUrl.substr(assetPrefix.size());
      return resourceStr;
    }
  } else {
    return assetUrl;
  }
}

}
}
}