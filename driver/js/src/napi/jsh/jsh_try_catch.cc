/*
 *
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
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include "driver/napi/jsh/jsh_try_catch.h"

#include "footstone/string_view.h"
#include "footstone/string_view_utils.h"
#include "driver/napi/jsh/jsh_ctx.h"
#include "driver/napi/jsh/jsh_ctx_value.h"
#include "driver/vm/jsh/jsh_vm.h"

namespace hippy {
inline namespace driver {
inline namespace napi {

using string_view = footstone::string_view;

std::shared_ptr<TryCatch> CreateTryCatchScope(bool enable,
                                              std::shared_ptr<Ctx> ctx) {
  return std::make_shared<JSHTryCatch>(enable, ctx);
}

JSHTryCatch::JSHTryCatch(bool enable, const std::shared_ptr<Ctx>& ctx)
    : TryCatch(enable, ctx) {//, try_catch_(nullptr) {
  if (enable) {
//     std::shared_ptr<V8Ctx> v8_ctx = std::static_pointer_cast<V8Ctx>(ctx);
//     if (v8_ctx) {
//       try_catch_ = std::make_shared<v8::TryCatch>(v8_ctx->isolate_);
//     }
  }
}

JSHTryCatch::~JSHTryCatch() = default;

void JSHTryCatch::ReThrow() {
//   if (try_catch_) {
//     try_catch_->ReThrow();
//   }
}

bool JSHTryCatch::HasCaught() {
//   if (try_catch_) {
//     return try_catch_->HasCaught();
//   }
  return false;
}

bool JSHTryCatch::CanContinue() {
//   if (try_catch_) {
//     return try_catch_->CanContinue();
//   }
  return true;
}

bool JSHTryCatch::HasTerminated() {
//   if (try_catch_) {
//     return try_catch_->HasTerminated();
//   }
  return false;
}

bool JSHTryCatch::IsVerbose() {
//   if (try_catch_) {
//     return try_catch_->IsVerbose();
//   }
  return false;
}

void JSHTryCatch::SetVerbose(bool verbose) {
//   if (try_catch_) {
//     try_catch_->SetVerbose(verbose);
//   }
}

std::shared_ptr<CtxValue> JSHTryCatch::Exception() {
//   if (try_catch_) {
//     FOOTSTONE_CHECK(ctx_);
//     auto v8_ctx = std::static_pointer_cast<V8Ctx>(ctx_);
//     v8::HandleScope handle_scope(v8_ctx->isolate_);
//     auto context = v8_ctx->context_persistent_.Get(v8_ctx->isolate_);
//     v8::Context::Scope context_scope(context);
//     auto exception = try_catch_->Exception();
//     return std::make_shared<V8CtxValue>(v8_ctx->isolate_, exception);
//   }
  return nullptr;
}

string_view JSHTryCatch::GetExceptionMessage() {
//   if (!try_catch_) {
//     return {};
//   }
//
//   auto v8_ctx = std::static_pointer_cast<V8Ctx>(ctx_);
//   auto isolate = v8_ctx->isolate_;
//   v8::HandleScope handle_scope(isolate);
//   auto context = v8_ctx->context_persistent_.Get(isolate);
//   v8::Context::Scope context_scope(context);
//
//   auto message = try_catch_->Message();
//   auto desc = V8VM::GetMessageDescription(isolate, context, message);
//   auto stack= V8VM::GetStackTrace(isolate, context, message->GetStackTrace());
//   return string_view("message: ") + desc + string_view(", stack: ") + stack;
  return {};
}

}
}
}
