/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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

#include "oh_location.h"
#include "locator_c_impl.h"

#include "locator.h"

std::mutex g_locationCallbackVectorMutex;
std::vector<OHOS::sptr<OHOS::Location::LocationInfoCallbackHost>> g_locationCallbackVector;
auto g_locatorProxy = OHOS::Location::Locator::GetInstance();
 
Location_ResultCode OH_Location_IsLocatingEnabled(bool *enabled)
{
    if (enabled == nullptr) {
        return LOCATION_INVALID_PARAM;
    }
    bool isEnabled = false;
    auto errCode = g_locatorProxy->IsLocationEnabledV9(isEnabled);
    if (errCode != OHOS::Location::ERRCODE_SUCCESS) {
        return LocationErrCodeToLocationResultCode(errCode);
    }
    *enabled = isEnabled;
    return LOCATION_SUCCESS;
}

Location_ResultCode OH_Location_StartLocating(const Location_RequestConfig* requestConfig,
    Location_InfoCallback callback)
{
    LBSLOGI(OHOS::Location::LOCATION_CAPI, "OH_Location_StartLocating ");
    if (requestConfig == nullptr || callback == nullptr) {
        return LOCATION_INVALID_PARAM;
    }
    if (GetLocationCallBack(callback) != nullptr) {
        LBSLOGE(OHOS::Location::LOCATION_CAPI, "%{public}s callback has registered.", __func__);
        return LOCATION_INVALID_PARAM;
    }
    auto locatorCallbackHost = OHOS::sptr<OHOS::Location::LocationInfoCallbackHost>(
            new (std::nothrow) OHOS::Location::LocationInfoCallbackHost());
    locatorCallbackHost->SetCallback(callback);
    auto locatorCallback = OHOS::sptr<OHOS::Location::ILocatorCallback>(locatorCallbackHost);
    auto requestConfigV9 = std::make_unique<OHOS::Location::RequestConfig>();
    if (requestConfig->scenario_ == OHOS::Location::SCENE_UNSET) {
        requestConfigV9->SetScenario(LOCATION_USE_SCENE_DAILY_LIFE_SERVICE);
    } else {
        requestConfigV9->SetScenario(requestConfig->scenario_);
    }
    requestConfigV9->SetTimeInterval(requestConfig->timeInterval_);
    AddLocationCallBack(locatorCallbackHost);
    auto errCode = g_locatorProxy->StartLocatingV9(requestConfigV9, locatorCallback);
    if (errCode != OHOS::Location::ERRCODE_SUCCESS) {
        RemoveLocationCallBack(callback);
        return LocationErrCodeToLocationResultCode(errCode);
    }
    return LOCATION_SUCCESS;
}

Location_ResultCode OH_Location_StopLocating(Location_InfoCallback callback)
{
    LBSLOGI(OHOS::Location::LOCATION_CAPI, "OH_Location_StopLocating");
    if (callback == nullptr) {
        return LOCATION_INVALID_PARAM;
    }
    auto locatorCallbackHost = GetLocationCallBack(callback);
    if (locatorCallbackHost == nullptr) {
        LBSLOGE(OHOS::Location::LOCATION_CAPI, "%{public}s locatorCallbackHost is nullptr.", __func__);
        return LOCATION_INVALID_PARAM;
    }
    auto locatorCallback = OHOS::sptr<OHOS::Location::ILocatorCallback>(locatorCallbackHost);
    auto errCode = g_locatorProxy->StopLocatingV9(locatorCallback);
    if (errCode != OHOS::Location::ERRCODE_SUCCESS) {
        return LocationErrCodeToLocationResultCode(errCode);
    }
    RemoveLocationCallBack(callback);
    return LOCATION_SUCCESS;
}

Location_BasicInfo OH_LocationInfo_GetBasicInfo(Location_Info* location)
{
    Location_BasicInfo location_Basic;
    memset_s(&location_Basic, sizeof(Location_BasicInfo), 0, sizeof(Location_BasicInfo));
    if (location == nullptr) {
        return location_Basic;
    }
    location_Basic.latitude = location->latitude;
    location_Basic.longitude = location->longitude;
    location_Basic.altitude = location->altitude;
    location_Basic.accuracy = location->accuracy;
    location_Basic.speed = location->speed;
    location_Basic.direction = location->direction;
    location_Basic.timeForFix = location->timeForFix;
    location_Basic.timeSinceBoot = location->timeSinceBoot;
    location_Basic.altitudeAccuracy = location->altitudeAccuracy;
    location_Basic.speedAccuracy = location->speedAccuracy;
    location_Basic.directionAccuracy = location->directionAccuracy;
    location_Basic.uncertaintyOfTimeSinceBoot = location->uncertaintyOfTimeSinceBoot;
    location_Basic.locationSourceType = location->locationSourceType;
    return location_Basic;
}

Location_ResultCode OH_LocationInfo_GetAdditionalInfo(Location_Info* location, char* additionalInfo, uint32_t length)
{
    if (location == nullptr || additionalInfo == nullptr) {
        return LOCATION_INVALID_PARAM;
    }
    if (length < strlen(location->additions)) {
        return LOCATION_INVALID_PARAM;
    }
    memset_s(additionalInfo, length, 0, length);
    auto ret = strcpy_s(additionalInfo, length - 1, location->additions);
    if (ret != 0) {
        LBSLOGE(OHOS::Location::LOCATION_CAPI, "strcpy_s failed, ret: %{public}d", ret);
        // addition is empty, no need return
    }
    return LOCATION_SUCCESS;
}

