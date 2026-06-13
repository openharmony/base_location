/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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

#include "bluetooth_search_callback_napi.h"

#include "constant_definition.h"
#include "location_log.h"
#include "napi_util.h"

namespace OHOS {
namespace Location {
const int MAX_DEVICE_ID_ARRAY_SIZE = 64;
const int MAX_DEVICE_ID_STR_LEN = 64;

bool JsObjToBluetoothSearchRequest(const napi_env& env, const napi_value& object,
    BluetoothSearchRequestParams& params)
{
    napi_valuetype valueType;
    NAPI_CALL_BASE(env, napi_typeof(env, object, &valueType), false);
    if (valueType != napi_object) {
        LBSLOGE(LOCATOR_STANDARD, "Wrong argument type, value should be object");
        return false;
    }
    params.rssiThreshold = DEFAULT_RSSI_THRESHOLD;
    params.deviceIdArray.clear();
    if (!ParseDeviceIdArrayFromJs(env, object, params)) {
        return false;
    }
    if (!ParseRssiThresholdFromJs(env, object, params)) {
        return false;
    }
    return true;
}

bool ParseDeviceIdArrayFromJs(const napi_env& env, const napi_value& object,
    BluetoothSearchRequestParams& params)
{
    bool hasDeviceIdArray = false;
    napi_has_named_property(env, object, "deviceIdArray", &hasDeviceIdArray);
    if (!hasDeviceIdArray) {
        return true;
    }
    napi_value deviceIdArrayValue;
    napi_get_named_property(env, object, "deviceIdArray", &deviceIdArrayValue);
    bool isArray = false;
    napi_is_array(env, deviceIdArrayValue, &isArray);
    if (!isArray) {
        return true;
    }
    uint32_t arrayLength = 0;
    napi_get_array_length(env, deviceIdArrayValue, &arrayLength);
    if (arrayLength > MAX_DEVICE_ID_ARRAY_SIZE) {
        LBSLOGE(LOCATOR_STANDARD, "deviceIdArray too large, max is %{public}d", MAX_DEVICE_ID_ARRAY_SIZE);
        return false;
    }
    for (uint32_t i = 0; i < arrayLength; ++i) {
        napi_value element;
        napi_get_element(env, deviceIdArrayValue, i, &element);
        napi_typeof(env, element, &valueType);
        if (valueType != napi_string) {
            LBSLOGE(LOCATOR_STANDARD, "deviceIdArray element must be string");
            return false;
        }
        char deviceIdStr[MAX_DEVICE_ID_STR_LEN] = {0};
        size_t deviceIdLen = 0;
        napi_get_value_string_utf8(env, element, deviceIdStr, sizeof(deviceIdStr), &deviceIdLen);
        params.deviceIdArray.push_back(std::string(deviceIdStr));
    }
    return true;
}

bool ParseRssiThresholdFromJs(const napi_env& env, const napi_value& object,
    BluetoothSearchRequestParams& params)
{
    bool hasRssiThreshold = false;
    napi_has_named_property(env, object, "rssiThreshold", &hasRssiThreshold);
    if (!hasRssiThreshold) {
        return true;
    }
    napi_valuetype valueType;
    napi_value rssiValue;
    napi_get_named_property(env, object, "rssiThreshold", &rssiValue);
    napi_typeof(env, rssiValue, &valueType);
    if (valueType != napi_number) {
        return true;
    }
    int32_t rssi = 0;
    napi_status status = napi_get_value_int32(env, rssiValue, &rssi);
    if (status == napi_ok) {
        params.rssiThreshold = rssi;
    }
    return true;
}
}  // namespace Location
}  // namespace OHOS