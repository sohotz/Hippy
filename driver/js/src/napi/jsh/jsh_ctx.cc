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

#include "driver/napi/jsh/jsh_ctx.h"
#include <hilog/log.h>
#include <sys/stat.h>

#include "driver/base/js_value_wrapper.h"
#include "driver/napi/jsh/jsh_ctx_value.h"
#include "driver/napi/jsh/jsh_class_definition.h"
#include "driver/napi/jsh/jsh_try_catch.h"
#include "driver/napi/callback_info.h"
#include "driver/vm/jsh/jsh_vm.h"
#include "driver/vm/native_source_code.h"
#include "footstone/check.h"
#include "footstone/string_view.h"
#include "footstone/string_view_utils.h"

namespace hippy {
inline namespace driver {
inline namespace napi {

using string_view = footstone::string_view;
using StringViewUtils = footstone::StringViewUtils;
using JSHVM = hippy::vm::JSHVM;
using CallbackInfo = hippy::CallbackInfo;

// TODO(hot-js):
std::map<JSVM_Value, void*> sEmbedderExternalMap;

// TODO(hot-js):
void CheckPendingExeception(JSVM_Env env_, JSVM_Status status) {
  if (status == JSVM_PENDING_EXCEPTION) {
    JSVM_Value error;
    if (OH_JSVM_GetAndClearLastException((env_), &error) == JSVM_OK) {
      // 获取异常堆栈
      JSVM_Value stack;
      OH_JSVM_GetNamedProperty((env_), error, "stack", &stack);

      JSVM_Value message;
      OH_JSVM_GetNamedProperty((env_), error, "message", &message);

      char stackstr[256];
      OH_JSVM_GetValueStringUtf8(env_, stack, stackstr, 256, nullptr);
      OH_LOG_INFO(LOG_APP, "xxx hippy JSVM error stack: %{public}s", stackstr);

      char messagestr[256];
      OH_JSVM_GetValueStringUtf8(env_, message, messagestr, 256, nullptr);
      OH_LOG_INFO(LOG_APP, "xxx hippy JSVM error message: %{public}s", messagestr);
    }
  }
}

JSVM_Value InvokeJsCallback(JSVM_Env env, JSVM_CallbackInfo info) {
  size_t argc = 0;
  auto status = OH_JSVM_GetCbInfo(env, info, &argc, nullptr, nullptr, nullptr);
  FOOTSTONE_DCHECK(status == JSVM_OK);
  std::vector<JSVM_Value> argv;
  if (argc > 0) {
    argv.resize(argc);
  }
  JSVM_Value thisArg = nullptr;
  void *data = nullptr;
  status = OH_JSVM_GetCbInfo(env, info, &argc, &argv[0], &thisArg, &data);
  FOOTSTONE_DCHECK(status == JSVM_OK);
  
  CallbackInfo cb_info;

  void *scope_data = nullptr;
  status = OH_JSVM_GetInstanceData(env, &scope_data);
  FOOTSTONE_DCHECK(status == JSVM_OK);
  cb_info.SetSlot(scope_data);
  
  void *internal_data = nullptr;
  status = OH_JSVM_Unwrap(env, thisArg, &internal_data);
  if (status == JSVM_OK) {
    if (internal_data) {
      cb_info.SetData(internal_data);
    }
  }
  
  cb_info.SetReceiver(std::make_shared<JSHCtxValue>(env, thisArg));
  for (size_t i = 0; i < argc; i++) {
    cb_info.AddValue(std::make_shared<JSHCtxValue>(env, argv[i]));
  }

  auto function_wrapper = reinterpret_cast<FunctionWrapper*>(data);
  FOOTSTONE_CHECK(function_wrapper);
  auto js_cb = function_wrapper->callback;
  auto external_data = function_wrapper->data;
  
  js_cb(cb_info, external_data);
  
  auto exception = std::static_pointer_cast<JSHCtxValue>(cb_info.GetExceptionValue()->Get());
  if (exception) {
    // TODO(hot-js):
    
    JSVM_Value result = nullptr;
    OH_JSVM_GetUndefined(env, &result);
    return result;
  }

  auto ret_value = std::static_pointer_cast<JSHCtxValue>(cb_info.GetReturnValue()->Get());
  if (!ret_value) {
    JSVM_Value result = nullptr;
    OH_JSVM_GetUndefined(env, &result);
    return result;
  }

  return ret_value->GetValue();
}

JSVM_Value InvokeJsCallbackOnConstruct(JSVM_Env env, JSVM_CallbackInfo info) {
  size_t argc = 0;
  auto status = OH_JSVM_GetCbInfo(env, info, &argc, nullptr, nullptr, nullptr);
  FOOTSTONE_DCHECK(status == JSVM_OK);
  std::vector<JSVM_Value> argv;
  if (argc > 0) {
    argv.resize(argc);
  }
  JSVM_Value thisArg = nullptr;
  void *data = nullptr;
  status = OH_JSVM_GetCbInfo(env, info, &argc, &argv[0], &thisArg, &data);
  FOOTSTONE_DCHECK(status == JSVM_OK);

  CallbackInfo cb_info;

  void *scope_data = nullptr;
  status = OH_JSVM_GetInstanceData(env, &scope_data);
  FOOTSTONE_DCHECK(status == JSVM_OK);
  cb_info.SetSlot(scope_data);
  
  // TODO(hot-js):
  void *external = sEmbedderExternalMap[thisArg];
  auto new_instance_external = (void*)external;
  if (new_instance_external) {
    cb_info.SetData(new_instance_external);
  } else {
    cb_info.SetData((void*)1); // TODO(hot-js):
  }
  
  cb_info.SetReceiver(std::make_shared<JSHCtxValue>(env, thisArg));
  for (size_t i = 0; i < argc; i++) {
    cb_info.AddValue(std::make_shared<JSHCtxValue>(env, argv[i]));
  }

  auto function_wrapper = reinterpret_cast<FunctionWrapper*>(data);
  FOOTSTONE_CHECK(function_wrapper);
  auto js_cb = function_wrapper->callback;
  auto external_data = function_wrapper->data;
  
  js_cb(cb_info, external_data);

  auto exception = std::static_pointer_cast<JSHCtxValue>(cb_info.GetExceptionValue()->Get());
  if (exception) {
    // TODO(hot-js):
    
    JSVM_Value result = nullptr;
    OH_JSVM_GetUndefined(env, &result);
    return result;
  }

  auto ret_value = std::static_pointer_cast<JSHCtxValue>(cb_info.GetReturnValue()->Get());
  if (!ret_value) {
    JSVM_Value result = nullptr;
    OH_JSVM_GetUndefined(env, &result);
    return result;
  }
  
  status = OH_JSVM_Wrap(env, thisArg, cb_info.GetData(), nullptr, nullptr, nullptr);
  FOOTSTONE_DCHECK(status == JSVM_OK);
  
  return thisArg;
}

JSHCtx::JSHCtx(JSVM_VM vm) : vm_(vm) {
  auto status = OH_JSVM_CreateEnv(vm_, 0, nullptr, &env_);
  FOOTSTONE_DCHECK(status == JSVM_OK);
  status = OH_JSVM_OpenEnvScope(env_, &env_scope_);
  FOOTSTONE_DCHECK(status == JSVM_OK);
}

std::shared_ptr<CtxValue> JSHCtx::CreateTemplate(const std::unique_ptr<FunctionWrapper>& wrapper) {
  return nullptr;
}

std::shared_ptr<CtxValue> JSHCtx::CreateFunction(const std::unique_ptr<FunctionWrapper>& wrapper) {
  JSHHandleScope handleScope(env_);
  
  // TODO(hot-js):
  JSVM_CallbackStruct *param = new JSVM_CallbackStruct();
  param->data = wrapper.get();
  param->callback = InvokeJsCallback;

  JSVM_Value funcValue = nullptr;
  JSVM_Status status = OH_JSVM_CreateFunction(env_, nullptr, JSVM_AUTO_LENGTH, param, &funcValue);
  FOOTSTONE_DCHECK(status == JSVM_OK);
  return std::make_shared<JSHCtxValue>(env_, funcValue);
}

void JSHCtx::SetExternalData(void* address) {
  OH_JSVM_SetInstanceData(env_, address, nullptr, nullptr);
}

std::shared_ptr<ClassDefinition> JSHCtx::GetClassDefinition(const string_view& name) {
  FOOTSTONE_DCHECK(template_map_.find(name) != template_map_.end());
  return template_map_[name];
}

void JSHCtx::SetAlignedPointerInEmbedderData(int index, intptr_t address) {
  JSHHandleScope handleScope(env_);
}

std::string JSHCtx::GetSerializationBuffer(const std::shared_ptr<CtxValue>& value,
                                          std::string& reused_buffer) {
  JSHHandleScope handleScope(env_);
  return "";
}

std::shared_ptr<CtxValue> JSHCtx::GetGlobalObject() {
  JSHHandleScope handleScope(env_);
  
  JSVM_Value global = nullptr;
  JSVM_Status status = OH_JSVM_GetGlobal(env_, &global);
  FOOTSTONE_DCHECK(status == JSVM_OK);
  return std::make_shared<JSHCtxValue>(env_, global);
}

std::shared_ptr<CtxValue> JSHCtx::GetProperty(
    const std::shared_ptr<CtxValue>& object,
    const string_view& name) {
  FOOTSTONE_CHECK(object);

  JSHHandleScope handleScope(env_);

  auto key = JSHVM::CreateJSHString(env_, name);
  return GetProperty(object, key);
}

std::shared_ptr<CtxValue> JSHCtx::GetProperty(
    const std::shared_ptr<CtxValue>& object,
    std::shared_ptr<CtxValue> key) {
  FOOTSTONE_CHECK(object && key);
  
  JSHHandleScope handleScope(env_);

  auto jsh_object = std::static_pointer_cast<JSHCtxValue>(object);
  auto jsh_key = std::static_pointer_cast<JSHCtxValue>(key);

  JSVM_Value result = nullptr;
  auto status = OH_JSVM_GetProperty(env_, jsh_object->GetValue(), jsh_key->GetValue(), &result);
  FOOTSTONE_DCHECK(status == JSVM_OK);
  return std::make_shared<JSHCtxValue>(env_, result);
}

std::shared_ptr<CtxValue> JSHCtx::RunScript(const string_view& str_view,
                                           const string_view& file_name) {
  return RunScript(str_view, file_name, false, nullptr, true);
}

std::shared_ptr<CtxValue> JSHCtx::RunScript(const string_view& str_view,
                                           const string_view& file_name,
                                           bool is_use_code_cache,
                                           string_view* cache,
                                           bool is_copy) {
  FOOTSTONE_LOG(INFO) << "JSHCtx::RunScript file_name = " << file_name
                      << ", is_use_code_cache = " << is_use_code_cache
                      << ", cache = " << cache << ", is_copy = " << is_copy;
  JSHHandleScope handleScope(env_);
  
  auto source_value = JSHVM::CreateJSHString(env_, str_view);
  if (!source_value) {
    FOOTSTONE_DLOG(WARNING) << "jsh_source empty, file_name = " << file_name;
    return nullptr;
  }

  return InternalRunScript(source_value, file_name, is_use_code_cache, cache);
}

std::shared_ptr<CtxValue> JSHCtx::InternalRunScript(
    std::shared_ptr<CtxValue> &source_value,
    const string_view& file_name,
    bool is_use_code_cache,
    string_view* cache) {
  JSHHandleScope handleScope(env_);
  
  // TODO(hot-js): use code cache
  
  auto jsh_source_value = std::static_pointer_cast<JSHCtxValue>(source_value);
  
  JSVM_Script script = nullptr;
  auto status = OH_JSVM_CompileScript(env_, jsh_source_value->GetValue(), nullptr, 0, true, nullptr, &script);
  CheckPendingExeception(env_, status);
  FOOTSTONE_DCHECK(status == JSVM_OK);
  if (!script) {
    return nullptr;
  }
  
  JSVM_Value result = nullptr;
  status = OH_JSVM_RunScript(env_, script, &result);
  FOOTSTONE_DCHECK(status == JSVM_OK);
  if (!result) {
    return nullptr;
  }
  
  return std::make_shared<JSHCtxValue>(env_, result);
}

void JSHCtx::ThrowException(const std::shared_ptr<CtxValue>& exception) {
  JSHHandleScope handleScope(env_);

  // TODO(hot-js):
}

void JSHCtx::ThrowException(const string_view& exception) {
  ThrowException(CreateException(exception));
}

std::shared_ptr<CtxValue> JSHCtx::CallFunction(
    const std::shared_ptr<CtxValue>& function,
    const std::shared_ptr<CtxValue>& receiver,
    size_t argument_count,
    const std::shared_ptr<CtxValue> arguments[]) {
  if (!function) {
    FOOTSTONE_LOG(ERROR) << "function is nullptr";
    return nullptr;
  }
  
  JSHHandleScope handleScope(env_);

  auto ctx_value = std::static_pointer_cast<JSHCtxValue>(function);
  bool isFunction = false;
  OH_JSVM_IsFunction(env_, ctx_value->GetValue(), &isFunction);
  if (!isFunction) {
    FOOTSTONE_LOG(WARNING) << "CallFunction handle_value is not a function";
    return nullptr;
  }

  std::vector<JSVM_Value> args;
  if (argument_count > 0) {
    args.resize(argument_count);
  }
  for (size_t i = 0; i < argument_count; i++) {
    auto argument = std::static_pointer_cast<JSHCtxValue>(arguments[i]);
    if (argument) {
      args[i] = argument->GetValue();
    } else {
      FOOTSTONE_LOG(WARNING) << "CallFunction argument error, i = " << i;
      return nullptr;
    }
  }

  auto receiver_object = std::static_pointer_cast<JSHCtxValue>(receiver);
  JSVM_Value result = nullptr;
  auto status = OH_JSVM_CallFunction(env_, receiver_object->GetValue(), ctx_value->GetValue(), argument_count, &args[0], &result);
  CheckPendingExeception(env_, status);
  FOOTSTONE_DCHECK(status == JSVM_OK);
  
  if (!result) {
    FOOTSTONE_DLOG(INFO) << "maybe_result is empty";
    return nullptr;
  }
  return std::make_shared<JSHCtxValue>(env_, result);
}

std::shared_ptr<CtxValue> JSHCtx::CreateNumber(double number) {
  JSHHandleScope handleScope(env_);
  
  JSVM_Value value = nullptr;
  auto status = OH_JSVM_CreateDouble(env_, number, &value);
  FOOTSTONE_DCHECK(status == JSVM_OK);
  if (!value) {
    return nullptr;
  }
  return std::make_shared<JSHCtxValue>(env_, value);
}

std::shared_ptr<CtxValue> JSHCtx::CreateBoolean(bool b) {
  JSHHandleScope handleScope(env_);
  
  JSVM_Value value = nullptr;
  auto status = OH_JSVM_CreateInt32(env_, b ? 1 : 0, &value); // TODO(hot-js):
  FOOTSTONE_DCHECK(status == JSVM_OK);
  if (!value) {
    return nullptr;
  }
  return std::make_shared<JSHCtxValue>(env_, value);
}

std::shared_ptr<CtxValue> JSHCtx::CreateString(const string_view& str_view) {
  if (str_view.encoding() == string_view::Encoding::Unknown) {
    return nullptr;
  }
  
  JSHHandleScope handleScope(env_);
  
  auto value = JSHVM::CreateJSHString(env_, str_view);
  return value;
}

std::shared_ptr<CtxValue> JSHCtx::CreateUndefined() {
  JSHHandleScope handleScope(env_);
  
  JSVM_Value value = nullptr;
  auto status = OH_JSVM_GetUndefined(env_, &value);
  FOOTSTONE_DCHECK(status == JSVM_OK);
  if (!value) {
    return nullptr;
  }
  return std::make_shared<JSHCtxValue>(env_, value);
}

std::shared_ptr<CtxValue> JSHCtx::CreateNull() {
  JSHHandleScope handleScope(env_);

  JSVM_Value value = nullptr;
  auto status = OH_JSVM_GetNull(env_, &value);
  FOOTSTONE_DCHECK(status == JSVM_OK);
  if (!value) {
    return nullptr;
  }
  return std::make_shared<JSHCtxValue>(env_, value);
}

std::shared_ptr<CtxValue> JSHCtx::CreateObject(const std::unordered_map<
    string_view,
    std::shared_ptr<CtxValue>>& object) {
  std::unordered_map<std::shared_ptr<CtxValue>, std::shared_ptr<CtxValue>> obj;
  for (const auto& it: object) {
    auto key = CreateString(it.first);
    obj[key] = it.second;
  }
  return CreateObject(obj);
}

std::shared_ptr<CtxValue> JSHCtx::CreateObject(const std::unordered_map<
    std::shared_ptr<CtxValue>,
    std::shared_ptr<CtxValue>>& object) {
  JSHHandleScope handleScope(env_);
  
  JSVM_Value obj = nullptr;
  auto status = OH_JSVM_CreateObject(env_, &obj);
  FOOTSTONE_DCHECK(status == JSVM_OK);
  
  for (const auto& it: object) {
    auto key_ctx_value = std::static_pointer_cast<JSHCtxValue>(it.first);
    auto value_ctx_value = std::static_pointer_cast<JSHCtxValue>(it.second);
    
    status = OH_JSVM_SetProperty(env_, obj, key_ctx_value->GetValue(), value_ctx_value->GetValue());
    FOOTSTONE_DCHECK(status == JSVM_OK);
  }
  
  return std::make_shared<JSHCtxValue>(env_, obj);
}

std::shared_ptr<CtxValue> JSHCtx::CreateArray(
    size_t count,
    std::shared_ptr<CtxValue> value[]) {
  int array_size;
  if (!footstone::numeric_cast<size_t, int>(count, array_size)) {
    FOOTSTONE_LOG(ERROR) << "array length out of boundary";
    return nullptr;
  }

  JSHHandleScope handleScope(env_);
  
  JSVM_Value array = nullptr;
  auto status = OH_JSVM_CreateArrayWithLength(env_, (size_t)array_size, &array);
  FOOTSTONE_DCHECK(status == JSVM_OK);
  for (auto i = 0; i < array_size; i++) {
    std::shared_ptr<JSHCtxValue> ctx_value = std::static_pointer_cast<JSHCtxValue>(value[i]);
    if (ctx_value) {
      status = OH_JSVM_SetElement(env_, array, (uint32_t)i, ctx_value->GetValue());
      FOOTSTONE_DCHECK(status == JSVM_OK);
    } else {
      FOOTSTONE_LOG(ERROR) << "array item error";
      return nullptr;
    }
  }
  
  return std::make_shared<JSHCtxValue>(env_, array);
}

std::shared_ptr<CtxValue> JSHCtx::CreateMap(const std::map<
    std::shared_ptr<CtxValue>,
    std::shared_ptr<CtxValue>>& map) {
  JSHHandleScope handleScope(env_);
  
  JSVM_Value obj = nullptr;
  auto status = OH_JSVM_CreateObject(env_, &obj); // TODO(hot-js):
  FOOTSTONE_DCHECK(status == JSVM_OK);
  
  for (const auto& it: map) {
    auto key_ctx_value = std::static_pointer_cast<JSHCtxValue>(it.first);
    auto value_ctx_value = std::static_pointer_cast<JSHCtxValue>(it.second);
    
    status = OH_JSVM_SetProperty(env_, obj, key_ctx_value->GetValue(), value_ctx_value->GetValue());
    FOOTSTONE_DCHECK(status == JSVM_OK);
  }
  return std::make_shared<JSHCtxValue>(env_, obj);
}

std::shared_ptr<CtxValue> JSHCtx::CreateException(const string_view& msg) {
  FOOTSTONE_DLOG(INFO) << "JSHCtx::CreateException msg = " << msg;
  JSHHandleScope handleScope(env_);
  
  JSVM_Value code = nullptr;
  auto msg_value = JSHVM::CreateJSHString(env_, msg);
  auto jsh_msg_value = std::static_pointer_cast<JSHCtxValue>(msg_value);
  JSVM_Value error = nullptr;
  auto status = OH_JSVM_CreateError(env_, code, jsh_msg_value->GetValue(), &error);
  FOOTSTONE_DCHECK(status == JSVM_OK);
  if (!error) {
    FOOTSTONE_LOG(INFO) << "error is empty";
    return nullptr;
  }
  
  return std::make_shared<JSHCtxValue>(env_, error);
}

std::shared_ptr<CtxValue> JSHCtx::CreateByteBuffer(void* buffer, size_t length) {
  if (!buffer) {
    return nullptr;
  }

  JSHHandleScope handleScope(env_);
  
  JSVM_Value array_buffer = nullptr;
  auto status = OH_JSVM_CreateArraybuffer(env_, length, &buffer, &array_buffer);
  FOOTSTONE_DCHECK(status == JSVM_OK);
  if (!array_buffer) {
    FOOTSTONE_LOG(ERROR) << "array_buffer is empty";
    return nullptr;
  }

  return std::make_shared<JSHCtxValue>(env_, array_buffer);
}

bool JSHCtx::GetValueNumber(const std::shared_ptr<CtxValue>& value, double* result) {
  if (!value || !result) {
    return false;
  }
  JSHHandleScope handleScope(env_);
  auto ctx_value = std::static_pointer_cast<JSHCtxValue>(value);
  if (!ctx_value || !IsNumber(ctx_value)) {
    return false;
  }

  auto status = OH_JSVM_GetValueDouble(env_, ctx_value->GetValue(), result);
  FOOTSTONE_DCHECK(status == JSVM_OK);
  
  return true;
}

bool JSHCtx::GetValueNumber(const std::shared_ptr<CtxValue>& value, int32_t* result) {
  if (!value || !result) {
    return false;
  }
  JSHHandleScope handleScope(env_);
  std::shared_ptr<JSHCtxValue> ctx_value = std::static_pointer_cast<JSHCtxValue>(value);
  if (!ctx_value || !IsNumber(ctx_value)) {
    return false;
  }

  auto status = OH_JSVM_GetValueInt32(env_, ctx_value->GetValue(), result);
  FOOTSTONE_DCHECK(status == JSVM_OK);
  return true;
}

bool JSHCtx::GetValueBoolean(const std::shared_ptr<CtxValue>& value, bool* result) {
  if (!value || !result) {
    return false;
  }
  JSHHandleScope handleScope(env_);
  std::shared_ptr<JSHCtxValue> ctx_value = std::static_pointer_cast<JSHCtxValue>(value);
  if (!ctx_value || !IsBoolean(ctx_value)) {
    return false;
  }

  auto status = OH_JSVM_GetValueBool(env_, ctx_value->GetValue(), result);
  FOOTSTONE_DCHECK(status == JSVM_OK);
  return true;
}

bool JSHCtx::GetValueString(const std::shared_ptr<CtxValue>& value,
                           string_view* result) {
  if (!value || !result) {
    return false;
  }
  JSHHandleScope handleScope(env_);
  std::shared_ptr<JSHCtxValue> ctx_value = std::static_pointer_cast<JSHCtxValue>(value);
  if (!ctx_value || !IsString(ctx_value)) {
    return false;
  }
  
  *result = JSHVM::ToStringView(env_, ctx_value->GetValue());
  return true;
}

bool JSHCtx::GetValueJson(const std::shared_ptr<CtxValue>& value,
                         string_view* result) {
  if (!value || !result) {
    return false;
  }
  JSHHandleScope handleScope(env_);
  std::shared_ptr<JSHCtxValue> ctx_value = std::static_pointer_cast<JSHCtxValue>(value);
  if (!ctx_value || !IsObject(ctx_value)) {
    return false;
  }
  
  JSVM_Value jsonResult = nullptr;
  auto status = OH_JSVM_JsonStringify(env_, ctx_value->GetValue(), &jsonResult);
  FOOTSTONE_DCHECK(status == JSVM_OK);
  if (!jsonResult) {
    return false;
  }

  *result = JSHVM::ToStringView(env_, jsonResult);
  return true;
}

bool JSHCtx::GetEntriesFromObject(const std::shared_ptr<CtxValue>& value,
                                 std::unordered_map<std::shared_ptr<CtxValue>, std::shared_ptr<CtxValue>>& map) {
  JSHHandleScope handleScope(env_);
  auto ctx_value = std::static_pointer_cast<JSHCtxValue>(value);
  if (!ctx_value) {
    return false;
  }
  
  auto obj = ctx_value->GetValue();
  
  JSVM_Value propNames = nullptr;
  auto status = OH_JSVM_GetPropertyNames(env_, obj, &propNames);
  FOOTSTONE_DCHECK(status == JSVM_OK);

  bool isArray = false;
  status = OH_JSVM_IsArray(env_, propNames, &isArray);
  FOOTSTONE_DCHECK(status == JSVM_OK);
  if (!isArray) {
    return false;
  }

  uint32_t arrayLength = 0;
  status = OH_JSVM_GetArrayLength(env_, propNames, &arrayLength);
  FOOTSTONE_DCHECK(status == JSVM_OK);
  if (!arrayLength) {
    return true; // TODO(hot-js):
  }

  for (uint32_t i = 0; i < arrayLength; i++)
  {
    bool hasElement = false;
    status = OH_JSVM_HasElement(env_, propNames, i, &hasElement);
    FOOTSTONE_DCHECK(status == JSVM_OK);
    if (!hasElement) {
      continue;
    }

    JSVM_Value propName = nullptr;
    status = OH_JSVM_GetElement(env_, propNames, i, &propName);
    FOOTSTONE_DCHECK(status == JSVM_OK);

    bool hasProp = false;
    status = OH_JSVM_HasProperty(env_, obj, propName, &hasProp);
    FOOTSTONE_DCHECK(status == JSVM_OK);
    if (!hasProp) {
      continue;
    }

    JSVM_Value propValue = nullptr;
    status = OH_JSVM_GetProperty(env_, obj, propName, &propValue);
    FOOTSTONE_DCHECK(status == JSVM_OK);
    
    map[std::make_shared<JSHCtxValue>(env_, propName)] = std::make_shared<JSHCtxValue>(env_, propValue);
  }
  return true;
}

bool JSHCtx::GetEntriesFromMap(const std::shared_ptr<CtxValue>& value,
                              std::unordered_map<std::shared_ptr<CtxValue>, std::shared_ptr<CtxValue>>& map) {
  JSHHandleScope handleScope(env_);
  auto ctx_value = std::static_pointer_cast<JSHCtxValue>(value);
  if (!ctx_value) {
    return false;
  }
  auto js_value = ctx_value->GetValue();

  std::shared_ptr<CtxValue> map_key = nullptr;
  
  // TODO(hot-js):
  bool isJsArray = false;
  auto status = OH_JSVM_IsArray(env_, js_value, &isJsArray);
  FOOTSTONE_DCHECK(status == JSVM_OK);
  if (isJsArray) {
    uint32_t arrayLength = 0;
    status = OH_JSVM_GetArrayLength(env_, js_value, &arrayLength);
    FOOTSTONE_DCHECK(status == JSVM_OK);

    for (uint32_t i = 0; i < arrayLength; i++) {
      JSVM_Value element = nullptr;
      status = OH_JSVM_GetElement(env_, js_value, i, &element);
      FOOTSTONE_DCHECK(status == JSVM_OK);
      
      if (i % 2 == 0) {
        map_key = std::make_shared<JSHCtxValue>(env_, element);
      } else {
        map[map_key] = std::make_shared<JSHCtxValue>(env_, element);
      }
    }
  }
  
  bool isJsObj = false;
  status = OH_JSVM_IsObject(env_, js_value, &isJsObj);
  FOOTSTONE_DCHECK(status == JSVM_OK);
  if (isJsObj) {
    JSVM_Value propNames = nullptr;
    status = OH_JSVM_GetPropertyNames(env_, js_value, &propNames);
    FOOTSTONE_DCHECK(status == JSVM_OK);
  
    bool isArray = false;
    status = OH_JSVM_IsArray(env_, propNames, &isArray);
    FOOTSTONE_DCHECK(status == JSVM_OK);
    if (!isArray) {
      return false;
    }
  
    uint32_t arrayLength = 0;
    status = OH_JSVM_GetArrayLength(env_, propNames, &arrayLength);
    FOOTSTONE_DCHECK(status == JSVM_OK);
    if (!arrayLength) {
      return false;
    }
  
    for (uint32_t i = 0; i < arrayLength; i++)
    {
      bool hasElement = false;
      status = OH_JSVM_HasElement(env_, propNames, i, &hasElement);
      FOOTSTONE_DCHECK(status == JSVM_OK);
      if (!hasElement) {
        continue;
      }
  
      JSVM_Value propName = nullptr;
      status = OH_JSVM_GetElement(env_, propNames, i, &propName);
      FOOTSTONE_DCHECK(status == JSVM_OK);
  
      bool hasProp = false;
      status = OH_JSVM_HasProperty(env_, js_value, propName, &hasProp);
      FOOTSTONE_DCHECK(status == JSVM_OK);
      if (!hasProp) {
        continue;
      }
  
      JSVM_Value propValue = nullptr;
      status = OH_JSVM_GetProperty(env_, js_value, propName, &propValue);
      FOOTSTONE_DCHECK(status == JSVM_OK);
      
      map[std::make_shared<JSHCtxValue>(env_, propName)] = std::make_shared<JSHCtxValue>(env_, propValue);
    }
  }
  return true;
}

bool JSHCtx::IsMap(const std::shared_ptr<CtxValue>& value) {
  if (!value) {
    return false;
  }
  
  JSHHandleScope handleScope(env_);
  auto ctx_value = std::static_pointer_cast<JSHCtxValue>(value);
  
  // TODO(hot-js):
  bool result = false;
  auto status = OH_JSVM_IsObject(env_, ctx_value->GetValue(), &result);
  FOOTSTONE_DCHECK(status == JSVM_OK);
  return result;
}

bool JSHCtx::IsNull(const std::shared_ptr<CtxValue>& value) {
  if (!value) {
    return false;
  }

  JSHHandleScope handleScope(env_);
  auto ctx_value = std::static_pointer_cast<JSHCtxValue>(value);
  
  bool result = false;
  auto status = OH_JSVM_IsNull(env_, ctx_value->GetValue(), &result);
  FOOTSTONE_DCHECK(status == JSVM_OK);
  return result;
}

bool JSHCtx::IsUndefined(const std::shared_ptr<CtxValue>& value) {
  if (!value) {
    return false;
  }

  JSHHandleScope handleScope(env_);
  auto ctx_value = std::static_pointer_cast<JSHCtxValue>(value);
  
  bool result = false;
  auto status = OH_JSVM_IsUndefined(env_, ctx_value->GetValue(), &result);
  FOOTSTONE_DCHECK(status == JSVM_OK);
  return result;
}

bool JSHCtx::IsNullOrUndefined(const std::shared_ptr<CtxValue>& value) {
  if (!value) {
    return true;
  }

  JSHHandleScope handleScope(env_);
  auto ctx_value = std::static_pointer_cast<JSHCtxValue>(value);

  bool result = false;
  auto status = OH_JSVM_IsNullOrUndefined(env_, ctx_value->GetValue(), &result);
  FOOTSTONE_DCHECK(status == JSVM_OK);
  return result;
}

// Array Helpers

bool JSHCtx::IsArray(const std::shared_ptr<CtxValue>& value) {
  if (!value) {
    return false;
  }
  JSHHandleScope handleScope(env_);
  auto ctx_value = std::static_pointer_cast<JSHCtxValue>(value);

  bool result = false;
  auto status = OH_JSVM_IsArray(env_, ctx_value->GetValue(), &result);
  FOOTSTONE_DCHECK(status == JSVM_OK);
  return result;
}

uint32_t JSHCtx::GetArrayLength(const std::shared_ptr<CtxValue>& value) {
  if (value == nullptr) {
    return 0;
  }
  JSHHandleScope handleScope(env_);
  auto ctx_value = std::static_pointer_cast<JSHCtxValue>(value);
  
  bool isArray = false;
  auto status = OH_JSVM_IsArray(env_, ctx_value->GetValue(), &isArray);
  FOOTSTONE_DCHECK(status == JSVM_OK);
  if (isArray) {
    uint32_t arrayLength = 0;
    status = OH_JSVM_GetArrayLength(env_, ctx_value->GetValue(), &arrayLength);
    FOOTSTONE_DCHECK(status == JSVM_OK);
    return arrayLength;
  }

  return 0;
}

std::shared_ptr<CtxValue> JSHCtx::CopyArrayElement(
    const std::shared_ptr<CtxValue>& value,
    uint32_t index) {
  if (!value) {
    return nullptr;
  }
  JSHHandleScope handleScope(env_);
  auto ctx_value = std::static_pointer_cast<JSHCtxValue>(value);

  bool isArray = false;
  auto status = OH_JSVM_IsArray(env_, ctx_value->GetValue(), &isArray);
  FOOTSTONE_DCHECK(status == JSVM_OK);
  if (isArray) {
    uint32_t arrayLength = 0;
    status = OH_JSVM_GetArrayLength(env_, ctx_value->GetValue(), &arrayLength);
    FOOTSTONE_DCHECK(status == JSVM_OK);
    if (index >= arrayLength) {
      return nullptr;
    }
    JSVM_Value result = nullptr;
    status = OH_JSVM_GetElement(env_, ctx_value->GetValue(), index, &result);
    FOOTSTONE_DCHECK(status == JSVM_OK);
    if (!result) {
      return nullptr;
    }
    return std::make_shared<JSHCtxValue>(env_, result);
  }
  
  return nullptr;
}

// Map Helpers
size_t JSHCtx::GetMapLength(std::shared_ptr<CtxValue>& value) {
  if (value == nullptr) {
    return 0;
  }
  JSHHandleScope handleScope(env_);
  auto ctx_value = std::static_pointer_cast<JSHCtxValue>(value);
  auto js_value = ctx_value->GetValue();
  
  // TODO(hot-js):
  bool isJsObj = false;
  auto status = OH_JSVM_IsObject(env_, ctx_value->GetValue(), &isJsObj);
  FOOTSTONE_DCHECK(status == JSVM_OK);
  if (isJsObj) {
    JSVM_Value propNames = nullptr;
    status = OH_JSVM_GetPropertyNames(env_, js_value, &propNames);
    FOOTSTONE_DCHECK(status == JSVM_OK);
  
    bool isArray = false;
    status = OH_JSVM_IsArray(env_, propNames, &isArray);
    FOOTSTONE_DCHECK(status == JSVM_OK);
    if (!isArray) {
      return 0;
    }
  
    uint32_t arrayLength = 0;
    status = OH_JSVM_GetArrayLength(env_, propNames, &arrayLength);
    FOOTSTONE_DCHECK(status == JSVM_OK);
    return arrayLength;
  }

  return 0;
}

std::shared_ptr<CtxValue> JSHCtx::ConvertMapToArray(
    const std::shared_ptr<CtxValue>& value) {
  if (value == nullptr) {
    return nullptr;
  }
  JSHHandleScope handleScope(env_);
  // TODO(hot):
  return nullptr;
}

// Object Helpers

bool JSHCtx::HasNamedProperty(const std::shared_ptr<CtxValue>& value,
                             const string_view& name) {
  if (!value || StringViewUtils::IsEmpty(name)) {
    return false;
  }
  JSHHandleScope handleScope(env_);
  auto ctx_value = std::static_pointer_cast<JSHCtxValue>(value);

  // TODO(hot-js):
  bool isJsObj = false;
  auto status = OH_JSVM_IsObject(env_, ctx_value->GetValue(), &isJsObj);
  FOOTSTONE_DCHECK(status == JSVM_OK);
  if (isJsObj) {
    auto key_value = JSHVM::CreateJSHString(env_, name);
    auto jsh_key_value = std::static_pointer_cast<JSHCtxValue>(key_value);
    
    bool result = false;
    status = OH_JSVM_HasProperty(env_, ctx_value->GetValue(), jsh_key_value->GetValue(), &result);
    FOOTSTONE_DCHECK(status == JSVM_OK);
    return result;
  }
  
  return false;
}

std::shared_ptr<CtxValue> JSHCtx::CopyNamedProperty(
    const std::shared_ptr<CtxValue>& value,
    const string_view& name) {
  if (!value || StringViewUtils::IsEmpty(name)) {
    return nullptr;
  }
  JSHHandleScope handleScope(env_);
  auto ctx_value = std::static_pointer_cast<JSHCtxValue>(value);
  
  // TODO(hot-js):
  bool isJsObj = false;
  auto status = OH_JSVM_IsObject(env_, ctx_value->GetValue(), &isJsObj);
  FOOTSTONE_DCHECK(status == JSVM_OK);
  if (isJsObj) {
    auto key_value = JSHVM::CreateJSHString(env_, name);
    auto jsh_key_value = std::static_pointer_cast<JSHCtxValue>(key_value);
    
    JSVM_Value result = nullptr;
    status = OH_JSVM_GetProperty(env_, ctx_value->GetValue(), jsh_key_value->GetValue(), &result);
    FOOTSTONE_DCHECK(status == JSVM_OK);
    return std::make_shared<JSHCtxValue>(env_, result);
  }

  return nullptr;
}

std::shared_ptr<CtxValue> JSHCtx::GetPropertyNames(const std::shared_ptr<CtxValue>& value) {
  if (!value) {
    return nullptr;
  }
  JSHHandleScope handleScope(env_);
  auto ctx_value = std::static_pointer_cast<JSHCtxValue>(value);
  auto js_value = ctx_value->GetValue();
  
  bool isJsObj = false;
  auto status = OH_JSVM_IsObject(env_, js_value, &isJsObj);
  FOOTSTONE_DCHECK(status == JSVM_OK);
  if (!isJsObj) {
    return nullptr;  
  }
  
  JSVM_Value propNames = nullptr;
  status = OH_JSVM_GetPropertyNames(env_, js_value, &propNames);
  FOOTSTONE_DCHECK(status == JSVM_OK);
  
  return std::make_shared<JSHCtxValue>(env_, propNames);
}

std::shared_ptr<CtxValue> JSHCtx::GetOwnPropertyNames(const std::shared_ptr<CtxValue>& value) {
  if (!value) {
    return nullptr;
  }
  JSHHandleScope handleScope(env_);
  auto ctx_value = std::static_pointer_cast<JSHCtxValue>(value);
  auto js_value = ctx_value->GetValue();
  
  bool isJsObj = false;
  auto status = OH_JSVM_IsObject(env_, js_value, &isJsObj);
  FOOTSTONE_DCHECK(status == JSVM_OK);
  if (!isJsObj) {
    return nullptr;  
  }
  
  // TODO(hot-js):
  JSVM_Value propNames = nullptr;
  status = OH_JSVM_GetAllPropertyNames(env_, js_value, JSVM_KEY_OWN_ONLY,
                                static_cast<JSVM_KeyFilter>(JSVM_KEY_ENUMERABLE | JSVM_KEY_SKIP_SYMBOLS),
                                JSVM_KEY_NUMBERS_TO_STRINGS, &propNames);
  FOOTSTONE_DCHECK(status == JSVM_OK);
  
  return std::make_shared<JSHCtxValue>(env_, propNames);
}

bool JSHCtx::IsBoolean(const std::shared_ptr<CtxValue>& value) {
  if (!value) {
    return false;
  }
  auto ctx_value = std::static_pointer_cast<JSHCtxValue>(value);
  
  JSHHandleScope handleScope(env_);
  
  bool result = false;
  auto status = OH_JSVM_IsBoolean(env_, ctx_value->GetValue(), &result);
  FOOTSTONE_DCHECK(status == JSVM_OK);
  return result;
}

bool JSHCtx::IsNumber(const std::shared_ptr<CtxValue>& value) {
  if (!value) {
    return false;
  }
  auto ctx_value = std::static_pointer_cast<JSHCtxValue>(value);

  JSHHandleScope handleScope(env_);
  
  bool result = false;
  auto status = OH_JSVM_IsNumber(env_, ctx_value->GetValue(), &result);
  FOOTSTONE_DCHECK(status == JSVM_OK);
  return result;
}

bool JSHCtx::IsString(const std::shared_ptr<CtxValue>& value) {
  if (!value) {
    return false;
  }
  auto ctx_value = std::static_pointer_cast<JSHCtxValue>(value);
  
  JSHHandleScope handleScope(env_);
  
  bool result = false;
  auto status = OH_JSVM_IsString(env_, ctx_value->GetValue(), &result);
  FOOTSTONE_DCHECK(status == JSVM_OK);
  return result;
}

bool JSHCtx::IsFunction(const std::shared_ptr<CtxValue>& value) {
  if (!value) {
    return false;
  }
  JSHHandleScope handleScope(env_);
  
  std::shared_ptr<JSHCtxValue> ctx_value = std::static_pointer_cast<JSHCtxValue>(value);
  
  bool result = false;
  auto status = OH_JSVM_IsFunction(env_, ctx_value->GetValue(), &result);
  FOOTSTONE_DCHECK(status == JSVM_OK);
  return result;
}

bool JSHCtx::IsObject(const std::shared_ptr<CtxValue>& value) {
  if (!value) {
    return false;
  }

  JSHHandleScope handleScope(env_);
  
  std::shared_ptr<JSHCtxValue> ctx_value = std::static_pointer_cast<JSHCtxValue>(value);

  bool result = false;
  auto status = OH_JSVM_IsObject(env_, ctx_value->GetValue(), &result);
  FOOTSTONE_DCHECK(status == JSVM_OK);
  
  return result;
}

string_view JSHCtx::CopyFunctionName(const std::shared_ptr<CtxValue>& function) {
  if (!function) {
    return {};
  }

  JSHHandleScope handleScope(env_);
  
  auto ctx_value = std::static_pointer_cast<JSHCtxValue>(function);

  string_view result;
  
  bool isFunc = false;
  auto status = OH_JSVM_IsFunction(env_, ctx_value->GetValue(), &isFunc);
  FOOTSTONE_DCHECK(status == JSVM_OK);
  if (isFunc) {
    // TODO(hot-js):
  }

  return result;
}

bool JSHCtx::SetProperty(std::shared_ptr<CtxValue> object,
                        std::shared_ptr<CtxValue> key,
                        std::shared_ptr<CtxValue> value) {
  JSHHandleScope handleScope(env_);
  auto jsh_object = std::static_pointer_cast<JSHCtxValue>(object);
  auto jsh_key = std::static_pointer_cast<JSHCtxValue>(key);
  auto jsh_value = std::static_pointer_cast<JSHCtxValue>(value);

  auto status = OH_JSVM_SetProperty(env_, jsh_object->GetValue(), jsh_key->GetValue(), jsh_value->GetValue());
  FOOTSTONE_DCHECK(status == JSVM_OK);
  return true;
}

bool JSHCtx::SetProperty(std::shared_ptr<CtxValue> object,
                        std::shared_ptr<CtxValue> key,
                        std::shared_ptr<CtxValue> value,
                        const PropertyAttribute& attr) {
  if (!IsString(key)) {
    return false;
  }

  JSHHandleScope handleScope(env_);
  auto jsh_object = std::static_pointer_cast<JSHCtxValue>(object);
  auto jsh_key = std::static_pointer_cast<JSHCtxValue>(key);
  auto jsh_value = std::static_pointer_cast<JSHCtxValue>(value);

  // TODO(hot-js):
  auto status = OH_JSVM_SetProperty(env_, jsh_object->GetValue(), jsh_key->GetValue(), jsh_value->GetValue());
  FOOTSTONE_DCHECK(status == JSVM_OK);
  return true;
}

std::shared_ptr<CtxValue> JSHCtx::CreateObject() {
  JSHHandleScope handleScope(env_);
  JSVM_Value obj = nullptr;
  auto status = OH_JSVM_CreateObject(env_, &obj);
  FOOTSTONE_DCHECK(status == JSVM_OK);
  return std::make_shared<JSHCtxValue>(env_, obj);
}

std::shared_ptr<CtxValue> JSHCtx::NewInstance(const std::shared_ptr<CtxValue>& cls,
                                             int argc, std::shared_ptr<CtxValue> argv[],
                                             void* external) {
  JSHHandleScope handleScope(env_);
  auto jsh_cls = std::static_pointer_cast<JSHCtxValue>(cls);

  JSVM_Value instanceValue = nullptr;
  
  std::vector<JSVM_Value> jsh_argv;
  if (argc > 0) {
    jsh_argv.resize((size_t)argc);
  }
  for (auto i = 0; i < argc; ++i) {
    auto jsh_value = std::static_pointer_cast<JSHCtxValue>(argv[i]);
    jsh_argv[(size_t)i] = jsh_value->GetValue();
  }
  
  JSVM_Status status = OH_JSVM_NewInstance(env_, jsh_cls->GetValue(), (size_t)argc, &jsh_argv[0], &instanceValue);
  FOOTSTONE_DCHECK(status == JSVM_OK);
  
  // TODO(hot-js):
  if (external) {
    sEmbedderExternalMap[instanceValue] = external;
  }

  return std::make_shared<JSHCtxValue>(env_, instanceValue);
}

void* JSHCtx::GetObjectExternalData(const std::shared_ptr<CtxValue>& object) {
  JSHHandleScope handleScope(env_);
  // TODO(hot-js):
  return nullptr;
}

std::shared_ptr<CtxValue> JSHCtx::DefineProxy(const std::unique_ptr<FunctionWrapper>& constructor_wrapper) {
  JSHHandleScope handleScope(env_);
  // TODO(hot-js):
  return nullptr;
}

std::shared_ptr<CtxValue> JSHCtx::DefineClass(const string_view& name,
                                             const std::shared_ptr<ClassDefinition>& parent,
                                             const std::unique_ptr<FunctionWrapper>& constructor_wrapper,
                                             size_t property_count,
                                             std::shared_ptr<PropertyDescriptor> properties[]) {
  JSHHandleScope handleScope(env_);
  
  if (parent) {
    auto parent_template = std::static_pointer_cast<JSHClassDefinition>(parent);
    // TODO(hot-js):
  }
  
  // TODO(hot-js): new and delete
  JSVM_PropertyDescriptor *propParams = new JSVM_PropertyDescriptor[property_count]; // TODO(hot-js):
  JSVM_CallbackStruct *callbackParams = new JSVM_CallbackStruct[property_count];
  for (size_t i = 0; i < property_count; i++) {
    const auto &prop_desc = properties[i];
    auto &propParam = propParams[i];
    auto &callbackParam = callbackParams[i];
    auto prop_name = std::static_pointer_cast<JSHCtxValue>(prop_desc->name);
    propParam.utf8name = nullptr;
    propParam.name = prop_name->GetValue();
    propParam.method = nullptr;
    propParam.value = nullptr;
    propParam.attributes = JSVM_DEFAULT;
    if (prop_desc->getter || prop_desc->setter) {
      if (prop_desc->getter) {
        // TODO(hot-js):
        JSVM_CallbackStruct *callbackP = new JSVM_CallbackStruct();
        callbackP->data = prop_desc->getter.get();
        callbackP->callback = InvokeJsCallback;
        propParam.getter = callbackP;
      } else {
        propParam.getter = nullptr;
      }
      if (prop_desc->setter) {
        // TODO(hot-js):
        JSVM_CallbackStruct *callbackP = new JSVM_CallbackStruct();
        callbackP->data = prop_desc->setter.get();
        callbackP->callback = InvokeJsCallback;
        propParam.setter = callbackP;
      } else {
        propParam.setter = nullptr;
      }
    } else if (prop_desc->method) {
      callbackParam.data = prop_desc->method.get();
      callbackParam.callback = InvokeJsCallback;
      propParam.utf8name = nullptr;
      propParam.name = prop_name->GetValue();
      propParam.method = &callbackParam;
      propParam.getter = nullptr;
      propParam.setter = nullptr;
      propParam.value = nullptr;
      propParam.attributes = JSVM_DEFAULT;
    } else {
      auto prop_value = std::static_pointer_cast<JSHCtxValue>(prop_desc->value);
      propParam.utf8name = nullptr;
      propParam.name = prop_name->GetValue();
      propParam.method = nullptr;
      propParam.getter = nullptr;
      propParam.setter = nullptr;
      propParam.value = prop_value->GetValue();
      propParam.attributes = JSVM_DEFAULT;
    }
  }
  
  string_view utf8Name = StringViewUtils::ConvertEncoding(name, string_view::Encoding::Utf8);
  
  // TODO(hot-js):
  JSVM_CallbackStruct *constructorParam = new JSVM_CallbackStruct();
  constructorParam->data = constructor_wrapper.get();
  constructorParam->callback = InvokeJsCallbackOnConstruct;
  
  JSVM_Value theClass = nullptr;
  auto status = OH_JSVM_DefineClass(env_, (const char *)utf8Name.utf8_value().c_str(), JSVM_AUTO_LENGTH, constructorParam, property_count, propParams, &theClass);
  FOOTSTONE_DCHECK(status == JSVM_OK);
  FOOTSTONE_DCHECK(theClass);

  template_map_[name] = std::make_shared<JSHClassDefinition>(env_, theClass);

  return std::make_shared<JSHCtxValue>(env_, theClass);
}

bool JSHCtx::Equals(const std::shared_ptr<CtxValue>& lhs, const std::shared_ptr<CtxValue>& rhs) {
  FOOTSTONE_DCHECK(lhs != nullptr && rhs != nullptr);
  JSHHandleScope handleScope(env_);
  std::shared_ptr<JSHCtxValue> ctx_lhs = std::static_pointer_cast<JSHCtxValue>(lhs);
  std::shared_ptr<JSHCtxValue> ctx_rhs = std::static_pointer_cast<JSHCtxValue>(rhs);
  
  bool isEquals = false;
  auto status = OH_JSVM_Equals(env_, ctx_lhs->GetValue(), ctx_rhs->GetValue(), &isEquals);
  FOOTSTONE_DCHECK(status == JSVM_OK);
  return isEquals;
}

bool JSHCtx::IsByteBuffer(const std::shared_ptr<CtxValue>& value) {
  JSHHandleScope handleScope(env_);
  std::shared_ptr<JSHCtxValue> ctx_value = std::static_pointer_cast<JSHCtxValue>(value);
  if (!ctx_value) {
    return false;
  }
  
  bool result = false;
  auto status = OH_JSVM_IsArraybuffer(env_, ctx_value->GetValue(), &result);
  FOOTSTONE_DCHECK(status == JSVM_OK);
  return result;
}

bool JSHCtx::GetByteBuffer(const std::shared_ptr<CtxValue>& value,
                          void** out_data,
                          size_t& out_length,
                          uint32_t& out_type) {
  JSHHandleScope handleScope(env_);
  auto ctx_value = std::static_pointer_cast<JSHCtxValue>(value);
  if (!ctx_value) {
    return false;
  }

  bool isArrayBuffer = false;
  auto status = OH_JSVM_IsArraybuffer(env_, ctx_value->GetValue(), &isArrayBuffer);
  FOOTSTONE_DCHECK(status == JSVM_OK);
  if (!isArrayBuffer) {
    return false;
  }

  void *tmpArrayBufferPtr = nullptr;
  size_t arrayBufferLength = 0;
  status = OH_JSVM_GetArraybufferInfo(env_, ctx_value->GetValue(), &tmpArrayBufferPtr, &arrayBufferLength);
  FOOTSTONE_DCHECK(status == JSVM_OK);
  
  *out_data = tmpArrayBufferPtr;
  out_length = arrayBufferLength;
  
  return true;
}

void  JSHCtx::SetWeak(std::shared_ptr<CtxValue> value,
                     const std::unique_ptr<WeakCallbackWrapper>& wrapper) {
  JSHHandleScope handleScope(env_);
  auto ctx_value = std::static_pointer_cast<JSHCtxValue>(value);
  // TODO(hot-js):
}

}  // namespace napi
}  // namespace driver
}  // namespace hippy
