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

#include "vfs/handler/napi_delegate_handler.h"
#include "footstone/check.h"
#include "footstone/logging.h"
#include "footstone/string_view_utils.h"
#include "vfs/uri_loader.h"
#include "vfs/handler/asset_handler.h"
#include "vfs/handler/file_handler.h"
#include "vfs/job_response.h"
#include "vfs/request_job.h"

namespace hippy {
inline namespace vfs {

NapiDelegateHandler::AsyncWrapperMap NapiDelegateHandler::wrapper_map_;
std::atomic<uint32_t> NapiDelegateHandler::request_id_ = 1;

std::atomic<uint32_t> g_delegate_id = 1;

NapiDelegateHandler::NapiDelegateHandler() {

}

void NapiDelegateHandler::RequestUntrustedContent(
    std::shared_ptr<RequestJob> request,
    std::shared_ptr<JobResponse> response,
    std::function<std::shared_ptr<UriHandler>()> next) {
  FOOTSTONE_DCHECK(!next()) << "native delegate must be the last handler";

}

void NapiDelegateHandler::RequestUntrustedContent(
    std::shared_ptr<RequestJob> request,
    std::function<void(std::shared_ptr<JobResponse>)> cb,
    std::function<std::shared_ptr<UriHandler>()> next) {
  FOOTSTONE_DCHECK(!next()) << "native delegate must be the last handler";

}

}
}
