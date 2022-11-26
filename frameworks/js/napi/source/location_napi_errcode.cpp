/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
 */

#include "location_napi_errcode.h"
#include <map>
#include "common_utils.h"
#include "napi_util.h"

namespace OHOS {
namespace Location {
napi_value GetErrorValue(napi_env env, const int32_t errCode, const std::string errMsg)
{
    napi_value businessError = nullptr;
    napi_value eCode = nullptr;
    napi_value eMsg = nullptr;
    NAPI_CALL(env, napi_create_int32(env, errCode, &eCode));
    NAPI_CALL(env, napi_create_string_utf8(env, errMsg.c_str(),  errMsg.length(), &eMsg));
    NAPI_CALL(env, napi_create_object(env, &businessError));
    NAPI_CALL(env, napi_set_named_property(env, businessError, "code", eCode));
    NAPI_CALL(env, napi_set_named_property(env, businessError, "message", eMsg));
    return businessError;
}

void HandleSyncErrCode(const napi_env &env, int32_t errCode)
{
    LBSLOGI(LOCATOR_STANDARD, "HandleSyncErrCode, errCode = %{public}d", errCode);
    std::string errMsg = GetErrorMsgByCode(errCode);
    if (errMsg != "") {
        napi_throw_error(env, std::to_string(errCode).c_str(), errMsg.c_str());
    }
}

}  // namespace Location
}  // namespace OHOS
