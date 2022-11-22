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

namespace OHOS {
namespace Location {

static std::map<int32_t, std::string> g_napiErrMsgMap {
    {SUCCESS, "SUCCESS"},
    {NOT_SUPPORTED, "NOT_SUPPORTED"},
    {INPUT_PARAMS_ERROR, "INPUT_PARAMS_ERROR"},
    {REVERSE_GEOCODE_ERROR, "REVERSE_GEOCODE_ERROR"},
    {GEOCODE_ERROR, "GEOCODE_ERROR"},
    {LOCATOR_ERROR, "LOCATOR_ERROR"},
    {LOCATION_SWITCH_ERROR, "LOCATION_SWITCH_ERROR"},
    {LAST_KNOWN_LOCATION_ERROR, "LAST_KNOWN_LOCATION_ERROR"},
    {LOCATION_REQUEST_TIMEOUT_ERROR, "LOCATION_REQUEST_TIMEOUT_ERROR"},
    {QUERY_COUNTRY_CODE_ERROR, "QUERY_COUNTRY_CODE_ERROR"},
    { LocationNapiErrCode::ERRCODE_PERMISSION_DENIED, "Permission denied." },
    { LocationNapiErrCode::ERRCODE_INVALID_PARAM, "Parameter error." },
    { LocationNapiErrCode::ERRCODE_NOT_SUPPORTED, "Capability not supported." },
    { LocationNapiErrCode::ERRCODE_SERVICE_UNAVAILABLE, "Location service is unavailable." },
    { LocationNapiErrCode::ERRCODE_SWITCH_OFF, "The location switch is off." },
    { LocationNapiErrCode::ERRCODE_LOCATING_FAIL, "Failed to obtain the geographical location." },
    { LocationNapiErrCode::ERRCODE_REVERSE_GEOCODING_FAIL, "Reverse geocoding query failed." },
    { LocationNapiErrCode::ERRCODE_GEOCODING_FAIL, "Geocoding query failed." },
    { LocationNapiErrCode::ERRCODE_COUNTRYCODE_FAIL, "Failed to query the area information." },
    { LocationNapiErrCode::ERRCODE_GEOFENCE_FAIL, "Failed to operate the geofence." },
    { LocationNapiErrCode::ERRCODE_NO_RESPONSE, "No response to the request." },
};

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

std::string GetNapiErrMsg(const napi_env &env, const int32_t errCode)
{
    auto iter = g_napiErrMsgMap.find(errCode);
    if (iter != g_napiErrMsgMap.end()) {
        std::string errMessage = "BussinessError ";
        errMessage.append(std::to_string(errCode)).append(": ").append(iter->second);
        return errMessage;
    }
    return "undefined error.";
}

void HandleSyncErrCode(const napi_env &env, int32_t errCode)
{
    LBSLOGI(LOCATOR_STANDARD, "HandleSyncErrCode, errCode = %{public}d", errCode);
    std::string errMsg = GetNapiErrMsg(env, errCode);
    if (errMsg != "") {
        napi_throw_error(env, std::to_string(errCode).c_str(), errMsg.c_str());
    }
}

}  // namespace Location
}  // namespace OHOS
