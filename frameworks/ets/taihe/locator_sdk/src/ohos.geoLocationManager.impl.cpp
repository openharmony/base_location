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
 
#include "ohos.geoLocationManager.proj.hpp"
#include "ohos.geoLocationManager.impl.hpp"
#include "taihe/runtime.hpp"
#include "stdexcept"
#include "common_utils.h"
#include "locator.h"
#include "constant_definition.h"
#include "geo_address.h"
#include "permission_manager.h"
#include "ipc_skeleton.h"
#include "request_config.h"
#include "location_callback_taihe.h"
#include "location_switch_callback_taihe.h"
#include "country_code_callback_taihe.h"
#include "gnss_status_callback_taihe.h"
#include "locating_required_data_callback_taihe.h"
#include "nmea_message_callback_taihe.h"
#include "location_error_callback_taihe.h"
#include "bluetooth_scan_result_callback_taihe.h"
#include "util.h"

namespace {
// To be implemented.
using namespace OHOS::Location;
std::vector<OHOS::sptr<LocatorCallbackTaihe>> g_taiheLocationCallbackMap;
std::vector<OHOS::sptr<LocatingRequiredDataCallbackTaihe>> g_taiheLocatingRequiredDataCallbackMap;
std::vector<OHOS::sptr<CountryCodeCallbackTaihe>> g_taiheCountryCodeCallbackMap;
std::vector<OHOS::sptr<NmeaMessageCallbackTaihe>> g_taiheNmeaMessageCallbackMap;
std::vector<OHOS::sptr<GnssStatusCallbackTaihe>> g_taiheGnssStatusCallbackMap;
std::vector<OHOS::sptr<LocationSwitchCallbackTaihe>> g_taiheLocationSwitchCallbackMap;
std::vector<OHOS::sptr<BluetoothScanResultCallbackTaihe>> g_taiheBluetoothScanResultCallbackMap;
std::vector<OHOS::sptr<LocationErrorCallbackTaihe>> g_taiheLocationErrorCallbackMap;
static constexpr int LASTLOCATION_CACHED_TIME = 10 * 60;

int GetCurrentLocationType(std::unique_ptr<OHOS::Location::RequestConfig>& config)
{
    if (config->GetPriority() == OHOS::Location::LOCATION_PRIORITY_ACCURACY ||
        (config->GetScenario() == OHOS::Location::SCENE_UNSET &&
        config->GetPriority() == OHOS::Location::PRIORITY_ACCURACY) ||
        config->GetScenario() == OHOS::Location::SCENE_NAVIGATION ||
        config->GetScenario() == OHOS::Location::SCENE_TRAJECTORY_TRACKING ||
        config->GetScenario() == OHOS::Location::SCENE_CAR_HAILING) {
        return OHOS::Location::LOCATION_PRIORITY_ACCURACY;
    } else {
        return OHOS::Location::LOCATION_PRIORITY_LOCATING_SPEED;
    }
}

OHOS::sptr<LocatorCallbackTaihe> CreateSingleLocationCallbackHost()
{
    auto singleLocatorCallbackHost =
        OHOS::sptr<LocatorCallbackTaihe>(new LocatorCallbackTaihe());
    singleLocatorCallbackHost->SetFixNumber(1);
    return singleLocatorCallbackHost;
}

bool IsRequestConfigValid(std::unique_ptr<OHOS::Location::RequestConfig>& config)
{
    if (config == nullptr) {
        return false;
    }
    if ((config->GetScenario() > OHOS::Location::SCENE_NO_POWER ||
        config->GetScenario() < OHOS::Location::SCENE_UNSET) &&
        (config->GetScenario() > OHOS::Location::LOCATION_SCENE_RIDE ||
        config->GetScenario() < OHOS::Location::LOCATION_SCENE_NAVIGATION) &&
        (config->GetScenario() > OHOS::Location::LOCATION_SCENE_NO_POWER_CONSUMPTION ||
        config->GetScenario() < OHOS::Location::LOCATION_SCENE_HIGH_POWER_CONSUMPTION)) {
        return false;
    }
    if ((config->GetPriority() > OHOS::Location::PRIORITY_FAST_FIRST_FIX ||
        config->GetPriority() < OHOS::Location::PRIORITY_UNSET) &&
        (config->GetPriority() > OHOS::Location::LOCATION_PRIORITY_LOCATING_SPEED ||
        config->GetPriority() < OHOS::Location::LOCATION_PRIORITY_ACCURACY)) {
        return false;
    }
    if (config->GetTimeOut() < 1) {
        return false;
    }
    if (config->GetTimeInterval() < 0) {
        return false;
    }
    if (config->GetDistanceInterval() < 0) {
        return false;
    }
    if (config->GetMaxAccuracy() < 0) {
        return false;
    }
    return true;
}

bool IsLocationEnabled()
{
    bool isEnabled = false;
    LocationErrCode errorCode = Locator::GetInstance()->IsLocationEnabledV9(isEnabled);
    if (errorCode != ERRCODE_SUCCESS) {
        TH_THROW(std::runtime_error, "isLocationEnabled not implemented");
    }
    return isEnabled;
}

bool IsPoiServiceSupported()
{
    LBSLOGI(LOCATOR_STANDARD, "LocatorImpl::IsPoiServiceSupported() enter");
    bool poiServiceSupportState = false;
    LocationErrCode errorCode =
        Locator::GetInstance()->IsPoiServiceSupported(poiServiceSupportState)
    if (errorCode != ERRCODE_SUCCESS) {
        TH_THROW(std::runtime_error, "IsPoiServiceSupported not implemented");
    }
    return poiServiceSupportState;
}

::taihe::array<::ohos::geoLocationManager::GeoAddress> GetAddressesFromLocationSync(
    ::ohos::geoLocationManager::ReverseGeoCodeRequest const& request)
{
    bool isAvailable = false;
    LocationErrCode errorCode = Locator::GetInstance()->IsGeoServiceAvailableV9(isAvailable);
    if (errorCode != ERRCODE_SUCCESS) {
        return {};
    }
    if (!isAvailable) {
        return {};
    }
    OHOS::MessageParcel dataParcel;
    if (!dataParcel.WriteInterfaceToken(LocatorProxy::GetDescriptor())) {
        LBSLOGE(COUNTRY_CODE, "write interfaceToken fail!");
        return {};
    }
    if (request.locale) {
        dataParcel.WriteString16(OHOS::Str8ToStr16(std::string(*request.locale))); // locale
    } else {
        dataParcel.WriteString16(OHOS::Str8ToStr16("")); // locale
    }
    dataParcel.WriteDouble(request.latitude); // latitude
    dataParcel.WriteDouble(request.longitude); // longitude
    if (request.maxItems) {
        dataParcel.WriteInt32(request.maxItems); // maxItems
    } else {
        dataParcel.WriteInt32(1); // maxItems
    }
    dataParcel.WriteString16(OHOS::Str8ToStr16(CommonUtils::GenerateUuid())); // transId
    if (request.country) {
        dataParcel.WriteString16(OHOS::Str8ToStr16(std::string(*request.country))); // country
    } else {
        dataParcel.WriteString16(OHOS::Str8ToStr16("")); // locale
    }
    std::list<std::shared_ptr<GeoAddress>> replyList;
    errorCode = Locator::GetInstance()->GetAddressByCoordinateV9(dataParcel, replyList);
    if (replyList.empty() || errorCode != ERRCODE_SUCCESS) {
        return {};
    }
    std::vector<::ohos::geoLocationManager::GeoAddress> geoAddressList;
    Util::GeoAddressToTaihe(geoAddressList, replyList);
    return ::taihe::array<::ohos::geoLocationManager::GeoAddress>{taihe::copy_data_t{},
        geoAddressList.data(), geoAddressList.size()};
}

::ohos::geoLocationManager::Location GetCurrentLocationSync(
    ::taihe::optional_view<::ohos::geoLocationManager::CurrentRequest> request)
{
    ::taihe::map<::taihe::string, ::taihe::string> ret;
    ::ohos::geoLocationManager::Location result = {
        0.0, 0.0, 0.0, 0.0, 0.0, 0, 0.0, 0, {"", ""}, ret, 0, true, 0.0, 0.0, 0.0, 0,
        static_cast<::ohos::geoLocationManager::LocationSourceType::key_t>(1) };
    // request to capi request
    auto requestConfig = std::make_unique<OHOS::Location::RequestConfig>();
    Util::TaiheCurrentRequestObjToRequestConfig(request, requestConfig);
    // receive callback
    requestConfig->SetFixNumber(1);
    if (!IsRequestConfigValid(requestConfig)) {
        Util::ThrowBussinessError(LocationErrCode::ERRCODE_INVALID_PARAM);
    }
    auto singleLocatorCallbackHost = CreateSingleLocationCallbackHost();
    if (singleLocatorCallbackHost == nullptr) {
        Util::ThrowBussinessError(LocationErrCode::ERRCODE_SERVICE_UNAVAILABLE);
    }
    singleLocatorCallbackHost->SetLocationPriority(GetCurrentLocationType(requestConfig));

    auto callbackPtr = OHOS::sptr<OHOS::Location::ILocatorCallback>(singleLocatorCallbackHost);
    OHOS::Location::LocationErrCode errorCode = Locator::GetInstance()->StartLocatingV9(requestConfig, callbackPtr);
    if (errorCode != OHOS::Location::SUCCESS) {
        singleLocatorCallbackHost->SetCount(0);
    }
    if (requestConfig->GetTimeOut() > OHOS::Location::DEFAULT_TIMEOUT_30S) {
        singleLocatorCallbackHost->Wait(OHOS::Location::DEFAULT_TIMEOUT_30S);
        if (singleLocatorCallbackHost->GetSingleLocation() == nullptr) {
            singleLocatorCallbackHost->Wait(requestConfig->GetTimeOut() - OHOS::Location::DEFAULT_TIMEOUT_30S);
        }
    } else {
        singleLocatorCallbackHost->Wait(requestConfig->GetTimeOut());
    }
    Locator::GetInstance()->StopLocatingV9(callbackPtr);
    if (singleLocatorCallbackHost->GetCount() != 0 && singleLocatorCallbackHost->GetSingleLocation() == nullptr) {
        std::unique_ptr<OHOS::Location::Location> location = nullptr;
        Locator::GetInstance()->GetCachedLocationV9(location);
        int64_t curTime = OHOS::Location::CommonUtils::GetCurrentTimeStamp();
        if (location != nullptr &&
            (curTime - location->GetTimeStamp() / OHOS::Location::MILLI_PER_SEC) <= LASTLOCATION_CACHED_TIME) {
            singleLocatorCallbackHost->SetSingleLocation(location);
        }
    }
    if (singleLocatorCallbackHost != nullptr && singleLocatorCallbackHost->GetSingleLocation() != nullptr) {
        std::unique_ptr<OHOS::Location::Location> loc =
            std::make_unique<OHOS::Location::Location>(*singleLocatorCallbackHost->GetSingleLocation());
        Util::LocationToTaihe(result, loc);
    } else {
        Util::ThrowBussinessError(errorCode);
    }
    return result;
}

::ohos::geoLocationManager::Location GetCurrentLocationSyncNoRequest()
{
    ::taihe::map<::taihe::string, ::taihe::string> ret;
    ::ohos::geoLocationManager::Location result = {
        0.0, 0.0, 0.0, 0.0, 0.0, 0, 0.0, 0, {"", ""}, ret, 0, true, 0.0, 0.0, 0.0, 0,
        static_cast<::ohos::geoLocationManager::LocationSourceType::key_t>(1) };
    // request to capi request
    auto requestConfig = std::make_unique<OHOS::Location::RequestConfig>();
    // receive callback
    requestConfig->SetFixNumber(1);
    if (!IsRequestConfigValid(requestConfig)) {
        Util::ThrowBussinessError(LocationErrCode::ERRCODE_INVALID_PARAM);
    }
    auto singleLocatorCallbackHost = CreateSingleLocationCallbackHost();
    if (singleLocatorCallbackHost == nullptr) {
        Util::ThrowBussinessError(LocationErrCode::ERRCODE_SERVICE_UNAVAILABLE);
    }
    singleLocatorCallbackHost->SetLocationPriority(GetCurrentLocationType(requestConfig));

    auto callbackPtr = OHOS::sptr<OHOS::Location::ILocatorCallback>(singleLocatorCallbackHost);
    OHOS::Location::LocationErrCode errorCode = Locator::GetInstance()->StartLocatingV9(requestConfig, callbackPtr);
    if (errorCode != OHOS::Location::SUCCESS) {
        singleLocatorCallbackHost->SetCount(0);
    }
    if (requestConfig->GetTimeOut() > OHOS::Location::DEFAULT_TIMEOUT_30S) {
        singleLocatorCallbackHost->Wait(OHOS::Location::DEFAULT_TIMEOUT_30S);
        if (singleLocatorCallbackHost->GetSingleLocation() == nullptr) {
            singleLocatorCallbackHost->Wait(requestConfig->GetTimeOut() - OHOS::Location::DEFAULT_TIMEOUT_30S);
        }
    } else {
        singleLocatorCallbackHost->Wait(requestConfig->GetTimeOut());
    }
    Locator::GetInstance()->StopLocatingV9(callbackPtr);
    if (singleLocatorCallbackHost->GetCount() != 0 && singleLocatorCallbackHost->GetSingleLocation() == nullptr) {
        std::unique_ptr<OHOS::Location::Location> location = nullptr;
        Locator::GetInstance()->GetCachedLocationV9(location);
        int64_t curTime = OHOS::Location::CommonUtils::GetCurrentTimeStamp();
        if (location != nullptr &&
            (curTime - location->GetTimeStamp() / OHOS::Location::MILLI_PER_SEC) <= LASTLOCATION_CACHED_TIME) {
            singleLocatorCallbackHost->SetSingleLocation(location);
        }
    }
    if (singleLocatorCallbackHost != nullptr && singleLocatorCallbackHost->GetSingleLocation() != nullptr) {
        std::unique_ptr<OHOS::Location::Location> loc =
            std::make_unique<OHOS::Location::Location>(*singleLocatorCallbackHost->GetSingleLocation());
        Util::LocationToTaihe(result, loc);
    } else {
        Util::ThrowBussinessError(errorCode);
    }
    return result;
}

void EnableLocationSync()
{
    LocationErrCode errorCode = Locator::GetInstance()->EnableAbilityV9(true);
    if (errorCode != ERRCODE_SUCCESS) {
        Util::ThrowBussinessError(errorCode);
    }
}

void DisableLocation()
{
    LocationErrCode errorCode = Locator::GetInstance()->EnableAbilityV9(false);
    if (errorCode != ERRCODE_SUCCESS) {
        Util::ThrowBussinessError(errorCode);
    }
}

bool IsLocationPrivacyConfirmed(::ohos::geoLocationManager::LocationPrivacyType type)
{
    bool IsLocationPrivacyConfirmed = false;
    LocationErrCode errorCode = Locator::GetInstance()->IsLocationPrivacyConfirmedV9(type, IsLocationPrivacyConfirmed);
    if (errorCode != ERRCODE_SUCCESS) {
        Util::ThrowBussinessError(errorCode);
    }
    return IsLocationPrivacyConfirmed;
}

::ohos::geoLocationManager::Location GetLastLocation()
{
    ::taihe::map<::taihe::string, ::taihe::string> ret;
    ::ohos::geoLocationManager::Location location = {
        0.0, 0.0, 0.0, 0.0, 0.0, 0, 0.0, 0, {"", ""}, ret, 0, true, 0.0, 0.0, 0.0, 0,
        static_cast<::ohos::geoLocationManager::LocationSourceType::key_t>(1) };
    std::unique_ptr<Location> loc;
    LocationErrCode errorCode = Locator::GetInstance()->GetCachedLocationV9(loc);
    if (errorCode != ERRCODE_SUCCESS) {
        Util::ThrowBussinessError(errorCode);
    }
    if (loc != nullptr) {
        Util::LocationToTaihe(location, loc);
    }
    return location;
}

double GetDistanceBetweenLocations(::ohos::geoLocationManager::Location const& location1,
    ::ohos::geoLocationManager::Location const& location2)
{
    std::unique_ptr<Location> loc1;
    std::unique_ptr<Location> loc2;
    Util::TaiheToLocation(location1, loc1);
    Util::TaiheToLocation(location2, loc2);
    double distance;
    LocationErrCode errorCode =
        Locator::GetInstance()->GetDistanceBetweenLocations(loc1, loc2, distance);
    if (errorCode != ERRCODE_SUCCESS) {
        Util::ThrowBussinessError(errorCode);
    }
    return distance;
}

void OnCachedGnssLocationsChange(::ohos::geoLocationManager::CachedGnssLocationsRequest const& request,
    ::taihe::callback_view<void(::taihe::array_view<::ohos::geoLocationManager::Location>)> callback)
    {
    Util::ThrowBussinessError(LocationErrCode::ERRCODE_NOT_SUPPORTED);
}

void OffCachedGnssLocationsChange(
    ::taihe::optional_view<::taihe::callback<void(::taihe::array_view<::ohos::geoLocationManager::Location>)>>
    callback)
{
    Util::ThrowBussinessError(LocationErrCode::ERRCODE_NOT_SUPPORTED);
}

void OnLocationChange(::ohos::geoLocationManager::OnRequest const& request,
    ::taihe::callback_view<void(::ohos::geoLocationManager::Location const&)> callback)
{
    auto requestConfig = std::make_unique<OHOS::Location::RequestConfig>();
    Util::TaiheCurrentRequestObjToRequestConfig(request, requestConfig);
    auto locatorCallbackTaihe = OHOS::sptr<LocatorCallbackTaihe>(new LocatorCallbackTaihe());
    locatorCallbackTaihe->callback_ =
        ::taihe::optional<taihe::callback<void(::ohos::geoLocationManager::Location const&)>>{std::in_place_t{},
        callback};
    g_taiheLocationCallbackMap.push_back(locatorCallbackTaihe);
    auto locatorCallback = OHOS::sptr<ILocatorCallback>(locatorCallbackTaihe);
    LocationErrCode errorCode = Locator::GetInstance()->StartLocatingV9(requestConfig, locatorCallback);
    if (errorCode != ERRCODE_SUCCESS) {
        Util::ThrowBussinessError(errorCode);
    }
}

void OffLocationChange(
    ::taihe::optional_view<::taihe::callback<void(::ohos::geoLocationManager::Location const&)>> callback)
{
    for (std::uint32_t i = 0; i < g_taiheLocationCallbackMap.size(); i++) {
        auto locatorCallbackTaihe = g_taiheLocationCallbackMap[i];
        if (locatorCallbackTaihe->callback_ == callback) {
            auto locatorCallback = OHOS::sptr<ILocatorCallback>(locatorCallbackTaihe);
            LocationErrCode errorCode = Locator::GetInstance()->StopLocatingV9(locatorCallback);
            if (errorCode != ERRCODE_SUCCESS) {
                Util::ThrowBussinessError(errorCode);
            }
        }
        g_taiheLocationCallbackMap.erase(g_taiheLocationCallbackMap.begin() + i);
        break;
    }
}

void OnCountryCodeChange(::taihe::callback_view<void(::ohos::geoLocationManager::CountryCode const&)> callback)
{
    auto countryCodeCallbackTaihe = OHOS::sptr<CountryCodeCallbackTaihe>(new CountryCodeCallbackTaihe());
    countryCodeCallbackTaihe->callback_ =
        ::taihe::optional<taihe::callback<void(::ohos::geoLocationManager::CountryCode const&)>>{std::in_place_t{},
        callback};
    g_taiheCountryCodeCallbackMap.push_back(countryCodeCallbackTaihe);
    LocationErrCode errorCode =
        Locator::GetInstance()->RegisterCountryCodeCallbackV9(countryCodeCallbackTaihe->AsObject());
    if (errorCode != ERRCODE_SUCCESS) {
        Util::ThrowBussinessError(errorCode);
    }
}

void OffCountryCodeChange(
    ::taihe::optional_view<::taihe::callback<void(::ohos::geoLocationManager::CountryCode const&)>> callback)
{
    for (std::uint32_t i = 0; i < g_taiheCountryCodeCallbackMap.size(); i++) {
        auto countryCallbackTaihe = g_taiheCountryCodeCallbackMap[i];
        if (countryCallbackTaihe->callback_ == callback) {
            LocationErrCode errorCode =
                Locator::GetInstance()->UnregisterCountryCodeCallbackV9(countryCallbackTaihe->AsObject());
            if (errorCode != ERRCODE_SUCCESS) {
                Util::ThrowBussinessError(errorCode);
            }
        }
        g_taiheCountryCodeCallbackMap.erase(g_taiheCountryCodeCallbackMap.begin() + i);
        break;
    }
}

void OffNmeaMessage(::taihe::optional_view<::taihe::callback<void(::taihe::string_view)>> callback)
{
    for (std::uint32_t i = 0; i < g_taiheNmeaMessageCallbackMap.size(); i++) {
        auto nmeaCallbackTaihe = g_taiheNmeaMessageCallbackMap[i];
        if (nmeaCallbackTaihe->callback_ == callback) {
            LocationErrCode errorCode =
                Locator::GetInstance()->UnregisterNmeaMessageCallbackV9(nmeaCallbackTaihe->AsObject());
            if (errorCode != ERRCODE_SUCCESS) {
                Util::ThrowBussinessError(errorCode);
            }
        }
        g_taiheNmeaMessageCallbackMap.erase(g_taiheNmeaMessageCallbackMap.begin() + i);
        break;
    }
}

void OnNmeaMessage(::taihe::callback_view<void(::taihe::string_view)> callback)
{
    auto nmeaMessageCallbackTaihe = OHOS::sptr<NmeaMessageCallbackTaihe>(new NmeaMessageCallbackTaihe());
    nmeaMessageCallbackTaihe->callback_ =
        ::taihe::optional<::taihe::callback<void(::taihe::string_view)>>{std::in_place_t{}, callback};
    g_taiheNmeaMessageCallbackMap.push_back(nmeaMessageCallbackTaihe);
    LocationErrCode errorCode =
        Locator::GetInstance()->RegisterNmeaMessageCallbackV9(nmeaMessageCallbackTaihe->AsObject());
    if (errorCode != ERRCODE_SUCCESS) {
        Util::ThrowBussinessError(errorCode);
    }
}

void OffSatelliteStatusChange(
    ::taihe::optional_view<::taihe::callback<void(::ohos::geoLocationManager::SatelliteStatusInfo const&)>> callback)
{
    for (std::uint32_t i = 0; i < g_taiheGnssStatusCallbackMap.size(); i++) {
        auto satelliteStatusTaihe = g_taiheGnssStatusCallbackMap[i];
        if (satelliteStatusTaihe->callback_ == callback) {
            LocationErrCode errorCode =
                Locator::GetInstance()->UnregisterGnssStatusCallbackV9(satelliteStatusTaihe->AsObject());
            if (errorCode != ERRCODE_SUCCESS) {
                Util::ThrowBussinessError(errorCode);
            }
        }
        g_taiheGnssStatusCallbackMap.erase(g_taiheGnssStatusCallbackMap.begin() + i);
        break;
    }
}

void OnSatelliteStatusChange(
    ::taihe::callback_view<void(::ohos::geoLocationManager::SatelliteStatusInfo const&)> callback)
{
    auto gnssStatusCallbackTaihe = OHOS::sptr<GnssStatusCallbackTaihe>(new GnssStatusCallbackTaihe());
    gnssStatusCallbackTaihe->callback_ =
        ::taihe::optional<::taihe::callback<void(::ohos::geoLocationManager::SatelliteStatusInfo const&)>>{
            std::in_place_t{},
            callback};
    g_taiheGnssStatusCallbackMap.push_back(gnssStatusCallbackTaihe);
    LocationErrCode errorCode =
        Locator::GetInstance()->RegisterGnssStatusCallbackV9(gnssStatusCallbackTaihe->AsObject());
    if (errorCode != ERRCODE_SUCCESS) {
        Util::ThrowBussinessError(errorCode);
    }
}

void OffLocationEnabledChange(::taihe::optional_view<::taihe::callback<void(bool)>> callback)
{
    for (std::uint32_t i = 0; i < g_taiheLocationSwitchCallbackMap.size(); i++) {
        auto locationEnabledCallbackTaihe = g_taiheLocationSwitchCallbackMap[i];
        if (locationEnabledCallbackTaihe->callback_ == callback) {
            LocationErrCode errorCode =
                Locator::GetInstance()->UnregisterSwitchCallbackV9(locationEnabledCallbackTaihe->AsObject());
            if (errorCode != ERRCODE_SUCCESS) {
                Util::ThrowBussinessError(errorCode);
            }
        }
        g_taiheLocationSwitchCallbackMap.erase(g_taiheLocationSwitchCallbackMap.begin() + i);
        break;
    }
}

void OnLocationEnabledChange(::taihe::callback_view<void(bool)> callback)
{
    auto switchCallbackTaihe = OHOS::sptr<LocationSwitchCallbackTaihe>(new LocationSwitchCallbackTaihe());
    switchCallbackTaihe->callback_ = ::taihe::optional<::taihe::callback<void(bool)>>{std::in_place_t{}, callback};
    g_taiheLocationSwitchCallbackMap.push_back(switchCallbackTaihe);
    LocationErrCode errorCode = Locator::GetInstance()->RegisterSwitchCallbackV9(switchCallbackTaihe->AsObject());
    if (errorCode != ERRCODE_SUCCESS) {
        Util::ThrowBussinessError(errorCode);
    }
}

void OffBluetoothScanResultChange(
    ::taihe::optional_view<::taihe::callback<void(::ohos::geoLocationManager::BluetoothScanResult const&)>> callback)
{
    for (std::uint32_t i = 0; i < g_taiheBluetoothScanResultCallbackMap.size(); i++) {
        auto bluetoothScanResultCallbackTaihe = g_taiheBluetoothScanResultCallbackMap[i];
        if (bluetoothScanResultCallbackTaihe->callback_ == callback) {
            auto bluetoothScanResultCallback =
                OHOS::sptr<IBluetoothScanResultCallback>(bluetoothScanResultCallbackTaihe);
            LocationErrCode errorCode =
                Locator::GetInstance()->UnSubscribeBluetoothScanResultChange(bluetoothScanResultCallback);
            if (errorCode != ERRCODE_SUCCESS) {
                Util::ThrowBussinessError(errorCode);
            }
        }
        g_taiheBluetoothScanResultCallbackMap.erase(g_taiheBluetoothScanResultCallbackMap.begin() + i);
        break;
    }
}

void OnBluetoothScanResultChange(
    ::taihe::callback_view<void(::ohos::geoLocationManager::BluetoothScanResult const&)> callback)
{
    auto bluetoothScanResultCallbackTaihe =
        OHOS::sptr<BluetoothScanResultCallbackTaihe>(new BluetoothScanResultCallbackTaihe());
    bluetoothScanResultCallbackTaihe->callback_ =
        ::taihe::optional<::taihe::callback<void(::ohos::geoLocationManager::BluetoothScanResult const&)>>{
            std::in_place_t{}, callback};
    g_taiheBluetoothScanResultCallbackMap.push_back(bluetoothScanResultCallbackTaihe);
    auto bluetoothScanResultCallback =
        OHOS::sptr<IBluetoothScanResultCallback>(bluetoothScanResultCallbackTaihe);
    LocationErrCode errorCode =
        Locator::GetInstance()->SubscribeBluetoothScanResultChange(bluetoothScanResultCallback);
    if (errorCode != ERRCODE_SUCCESS) {
        Util::ThrowBussinessError(errorCode);
    }
}

void OffLocationError(
    ::taihe::optional_view<::taihe::callback<void(::ohos::geoLocationManager::LocationError)>> callback)
{
    for (std::uint32_t i = 0; i < g_taiheLocationErrorCallbackMap.size(); i++) {
        auto locationErrorCallbackTaihe = g_taiheLocationErrorCallbackMap[i];
        if (locationErrorCallbackTaihe->callback_ == callback) {
            auto locationErrorCallback = OHOS::sptr<ILocatorCallback>(locationErrorCallbackTaihe);
            LocationErrCode errorCode =
                Locator::GetInstance()->UnSubscribeLocationError(locationErrorCallback);
            if (errorCode != ERRCODE_SUCCESS) {
                Util::ThrowBussinessError(errorCode);
            }
        }
        g_taiheLocationErrorCallbackMap.erase(g_taiheLocationErrorCallbackMap.begin() + i);
        break;
    }
}

void OnLocationError(::taihe::callback_view<void(::ohos::geoLocationManager::LocationError)> callback)
{
    auto locationErrorCallbackTaihe =
        OHOS::sptr<LocationErrorCallbackTaihe>(new LocationErrorCallbackTaihe());
    locationErrorCallbackTaihe->callback_ =
        ::taihe::optional<::taihe::callback<void(::ohos::geoLocationManager::LocationError)>>{std::in_place_t{},
        callback};
    g_taiheLocationErrorCallbackMap.push_back(locationErrorCallbackTaihe);
    auto locationErrorCallback = OHOS::sptr<ILocatorCallback>(locationErrorCallbackTaihe);
    LocationErrCode errorCode = Locator::GetInstance()->SubscribeLocationError(locationErrorCallback);
    if (errorCode != ERRCODE_SUCCESS) {
        Util::ThrowBussinessError(errorCode);
    }
}

void OffLocationIconStatusChange(
    ::taihe::optional_view<::taihe::callback<void(::ohos::geoLocationManager::LocationIconStatus)>> callback)
{
    Util::ThrowBussinessError(LocationErrCode::ERRCODE_NOT_SUPPORTED);
}

void OnLocationIconStatusChange(
    ::taihe::callback_view<void(::ohos::geoLocationManager::LocationIconStatus)> callback)
{
    Util::ThrowBussinessError(LocationErrCode::ERRCODE_NOT_SUPPORTED);
}

void OnLocatingRequiredDataChange(
    ::ohos::geoLocationManager::LocatingRequiredDataConfig const& config,
    ::taihe::callback_view<void(::taihe::array_view<::ohos::geoLocationManager::LocatingRequiredData>)> callback)
{
    std::unique_ptr<LocatingRequiredDataConfig> dataConfig = std::make_unique<LocatingRequiredDataConfig>();
    dataConfig->SetType(config.type.get_value());
    dataConfig->SetNeedStartScan(config.needStartScan);
    if (config.scanInterval) {
        dataConfig->SetScanIntervalMs(*config.scanInterval);
    }
    if (config.scanTimeout) {
        dataConfig->SetScanTimeoutMs(*config.scanTimeout);
    }
    auto locatingRequiredDataCallbackTaihe =
        OHOS::sptr<LocatingRequiredDataCallbackTaihe>(new LocatingRequiredDataCallbackTaihe());
    locatingRequiredDataCallbackTaihe->callback_ =
        ::taihe::optional<::taihe::callback<void(
        ::taihe::array_view<::ohos::geoLocationManager::LocatingRequiredData>)>>{std::in_place_t{}, callback};
    g_taiheLocatingRequiredDataCallbackMap.push_back(locatingRequiredDataCallbackTaihe);
    auto locatingRequiredDataCallback =
        OHOS::sptr<ILocatingRequiredDataCallback>(locatingRequiredDataCallbackTaihe);
    LocationErrCode errorCode =
        Locator::GetInstance()->RegisterLocatingRequiredDataCallback(dataConfig, locatingRequiredDataCallback);
    if (errorCode != ERRCODE_SUCCESS) {
        Util::ThrowBussinessError(errorCode);
    }
}

void OffLocatingRequiredDataChange(
    ::taihe::optional_view<::taihe::callback<
    void(::taihe::array_view<::ohos::geoLocationManager::LocatingRequiredData>)>> callback)
{
    for (std::uint32_t i = 0; i < g_taiheLocatingRequiredDataCallbackMap.size(); i++) {
        auto locatingRequiredDataCallbackTaihe = g_taiheLocatingRequiredDataCallbackMap[i];
        if (locatingRequiredDataCallbackTaihe->callback_ == callback) {
            auto locatingRequiredDataCallback =
                OHOS::sptr<ILocatingRequiredDataCallback>(locatingRequiredDataCallbackTaihe);
            LocationErrCode errorCode =
                Locator::GetInstance()->UnRegisterLocatingRequiredDataCallback(locatingRequiredDataCallback);
            if (errorCode != ERRCODE_SUCCESS) {
                Util::ThrowBussinessError(errorCode);
            }
        }
        g_taiheLocatingRequiredDataCallbackMap.erase(g_taiheLocatingRequiredDataCallbackMap.begin() + i);
        break;
    }
}

void OnGnssFenceStatusChange(::ohos::geoLocationManager::GeofenceRequest const& request, uintptr_t want)
{
}

void OffGnssFenceStatusChange(::ohos::geoLocationManager::GeofenceRequest const& request, uintptr_t want)
{
}
}  // namespace

