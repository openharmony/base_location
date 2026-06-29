/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
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
const int MAX_DEVICE_ID_STR_LEN = 64;
const int MIN_RSSI_VALUE = -10000;
const int MAX_RSSI_VALUE = 10000;

static bool ParseDeviceIdArrayFromJs(const napi_env& env, const napi_value& object,
    BluetoothSearchRequestParams& params);
static bool ParseRssiThresholdFromJs(const napi_env& env, const napi_value& object,
    BluetoothSearchRequestParams& params);

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

static bool ParseDeviceIdArrayFromJs(const napi_env& env, const napi_value& object,
    BluetoothSearchRequestParams& params)
{
    bool hasDeviceIdArray = false;
    NAPI_CALL_BASE(env, napi_has_named_property(env, object, "deviceIdArray", &hasDeviceIdArray), false);
    if (!hasDeviceIdArray) {
        LBSLOGE(LOCATOR_STANDARD, "deviceIdArray is required");
        return false;
    }
    napi_value deviceIdArrayValue;
    NAPI_CALL_BASE(env, napi_get_named_property(env, object, "deviceIdArray", &deviceIdArrayValue), false);
    bool isArray = false;
    NAPI_CALL_BASE(env, napi_is_array(env, deviceIdArrayValue, &isArray), false);
    if (!isArray) {
        LBSLOGE(LOCATOR_STANDARD, "deviceIdArray must be array");
        return false;
    }
    uint32_t arrayLength = 0;
    NAPI_CALL_BASE(env, napi_get_array_length(env, deviceIdArrayValue, &arrayLength), false);
    if (arrayLength > MAX_BLUETOOTH_DEVICE_ID_ARRAY_SIZE) {
        LBSLOGE(LOCATOR_STANDARD, "deviceIdArray too large, max is %{public}d", MAX_BLUETOOTH_DEVICE_ID_ARRAY_SIZE);
        return false;
    }
    for (uint32_t i = 0; i < arrayLength; ++i) {
        napi_value element;
        napi_valuetype valueType;
        NAPI_CALL_BASE(env, napi_get_element(env, deviceIdArrayValue, i, &element), false);
        NAPI_CALL_BASE(env, napi_typeof(env, element, &valueType), false);
        if (valueType != napi_string) {
            LBSLOGE(LOCATOR_STANDARD, "deviceIdArray element must be string");
            return false;
        }
        char deviceIdStr[MAX_DEVICE_ID_STR_LEN] = {0};
        size_t deviceIdLen = 0;
        NAPI_CALL_BASE(env, napi_get_value_string_utf8(env, element, deviceIdStr,
            sizeof(deviceIdStr), &deviceIdLen), false);
        params.deviceIdArray.push_back(std::string(deviceIdStr));
    }
    return true;
}

static bool ParseRssiThresholdFromJs(const napi_env& env, const napi_value& object,
    BluetoothSearchRequestParams& params)
{
    bool hasRssiThreshold = false;
    NAPI_CALL_BASE(env, napi_has_named_property(env, object, "rssiThreshold", &hasRssiThreshold), false);
    if (!hasRssiThreshold) {
        return true;
    }
    napi_valuetype valueType;
    napi_value rssiValue;
    NAPI_CALL_BASE(env, napi_get_named_property(env, object, "rssiThreshold", &rssiValue), false);
    NAPI_CALL_BASE(env, napi_typeof(env, rssiValue, &valueType), false);
    if (valueType != napi_number) {
        LBSLOGE(LOCATOR_STANDARD, "rssiThreshold type error");
        return false;
    }
    int32_t rssi = 0;
    napi_status status = napi_get_value_int32(env, rssiValue, &rssi);
    if (status == napi_ok) {
        if (rssi < MIN_RSSI_VALUE || rssi > MAX_RSSI_VALUE) {
            LBSLOGE(LOCATOR_STANDARD, "rssiThreshold out of range [%{public}d, %{public}d]",
                MIN_RSSI_VALUE, MAX_RSSI_VALUE);
            return false;
        }
        params.rssiThreshold = rssi;
    }
    return true;
}
}  // namespace Location
}  // namespace OHOS