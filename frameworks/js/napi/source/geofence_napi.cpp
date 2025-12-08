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
#ifdef NOTIFICATION_ENABLE
#include "notification_request.h"
#include "notification.h"
#include "notification_napi.h"
#endif
#include "geofence_definition.h"
#include "geofence_napi.h"

namespace OHOS {
namespace Location {
const int MAX_TRANSITION_ARRAY_SIZE = 3;

bool ParseGnssGeofenceRequest(
    const napi_env& env, const napi_value& value, std::shared_ptr<GeofenceRequest>& request)
{
    napi_valuetype valueType;
    NAPI_CALL_BASE(env, napi_typeof(env, value, &valueType), false);
    if (valueType != napi_object) {
        LBSLOGE(NAPI_UTILS, "Wrong argument type, value should be object");
        return false;
    }
    return GenGnssGeofenceRequest(env, value, request);
}

bool GenGnssGeofenceRequest(
    const napi_env& env, const napi_value& value, std::shared_ptr<GeofenceRequest>& geofenceRequest)
{
    if (geofenceRequest == nullptr) {
        LBSLOGE(NAPI_UTILS, "geofenceRequest == nullptr");
        return false;
    }
    bool isValidParameter = JsObjToGeoFenceRequest(env, value, geofenceRequest);
    if (!isValidParameter) {
        return false;
    }
    std::vector<GeofenceTransitionEvent> geofenceTransitionStatusList;
    JsObjToGeofenceTransitionEventList(env, value, geofenceTransitionStatusList);
    geofenceRequest->SetGeofenceTransitionEventList(geofenceTransitionStatusList);
#ifdef NOTIFICATION_ENABLE
    std::vector<std::shared_ptr<OHOS::Notification::NotificationRequest>> notificationRequestList;
    JsObjToNotificationRequestList(env, value, notificationRequestList);
    geofenceRequest->SetNotificationRequestList(notificationRequestList);
#endif
    int loiterTimeMs = 0;
    if (JsObjectToInt(env, value, "loiterTimeMs", loiterTimeMs) == SUCCESS) {
        if (loiterTimeMs < 0) {
            LBSLOGE(NAPI_UTILS, "loiterTimeMs is invaild");
            return false;
        }
        geofenceRequest->SetLoiterTimeMs(loiterTimeMs);
    }
    return true;
}

void JsObjToGeofenceTransitionCallback(const napi_env& env, const napi_value& object,
    sptr<LocationGnssGeofenceCallbackNapi> callbackHost)
{
    napi_ref handlerRef = nullptr;
    napi_value callbackNapiValue = nullptr;
    NAPI_CALL_RETURN_VOID(env,
        napi_get_named_property(env, object, "geofenceTransitionCallback", &callbackNapiValue));
    NAPI_CALL_RETURN_VOID(env, napi_create_reference(env, callbackNapiValue, 1, &handlerRef));
    callbackHost->SetEnv(env);
    callbackHost->SetHandleCb(handlerRef);
}

#ifdef NOTIFICATION_ENABLE
void JsObjToNotificationRequestList(const napi_env& env, const napi_value& object,
    std::vector<std::shared_ptr<OHOS::Notification::NotificationRequest>>& notificationRequestList)
{
    napi_value notificationRequest = GetArrayProperty(env, object, "notifications");
    GetNotificationRequestArray(env, notificationRequest, notificationRequestList);
}

void GetNotificationRequestArray(const napi_env& env, const napi_value& notificationRequestValue,
    std::vector<std::shared_ptr<OHOS::Notification::NotificationRequest>>& notificationRequestList)
{
    napi_valuetype valueType;
    NAPI_CALL_RETURN_VOID(env, napi_typeof(env, notificationRequestValue, &valueType));
    if (valueType != napi_object) {
        LBSLOGE(NAPI_UTILS, "Wrong argument type, value should be object");
        return;
    }
    uint32_t arrayLength = 0;
    NAPI_CALL_RETURN_VOID(env, napi_get_array_length(env, notificationRequestValue, &arrayLength));
    if (arrayLength == 0 || arrayLength > MAX_TRANSITION_ARRAY_SIZE) {
        LBSLOGE(NAPI_UTILS, "The array is empty or out of range.");
        return;
    }
    for (uint32_t i = 0; i < arrayLength; i++) {
        napi_value elementValue = nullptr;
        NAPI_CALL_RETURN_VOID(env, napi_get_element(env, notificationRequestValue, i, &elementValue));
        napi_valuetype napiType;
        NAPI_CALL_RETURN_VOID(env, napi_typeof(env, elementValue, &napiType));
        if (napiType != napi_object) {
            LBSLOGE(NAPI_UTILS, "Wrong argument type.");
            break;
        }
        std::shared_ptr<OHOS::Notification::NotificationRequest> notificationRequest =
            std::make_shared<OHOS::Notification::NotificationRequest>();
        GenNotificationRequest(env, elementValue, notificationRequest);
        notificationRequestList.push_back(notificationRequest);
    }
}

void GenNotificationRequest(const napi_env& env, const napi_value& elementValue,
    std::shared_ptr<OHOS::Notification::NotificationRequest>& notificationRequest)
{
    napi_valuetype elementValueType;
    NAPI_CALL_RETURN_VOID(env, napi_typeof(env, elementValue, &elementValueType));
    if (elementValueType != napi_object) {
        LBSLOGE(NAPI_UTILS, "Wrong argument type, value should be object");
        return;
    }
    // argv[0] : NotificationRequest
    NotificationNapi::GetNotificationRequest(env, elementValue, *notificationRequest);
}
#endif

void JsObjToGeofenceTransitionEventList(const napi_env& env, const napi_value& object,
    std::vector<GeofenceTransitionEvent>& geofenceTransitionStatusList)
{
    napi_value monitorTransitionEvents = GetArrayProperty(env, object, "monitorTransitionEvents");
    GetGeofenceTransitionEventArray(env, monitorTransitionEvents, geofenceTransitionStatusList);
}

void GetGeofenceTransitionEventArray(const napi_env& env, const napi_value& monitorTransitionEvents,
    std::vector<GeofenceTransitionEvent>& geofenceTransitionStatusList)
{
    napi_valuetype valueType;
    NAPI_CALL_RETURN_VOID(env, napi_typeof(env, monitorTransitionEvents, &valueType));
    if (valueType != napi_object) {
        LBSLOGE(NAPI_UTILS, "Wrong argument type, value should be object");
        return;
    }
    uint32_t arrayLength = 0;
    NAPI_CALL_RETURN_VOID(env, napi_get_array_length(env, monitorTransitionEvents, &arrayLength));
    if (arrayLength == 0 || arrayLength > MAX_TRANSITION_ARRAY_SIZE) {
        LBSLOGE(NAPI_UTILS, "The array is empty or out of range.");
        return;
    }
    for (uint32_t i = 0; i < arrayLength; i++) {
        napi_value elementValue = nullptr;
        NAPI_CALL_RETURN_VOID(env, napi_get_element(env, monitorTransitionEvents, i, &elementValue));
        napi_valuetype napiType;
        NAPI_CALL_RETURN_VOID(env, napi_typeof(env, elementValue, &napiType));
        if (napiType != napi_number) {
            LBSLOGE(NAPI_UTILS, "Wrong argument type.");
            break;
        }
        int geofenceTransitionStatus = -1;
        NAPI_CALL_RETURN_VOID(env, napi_get_value_int32(env, elementValue, &geofenceTransitionStatus));
        geofenceTransitionStatusList.push_back(static_cast<GeofenceTransitionEvent>(geofenceTransitionStatus));
    }
}

static bool CheckGeofenceParameter(const std::shared_ptr<Geofence>& fenceInfo)
{
    if (fenceInfo->GetLatitude() > MAX_LATITUDE || fenceInfo->GetLatitude() < MIN_LATITUDE) {
        LBSLOGE(LOCATOR_STANDARD, "latitude error.");
        return false;
    }
    if (fenceInfo->GetLongitude() > MAX_LONGITUDE || fenceInfo->GetLongitude() < MIN_LONGITUDE) {
        LBSLOGE(LOCATOR_STANDARD, "longitude error.");
        return false;
    }
    if (!(fenceInfo->GetRadius() > 0)) {
        LBSLOGE(LOCATOR_STANDARD, "radius error.");
        return false;
    }
    if (!(fenceInfo->GetExpiration() > 0)) {
        LBSLOGE(LOCATOR_STANDARD, "expiration error.");
        return false;
    }
    return true;
}

bool JsObjToGeoFenceRequest(const napi_env& env, const napi_value& object,
    const std::shared_ptr<GeofenceRequest>& request)
{
    int value = 0;
    if (JsObjectToInt(env, object, "scenario", value) == SUCCESS) {
        request->SetScenario(value);
    }
    napi_value geofenceValue = GetNapiValueByKey(env, "geofence", object);
    if (geofenceValue == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "parse geofence failed");
        return false;
    }
    std::shared_ptr<Geofence> geofence = std::make_shared<Geofence>();
    double latitude = 0.0;
    if (JsObjectToDouble(env, geofenceValue, "latitude", latitude) != SUCCESS) {
        LBSLOGE(LOCATOR_STANDARD, "parse latitude failed");
        return false;
    }
    geofence->SetLatitude(latitude);
    double longitude = 0.0;
    if (JsObjectToDouble(env, geofenceValue, "longitude", longitude) != SUCCESS) {
        LBSLOGE(LOCATOR_STANDARD, "parse longitude failed");
        return false;
    }
    geofence->SetLongitude(longitude);
    if (JsObjectToInt(env, geofenceValue, "coordinateSystemType", value) == SUCCESS) {
        geofence->SetCoordinateSystemType(static_cast<CoordinateSystemType>(value));
    } else {
        geofence->SetCoordinateSystemType(CoordinateSystemType::WGS84);
    }
    double radius = 0.0;
    if (JsObjectToDouble(env, geofenceValue, "radius", radius) != SUCCESS) {
        LBSLOGE(LOCATOR_STANDARD, "parse radius failed");
        return false;
    }
    geofence->SetRadius(radius);
    double expiration = 0.0;
    if (JsObjectToDouble(env, geofenceValue, "expiration", expiration) != SUCCESS) {
        LBSLOGE(LOCATOR_STANDARD, "parse expiration failed");
        return false;
    }
    geofence->SetExpiration(expiration);
    bool isValidParameter = CheckGeofenceParameter(geofence);
    if (!isValidParameter) {
        return false;
    }
    request->SetGeofence(geofence);
    return true;
}

void GeofenceTransitionToJs(const napi_env& env,
    const GeofenceTransition geofenceTransition, napi_value& result)
{
    SetValueInt32(env, "geofenceId", geofenceTransition.fenceId, result);
    SetValueInt32(env, "transitionEvent", static_cast<int>(geofenceTransition.event), result);
    if (geofenceTransition.beaconFence != nullptr) {
        napi_value beaconFenceJsObj = CreateBeaconFenceJsObj(env, geofenceTransition.beaconFence);
        SetValueBeacon(env, "beaconFence", beaconFenceJsObj, result);
    }
}

napi_value CreateBeaconFenceJsObj(const napi_env& env, const std::shared_ptr<BeaconFence>& beaconFence)
{
    napi_value beaconFenceObject = nullptr;
    NAPI_CALL(env, napi_create_object(env, &beaconFenceObject));
    SetValueUtf8String(env, "identifier",
        beaconFence->GetIdentifier().c_str(), beaconFenceObject);
    SetValueInt32(env, "beaconFenceInfoType",
        static_cast<int>(beaconFence->GetBeaconFenceInfoType()), beaconFenceObject);

    napi_value manufactureDataObject = nullptr;
    NAPI_CALL(env, napi_create_object(env, &manufactureDataObject));
    SetValueInt32(env, "manufactureId",
        beaconFence->GetBeaconManufactureData().manufactureId, manufactureDataObject);
    SetValueArrayBuffer(env, "manufactureData",
        beaconFence->GetBeaconManufactureData().manufactureData, manufactureDataObject);
    SetValueArrayBuffer(env, "manufactureDataMask",
        beaconFence->GetBeaconManufactureData().manufactureDataMask, manufactureDataObject);
    SetValueBeacon(env, "manufactureData",
        manufactureDataObject, beaconFenceObject);
    return beaconFenceObject;
}

napi_status SetValueBeacon(const napi_env& env, const char* fieldStr, napi_value& value, napi_value& result)
{
    NAPI_CALL_BASE(env, napi_set_named_property(env, result, fieldStr, value), napi_generic_failure);
    return napi_ok;
}
}  // namespace Location
}  // namespace OHOS