Location_RequestConfig* OH_Location_CreateRequestConfig(void)
{
    return new (std::nothrow) Location_RequestConfig();
}

void OH_Location_DestroyRequestConfig(Location_RequestConfig* requestConfig)
{
    if (requestConfig == nullptr) {
        LBSLOGE(OHOS::Location::LOCATION_CAPI, "Parameter error.");
        return;
    }
    delete requestConfig;
    requestConfig = nullptr;
}

void OH_LocationRequestConfig_SetUseScene(Location_RequestConfig* requestConfig,
    Location_UseScene useScene)
{
    if (requestConfig == nullptr) {
        LBSLOGE(OHOS::Location::LOCATION_CAPI, "requestConfig is nullptr");
        return;
    }
    if (useScene > LOCATION_USE_SCENE_DAILY_LIFE_SERVICE ||
        useScene < LOCATION_USE_SCENE_NAVIGATION) {
        LBSLOGE(OHOS::Location::LOCATION_CAPI, "userScenario is invalid");
        return;
    }
    requestConfig->scenario_ = useScene;
}

void OH_LocationRequestConfig_SetPowerConsumptionScene(Location_RequestConfig* requestConfig,
    Location_PowerConsumptionScene powerConsumptionScene)
{
    if (requestConfig == nullptr) {
        LBSLOGE(OHOS::Location::LOCATION_CAPI, "requestConfig is nullptr");
        return;
    }
    if (powerConsumptionScene > LOCATION_NO_POWER_CONSUMPTION ||
        powerConsumptionScene < LOCATION_HIGH_POWER_CONSUMPTION) {
        LBSLOGE(OHOS::Location::LOCATION_CAPI, "powerConsumptionScene is invalid");
        return;
    }
    if (requestConfig->scenario_ == OHOS::Location::SCENE_UNSET ||
        (requestConfig->scenario_ >= LOCATION_HIGH_POWER_CONSUMPTION &&
        requestConfig->scenario_ <= LOCATION_NO_POWER_CONSUMPTION)) {
        requestConfig->scenario_ = powerConsumptionScene;
    }
    return;
}

void OH_LocationRequestConfig_SetInterval(Location_RequestConfig* requestConfig,
    int interval)
{
    if (requestConfig == nullptr) {
        LBSLOGE(OHOS::Location::LOCATION_CAPI, "requestConfig is nullptr");
        return;
    }
    if (interval < 1) {
        LBSLOGE(OHOS::Location::LOCATION_CAPI, "timeInterval is invalid");
        return;
    }

    requestConfig->timeInterval_ = interval;
    return;
}

void AddLocationCallBack(OHOS::sptr<OHOS::Location::LocationInfoCallbackHost>& callback)
{
    std::unique_lock<std::mutex> lock(g_locationCallbackVectorMutex);
    g_locationCallbackVector.push_back(callback);
    LBSLOGD(OHOS::Location::LOCATION_CAPI, "after AddLocationCallBack, callback size %{public}s",
        std::to_string(g_locationCallbackVector.size()).c_str());
}

void RemoveLocationCallBack(Location_InfoCallback callback)
{
    std::unique_lock<std::mutex> lock(g_locationCallbackVectorMutex);
    if (g_locationCallbackVector.size() <= 0) {
        LBSLOGE(OHOS::Location::LOCATION_CAPI, "locationcallbacksize <= 0");
        return;
    }
    size_t i = 0;
    for (; i < g_locationCallbackVector.size(); i++) {
        if (callback == g_locationCallbackVector[i]->GetCallback()) {
            break;
        }
    }
    if (i >= g_locationCallbackVector.size()) {
        LBSLOGD(OHOS::Location::LOCATION_CAPI, "location callback is not in vector");
        return;
    }
    g_locationCallbackVector.erase(g_locationCallbackVector.begin() + i);
    LBSLOGD(OHOS::Location::LOCATION_CAPI, "after RemoveLocationCallBack, callback size %{public}s",
        std::to_string(g_locationCallbackVector.size()).c_str());
}

OHOS::sptr<OHOS::Location::LocationInfoCallbackHost> GetLocationCallBack(Location_InfoCallback callback)
{
    std::unique_lock<std::mutex> lock(g_locationCallbackVectorMutex);
    if (g_locationCallbackVector.size() <= 0) {
        LBSLOGE(OHOS::Location::LOCATION_CAPI, "locationcallbacksize <= 0");
        return nullptr;
    }
    size_t i = 0;
    for (; i < g_locationCallbackVector.size(); i++) {
        if (callback == g_locationCallbackVector[i]->GetCallback()) {
            return g_locationCallbackVector[i];
        }
    }
    return nullptr;
}

Location_ResultCode LocationErrCodeToLocationResultCode(OHOS::Location::LocationErrCode errCode)
{
    switch (errCode) {
        case OHOS::Location::ERRCODE_SUCCESS:
            return LOCATION_SUCCESS;
        case OHOS::Location::ERRCODE_PERMISSION_DENIED:
            return LOCATION_PERMISSION_DENIED;
        case OHOS::Location::ERRCODE_INVALID_PARAM:
            return LOCATION_INVALID_PARAM;
        case OHOS::Location::ERRCODE_NOT_SUPPORTED:
            return LOCATION_NOT_SUPPORTED;
        case OHOS::Location::ERRCODE_SERVICE_UNAVAILABLE:
            return LOCATION_SERVICE_UNAVAILABLE;
        case OHOS::Location::ERRCODE_SWITCH_OFF:
            return LOCATION_SWITCH_OFF;
        default:
            return LOCATION_SERVICE_UNAVAILABLE;
    }
}