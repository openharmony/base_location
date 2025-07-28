/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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

#include "napi_util.h"
#include "securec.h"
#include "string_ex.h"
#include "common_utils.h"
#include "location_log.h"
#include "beacon_fence_napi.h"

namespace OHOS {
namespace Location {
static constexpr int MAX_BUF_LEN = 100;

bool ParseBeaconFenceRequest(
    const napi_env& env, const napi_value& value, std::shared_ptr<BeaconFenceRequest>& request)
{
    napi_valuetype valueType;
    NAPI_CALL_BASE(env, napi_typeof(env, value, &valueType), false);
    if (valueType != napi_object) {
        LBSLOGE(NAPI_UTILS, "Wrong argument type, value should be object");
        return false;
    }
    return GenBeaconFenceRequest(env, value, request);
}

bool GenBeaconFenceRequest(
    const napi_env& env, const napi_value& value, std::shared_ptr<BeaconFenceRequest>& beaconFenceRequest)
{
    if (beaconFenceRequest == nullptr) {
        LBSLOGE(NAPI_UTILS, "beaconFenceRequest == nullptr");
        return false;
    }
    napi_value beaconValue = GetNapiValueByKey(env, "beacon", value);
    if (beaconValue == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "parse beacon failed");
        return false;
    }
    std::shared_ptr<BeaconFence> beaconFence = std::make_shared<BeaconFence>();
    bool isValidParameter = JsObjToBeaconFence(env, beaconValue, beaconFence);
    if (!isValidParameter) {
        return false;
    }
    beaconFenceRequest->SetBeaconFence(beaconFence);
    std::string fenceExtensionAbilityName = "";
    int fenceExtensionAbilityNameRes =
        JsObjectToString(env, value, "fenceExtensionAbilityName", MAX_BUF_LEN, fenceExtensionAbilityName);
    if (fenceExtensionAbilityNameRes == SUCCESS) {
        beaconFenceRequest->SetFenceExtensionAbilityName(fenceExtensionAbilityName);
    }
    napi_value callbackNapiValue = nullptr;
    NAPI_CALL_BASE(env,
        napi_get_named_property(env, value, "transitionCallback", &callbackNapiValue), false);
    napi_valuetype napiValueType = napi_undefined;
    NAPI_CALL_BASE(env, napi_typeof(env, callbackNapiValue, &napiValueType), false);
    if (napiValueType == napi_undefined && fenceExtensionAbilityNameRes != SUCCESS) {
        LBSLOGE(NAPI_UTILS, "%{public}s, parse transitionCallback and fenceExtensionAbilityName failed", __func__);
        return false;
    }
    return true;
}

void JsObjToBeaconFenceTransitionCallback(const napi_env& env, const napi_value& object,
    sptr<LocationGnssGeofenceCallbackNapi> callbackHost)
{
    napi_ref handlerRef = nullptr;
    napi_value callbackNapiValue = nullptr;
    NAPI_CALL_RETURN_VOID(env,
        napi_get_named_property(env, object, "transitionCallback", &callbackNapiValue));
    NAPI_CALL_RETURN_VOID(env, napi_create_reference(env, callbackNapiValue, 1, &handlerRef));
    callbackHost->SetEnv(env);
    callbackHost->SetHandleCb(handlerRef);
}

bool JsObjToBeaconFence(const napi_env& env, const napi_value& object,
    const std::shared_ptr<BeaconFence>& beaconFence)
{
    if (JsObjectToString(env, object, "identifier", MAX_BUF_LEN, beaconFence->identifier_) != SUCCESS) {
        LBSLOGE(LOCATOR_STANDARD, "parse identifier failed");
        return false;
    }
    int value = 0;
    if (JsObjectToInt(env, object, "type", value) == SUCCESS) {
        beaconFence->SetBeaconFenceInfoType(static_cast<BeaconFenceInfoType>(value));
    } else {
        LBSLOGE(LOCATOR_STANDARD, "parse type failed");
        return false;
    }
    if (!GenBeaconManufactureDataRequest(env, object, beaconFence)) {
        LBSLOGE(LOCATOR_STANDARD, "parse ManufactureData failed");
        return false;
    }
    return true;
}

bool GenBeaconManufactureDataRequest(
    const napi_env& env, const napi_value& object, std::shared_ptr<BeaconFence> beaconFence)
{
    napi_value manufactureDataValue = GetNapiValueByKey(env, "manufactureData", object);
    if (manufactureDataValue == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "parse manufactureDataValue failed");
        return false;
    }
    BeaconManufactureData beaconManufactureData = {0};
    if (JsObjectToInt(env, manufactureDataValue, "manufactureId", beaconManufactureData.manufactureId) != SUCCESS) {
        LBSLOGE(LOCATOR_STANDARD, "parse manufactureId failed");
        return false;
    }
    std::vector<uint8_t> manufactureDataVector {};
    if (!ParseArrayBufferParams(env, manufactureDataValue, "manufactureData", manufactureDataVector)) {
        LBSLOGE(LOCATOR_STANDARD, "parse manufactureData failed");
        return false;
    }
    beaconManufactureData.manufactureData = std::move(manufactureDataVector);
    std::vector<uint8_t> manufactureDataMaskVector {};
    if (!ParseArrayBufferParams(env, manufactureDataValue, "manufactureDataMask", manufactureDataMaskVector)) {
        LBSLOGE(LOCATOR_STANDARD, "parse manufactureDataMask failed");
        return false;
    }
    beaconManufactureData.manufactureDataMask = std::move(manufactureDataMaskVector);
    beaconFence->SetBeaconManufactureData(beaconManufactureData);
    return true;
}

bool ParseArrayBufferParams(napi_env env, napi_value object, const char *name, std::vector<uint8_t> &outParam)
{
    bool hasProperty = false;
    napi_has_named_property(env, object, name, &hasProperty);
    if (hasProperty) {
        napi_value property;
        napi_get_named_property(env, object, name, &property);
        bool isArrayBuffer = false;
        napi_is_arraybuffer(env, property, &isArrayBuffer);
        if (!isArrayBuffer) {
            LBSLOGE(NAPI_UTILS, "Wrong argument type,arraybuffer");
            return false;
        }
        uint8_t *data = nullptr;
        size_t size = 0;
        bool isSuccess = ParseArrayBuffer(env, &data, size, property);
        if (!isSuccess) {
            LBSLOGE(NAPI_UTILS, "ParseArrayBuffer faild.");
            return false;
        }
        outParam = std::vector<uint8_t>(data, data + size);
    }
    return hasProperty;
}

bool ParseArrayBuffer(napi_env env, uint8_t** data, size_t &size, napi_value args)
{
    napi_status status;
    napi_valuetype valuetype;
    napi_typeof(env, args, &valuetype);
    if (valuetype != napi_object) {
        LBSLOGE(NAPI_UTILS, "Wrong argument type(%{public}d). object expected.", valuetype);
        return false;
    }
    status = napi_get_arraybuffer_info(env, args, reinterpret_cast<void**>(data), &size);
    if (status != napi_ok) {
        LBSLOGE(NAPI_UTILS, "can not get arraybuffer, error is %{public}d", status);
        return false;
    }
    LBSLOGE(NAPI_UTILS, "arraybuffer size is %{public}zu", size);
    return true;
}
}  // namespace Location
}  // namespace OHOS