// Since these macros are auto-generate, lint will cause false positive.
// NOLINTBEGIN
TH_EXPORT_CPP_API_IsLocationEnabled(IsLocationEnabled);
TH_EXPORT_CPP_API_IsPoiServiceSupported(IsPoiServiceSupported);
TH_EXPORT_CPP_API_GetAddressesFromLocationSync(GetAddressesFromLocationSync);
TH_EXPORT_CPP_API_GetCurrentLocationSync(GetCurrentLocationSync);
TH_EXPORT_CPP_API_GetCurrentLocationSyncNoRequest(GetCurrentLocationSyncNoRequest);
TH_EXPORT_CPP_API_EnableLocationSync(EnableLocationSync);
TH_EXPORT_CPP_API_DisableLocation(DisableLocation);
TH_EXPORT_CPP_API_IsLocationPrivacyConfirmed(IsLocationPrivacyConfirmed);
TH_EXPORT_CPP_API_GetLastLocation(GetLastLocation);
TH_EXPORT_CPP_API_GetDistanceBetweenLocations(GetDistanceBetweenLocations);
TH_EXPORT_CPP_API_OnCachedGnssLocationsChange(OnCachedGnssLocationsChange);
TH_EXPORT_CPP_API_OffCachedGnssLocationsChange(OffCachedGnssLocationsChange);
TH_EXPORT_CPP_API_OnLocationChange(OnLocationChange);
TH_EXPORT_CPP_API_OffLocationChange(OffLocationChange);
TH_EXPORT_CPP_API_OnCountryCodeChange(OnCountryCodeChange);
TH_EXPORT_CPP_API_OffCountryCodeChange(OffCountryCodeChange);
TH_EXPORT_CPP_API_OffNmeaMessage(OffNmeaMessage);
TH_EXPORT_CPP_API_OnNmeaMessage(OnNmeaMessage);
TH_EXPORT_CPP_API_OffSatelliteStatusChange(OffSatelliteStatusChange);
TH_EXPORT_CPP_API_OnSatelliteStatusChange(OnSatelliteStatusChange);
TH_EXPORT_CPP_API_OffLocationEnabledChange(OffLocationEnabledChange);
TH_EXPORT_CPP_API_OnLocationEnabledChange(OnLocationEnabledChange);
TH_EXPORT_CPP_API_OffBluetoothScanResultChange(OffBluetoothScanResultChange);
TH_EXPORT_CPP_API_OnBluetoothScanResultChange(OnBluetoothScanResultChange);
TH_EXPORT_CPP_API_OffLocationError(OffLocationError);
TH_EXPORT_CPP_API_OnLocationError(OnLocationError);
TH_EXPORT_CPP_API_OffLocationIconStatusChange(OffLocationIconStatusChange);
TH_EXPORT_CPP_API_OnLocationIconStatusChange(OnLocationIconStatusChange);
TH_EXPORT_CPP_API_OnLocatingRequiredDataChange(OnLocatingRequiredDataChange);
TH_EXPORT_CPP_API_OffLocatingRequiredDataChange(OffLocatingRequiredDataChange);
TH_EXPORT_CPP_API_OnGnssFenceStatusChange(OnGnssFenceStatusChange);
TH_EXPORT_CPP_API_OffGnssFenceStatusChange(OffGnssFenceStatusChange);
// NOLINTEND
