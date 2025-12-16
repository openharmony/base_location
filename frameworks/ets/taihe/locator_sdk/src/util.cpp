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
#include "util.h"
namespace OHOS {
namespace Location {
std::map<int, std::string> Util::GetErrorCodeMapTaihe()
{
    std::map<int, std::string> errorCodeMap = {
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
        {LocationErrCode::ERRCODE_SUCCESS, "SUCCESS."},
        {LocationErrCode::ERRCODE_PERMISSION_DENIED,
            "Permission verification failed. The application does not have the permission required to call the API."},
        {LocationErrCode::ERRCODE_SYSTEM_PERMISSION_DENIED,
            "Permission verification failed. A non-system application calls a system API."},
        {LocationErrCode::ERRCODE_INVALID_PARAM,
            "Parameter error. Possible causes:1.Mandatory parameters are left unspecified;" \
            "2.Incorrect parameter types;3. Parameter verification failed."},
        {LocationErrCode::ERRCODE_NOT_SUPPORTED,
            "Capability not supported." \
            "Failed to call function due to limited device capabilities."},
        {LocationErrCode::ERRCODE_SERVICE_UNAVAILABLE, "The location service is unavailable."},
        {LocationErrCode::ERRCODE_LOCATING_NETWORK_FAIL,
        "The network locating is failed because the network cannot be accessed."},
        {LocationErrCode::ERRCODE_LOCATING_ACC_FAIL,
        "The positioning result does not meet the precision requirement (maxAccuracy)" \
        " in the positioning request parameters. "},
        {LocationErrCode::ERRCODE_LOCATING_CACHE_FAIL, "The system does not have a cache locaiton."},
        {LocationErrCode::ERRCODE_SWITCH_OFF, "The location switch is off."},
        {LocationErrCode::ERRCODE_LOCATING_FAIL, "Failed to obtain the geographical location."},
        {LocationErrCode::ERRCODE_REVERSE_GEOCODING_FAIL, "Reverse geocoding query failed."},
        {LocationErrCode::ERRCODE_GEOCODING_FAIL, "Geocoding query failed."},
        {LocationErrCode::ERRCODE_COUNTRYCODE_FAIL, "Failed to query the area information."},
        {LocationErrCode::ERRCODE_GEOFENCE_FAIL, "Failed to operate the geofence."},
        {LocationErrCode::ERRCODE_NO_RESPONSE, "No response to the request."},
        {LocationErrCode::ERRCODE_GEOFENCE_EXCEED_MAXIMUM, "The number of geofences exceeds the maximum."},
        {LocationErrCode::ERRCODE_GEOFENCE_INCORRECT_ID, "Failed to delete a geofence due to an incorrect ID."},
        {LocationErrCode::ERRCODE_WIFI_IS_NOT_CONNECTED,
            "Failed to obtain the hotpot MAC address because the Wi-Fi is not connected."}
    };
    return errorCodeMap;
}

int Util::ConvertErrorCodeTaihe(int errorCode)
{
    if (errorCode == LocationErrCode::ERRCODE_LOCATING_NETWORK_FAIL ||
        errorCode == LocationErrCode::ERRCODE_LOCATING_CACHE_FAIL ||
        errorCode == LocationErrCode::ERRCODE_LOCATING_ACC_FAIL) {
        LBSLOGI(LOCATOR_STANDARD, "Convert ErrorCode: %{public}d to %{public}d",
            errorCode, LocationErrCode::ERRCODE_LOCATING_FAIL);
        return LocationErrCode::ERRCODE_LOCATING_FAIL;
    }
    return errorCode;
}

std::string Util::GetErrorMsgByCodeTaihe(int& code)
{
    static std::map<int, std::string> errorCodeMap = GetErrorCodeMapTaihe();
    auto iter = errorCodeMap.find(code);
    if (iter != errorCodeMap.end()) {
        code = ConvertErrorCodeTaihe(code);
        return iter->second;
    }
    return "undefined error.";
}

void Util::ThrowBussinessError(int code)
{
    std::string errMsg = GetErrorMsgByCodeTaihe(code);
    taihe::set_business_error(code, errMsg);
}

void Util::LocationToTaihe(::ohos::geoLocationManager::Location& location, std::unique_ptr<Location>& lastlocation)
{
    location.latitude = lastlocation->GetLatitude();
    location.longitude = lastlocation->GetLongitude();
    location.altitude = lastlocation->GetAltitude();
    location.accuracy = lastlocation->GetAccuracy();
    location.speed = lastlocation->GetSpeed();
    location.direction = lastlocation->GetDirection();
    location.timeStamp = lastlocation->GetTimeStamp();
    location.timeSinceBoot = lastlocation->GetTimeSinceBoot();
    location.additionSize = ::taihe::optional<int>(std::in_place_t{}, lastlocation->GetAdditionSize());
    if (lastlocation->GetIsSystemApp() != 0) {
        location.isFromMock = ::taihe::optional<bool>(std::in_place_t{}, lastlocation->GetIsFromMock());
    }
    location.altitudeAccuracy = ::taihe::optional<double>(std::in_place_t{}, lastlocation->GetAltitudeAccuracy());
    location.speedAccuracy = ::taihe::optional<double>(std::in_place_t{}, lastlocation->GetSpeedAccuracy());
    location.directionAccuracy = ::taihe::optional<double>(std::in_place_t{}, lastlocation->GetDirectionAccuracy());
    location.uncertaintyOfTimeSinceBoot = ::taihe::optional<long>(std::in_place_t{}, lastlocation->GetUncertaintyOfTimeSinceBoot());
    location.sourceType = ::taihe::optional<::ohos::geoLocationManager::LocationSourceType>(std::in_place_t{},
        static_cast<::ohos::geoLocationManager::LocationSourceType::key_t>(lastlocation->GetLocationSourceType()));
}

void Util::TaiheToLocation(::ohos::geoLocationManager::Location& location, std::shared_ptr<Location>& lastlocation)
{
    lastlocation->SetLatitude(location.latitude);
    lastlocation->SetLongitude(location.longitude);
    lastlocation->SetAltitude(location.altitude);
    lastlocation->SetAccuracy(location.accuracy);
    lastlocation->SetSpeed(location.speed);
    lastlocation->SetDirection(location.direction);
    lastlocation->SetTimeStamp(location.timeStamp);
    lastlocation->SetTimeSinceBoot(location.timeSinceBoot);
    if (location.altitudeAccuracy) {
        lastlocation->SetAltitudeAccuracy(*location.altitudeAccuracy);
    }
    if (location.speedAccuracy) {
        lastlocation->SetSpeedAccuracy(*location.speedAccuracy);
    }
    if (location.directionAccuracy) {
        lastlocation->SetDirectionAccuracy(*location.directionAccuracy);
    }
    if (location.uncertaintyOfTimeSinceBoot) {
        lastlocation->SetUncertaintyOfTimeSinceBoot(*location.uncertaintyOfTimeSinceBoot);
    }
    if (location.sourceType) {
        lastlocation->SetLocationSourceType(*location.sourceType);
    }
}

void Util::TaiheCurrentRequestObjToRequestConfig(
    ::taihe::optional_view<::ohos::geoLocationManager::CurrentRequest> request,
    std::unique_ptr<RequestConfig>& requestConfig)
{
    if (!request) {
        return;
    }
    if (request->get_tag() == ::ohos::geoLocationManager::CurrentRequest::tag_t::type_CurrentLocationRequest) {
        ::ohos::geoLocationManager::CurrentLocationRequest currentLocationRequest =
            request->get_type_CurrentLocationRequest_ref();
        if (currentLocationRequest.priority) {
            requestConfig->SetPriority(currentLocationRequest.priority.value());
        }
        if (currentLocationRequest.scenario) {
            requestConfig->SetScenario(currentLocationRequest.scenario.value());
        }
        if (currentLocationRequest.maxAccuracy) {
            requestConfig->SetMaxAccuracy(*currentLocationRequest.maxAccuracy);
        }
        if (currentLocationRequest.timeoutMs) {
            requestConfig->SetTimeOut(*currentLocationRequest.timeoutMs);
        }
    } else {
        ::ohos::geoLocationManager::SingleLocationRequest singleLocationRequest =
            request->get_type_SingleLocationRequest_ref();
        requestConfig->SetPriority(singleLocationRequest.locatingPriority);
        requestConfig->SetTimeOut(singleLocationRequest.locatingTimeoutMs);
    }
}

void Util::GeoAddressToTaihe(std::vector<::ohos::geoLocationManager::GeoAddress>& geoAddressList,
    std::list<std::shared_ptr<GeoAddress>> replyList)
{
    for (auto iter = replyList.begin(); iter != replyList.end(); ++iter) {
        auto geoAddress = *iter;
        uint32_t tokenId = OHOS::IPCSkeleton::GetCallingTokenID();
        uint64_t tokenIdEx = OHOS::IPCSkeleton::GetCallingFullTokenID();
        bool flag = false;
        if (PermissionManager::CheckSystemPermission(tokenId, tokenIdEx)) {
            flag = true;
        }
        geoAddress->SetIsSystemApp(flag);
        ::ohos::geoLocationManager::GeoAddress geoAddressTaihe = ::ohos::geoLocationManager::GeoAddress{};
        geoAddressTaihe.latitude = ::taihe::optional<double>(std::in_place_t{}, geoAddress->latitude_);
        geoAddressTaihe.longitude = ::taihe::optional<double>(std::in_place_t{}, geoAddress->longitude_);
        geoAddressTaihe.locale = ::taihe::optional<taihe::string>(std::in_place_t{}, geoAddress->locale_);
        geoAddressTaihe.placeName = ::taihe::optional<taihe::string>(std::in_place_t{}, geoAddress->placeName_);
        geoAddressTaihe.countryCode = ::taihe::optional<taihe::string>(std::in_place_t{}, geoAddress->countryCode_);
        geoAddressTaihe.countryName = ::taihe::optional<taihe::string>(std::in_place_t{}, geoAddress->countryName_);
        geoAddressTaihe.administrativeArea =
            ::taihe::optional<taihe::string>(std::in_place_t{}, geoAddress->administrativeArea_);
        geoAddressTaihe.subAdministrativeArea =
            ::taihe::optional<taihe::string>(std::in_place_t{}, geoAddress->subAdministrativeArea_);
        geoAddressTaihe.locality = ::taihe::optional<taihe::string>(std::in_place_t{}, geoAddress->locality_);
        geoAddressTaihe.subLocality = ::taihe::optional<taihe::string>(std::in_place_t{}, geoAddress->subLocality_);
        geoAddressTaihe.roadName = ::taihe::optional<taihe::string>(std::in_place_t{}, geoAddress->roadName_);
        geoAddressTaihe.subRoadName = ::taihe::optional<taihe::string>(std::in_place_t{}, geoAddress->subRoadName_);
        geoAddressTaihe.premises = ::taihe::optional<taihe::string>(std::in_place_t{}, geoAddress->premises_);
        geoAddressTaihe.postalCode = ::taihe::optional<taihe::string>(std::in_place_t{}, geoAddress->postalCode_);
        geoAddressTaihe.phoneNumber = ::taihe::optional<taihe::string>(std::in_place_t{}, geoAddress->phoneNumber_);
        geoAddressTaihe.addressUrl = ::taihe::optional<taihe::string>(std::in_place_t{}, geoAddress->addressUrl_);
        geoAddressTaihe.descriptionsSize = ::taihe::optional<int>(std::in_place_t{}, geoAddress->descriptionsSize_);
        std::vector<::taihe::string> descriptions;
        for (int i = 0; i < geoAddress->descriptionsSize_; i++) {
            descriptions.push_back(geoAddress->descriptions_[i]);
        }
        geoAddressTaihe.descriptions =
            ::taihe::optional<::taihe::array<::taihe::string>>(std::in_place_t{},
            ::taihe::array<::taihe::string>{taihe::copy_data_t{},
            descriptions.data(), descriptions.size()});
        if (geoAddress->GetIsSystemApp()) {
            geoAddressTaihe.isFromMock = ::taihe::optional<bool>(std::in_place_t{}, geoAddress->isFromMock_);
        }
        geoAddressList.push_back(geoAddressTaihe);
    }
}


void Util::TaiheToGeoAddress(::ohos::geoLocationManager::GeoAddress& geoAddressTaihe,
    std::shared_ptr<GeoAddress>& geoAddress)
{
    if (geoAddressTaihe.latitude) {
        geoAddress->latitude_ = *geoAddressTaihe.latitude;
    }
    if (geoAddressTaihe.longitude) {
        geoAddress->longitude_ = *geoAddressTaihe.longitude;
    }
    if (geoAddressTaihe.locale) {
        geoAddress->locale_ = *geoAddressTaihe.locale;
    }
    if (geoAddressTaihe.placeName) {
        geoAddress->placeName_ = *geoAddressTaihe.placeName;
    }
    if (geoAddressTaihe.countryCode) {
        geoAddress->countryCode_ = *geoAddressTaihe.countryCode;
    }
    if (geoAddressTaihe.countryName) {
        geoAddress->countryName_ = *geoAddressTaihe.countryName;
    }
    if (geoAddressTaihe.administrativeArea) {
        geoAddress->administrativeArea_ = *geoAddressTaihe.administrativeArea;
    }
    if (geoAddressTaihe.subAdministrativeArea) {
        geoAddress->subAdministrativeArea_ = *geoAddressTaihe.subAdministrativeArea;
    }
    if (geoAddressTaihe.locality) {
        geoAddress->locality_ = *geoAddressTaihe.locality;
    }
    if (geoAddressTaihe.subLocality) {
        geoAddress->subLocality_ = *geoAddressTaihe.subLocality;
    }
    if (geoAddressTaihe.roadName) {
        geoAddress->roadName_ = *geoAddressTaihe.roadName;
    }
    if (geoAddressTaihe.subRoadName) {
        geoAddress->subRoadName_ = *geoAddressTaihe.subRoadName;
    }
    if (geoAddressTaihe.premises) {
        geoAddress->premises_ = *geoAddressTaihe.premises;
    }
    if (geoAddressTaihe.postalCode) {
        geoAddress->postalCode_ = *geoAddressTaihe.postalCode;
    }
    if (geoAddressTaihe.phoneNumber) {
        geoAddress->phoneNumber_ = *geoAddressTaihe.phoneNumber;
    }
    if (geoAddressTaihe.addressUrl) {
        geoAddress->addressUrl_ = *geoAddressTaihe.addressUrl;
    }
    if (geoAddressTaihe.descriptionsSize) {
        geoAddress->descriptionsSize_ = *geoAddressTaihe.descriptionsSize;
    }
}

void Util::TaiheCurrentRequestObjToRequestConfig(::ohos::geoLocationManager::OnRequest const& request,
    std::unique_ptr<RequestConfig>& requestConfig)
{
    if (request.get_tag() == ::ohos::geoLocationManager::OnRequest::tag_t::type_LocationRequest) {
        ::ohos::geoLocationManager::LocationRequest locationRequest =
            request.get_type_LocationRequest_ref();
        if (locationRequest.priority) {
            requestConfig->SetPriority(locationRequest.priority.value());
        }
        if (locationRequest.scenario) {
            requestConfig->SetScenario(locationRequest.scenario.value());
        }
        if (locationRequest.timeInterval) {
            requestConfig->SetTimeInterval(*locationRequest.timeInterval);
        }
        if (locationRequest.distanceInterval) {
            requestConfig->SetDistanceInterval(*locationRequest.distanceInterval);
        }
        if (locationRequest.maxAccuracy) {
            requestConfig->SetMaxAccuracy(*locationRequest.maxAccuracy);
        }
    } else {
        ::ohos::geoLocationManager::ContinuousLocationRequest continuousLocationRequest =
            request.get_type_ContinuousLocationRequest_ref();
        int scenario;
        if (continuousLocationRequest.locationScenario.get_tag() ==
            ohos::geoLocationManager::ContinuousLocationRequestLocationScenario::tag_t::type_UserActivityScenario) {
            scenario = continuousLocationRequest.locationScenario.get_type_UserActivityScenario_ref().get_value();
        } else {
            scenario = continuousLocationRequest.locationScenario.get_type_PowerConsumptionScenario_ref().get_value();
        }
        requestConfig->SetScenario(scenario);
        requestConfig->SetTimeInterval(continuousLocationRequest.interval);
    }
}

void Util::LocatingRequiredDataToTaihe(
    std::vector<::ohos::geoLocationManager::LocatingRequiredData>& locatingRequiredDataList,
    const std::vector<std::shared_ptr<LocatingRequiredData>>& replyList)
{
    for (auto iter = replyList.begin(); iter != replyList.end(); ++iter) {
        auto locatingRequiredData = *iter;
        ::ohos::geoLocationManager::WifiScanInfo wifiScanInfoTaihe =
            ::ohos::geoLocationManager::WifiScanInfo{};
        wifiScanInfoTaihe.ssid = locatingRequiredData->GetWifiScanInfo()->GetSsid();
        wifiScanInfoTaihe.bssid = locatingRequiredData->GetWifiScanInfo()->GetBssid();
        wifiScanInfoTaihe.rssi = locatingRequiredData->GetWifiScanInfo()->GetRssi();
        wifiScanInfoTaihe.frequency = locatingRequiredData->GetWifiScanInfo()->GetFrequency();
        wifiScanInfoTaihe.timestamp = locatingRequiredData->GetWifiScanInfo()->GetTimestamp();
        locatingRequiredDataList.push_back({
            ::taihe::optional<::ohos::geoLocationManager::WifiScanInfo>{std::in_place_t{},
            wifiScanInfoTaihe},
            {}});
    }
}

void Util::SatelliteStatusInfoToTaihe(::ohos::geoLocationManager::SatelliteStatusInfo& satelliteStatusInfo,
    const std::unique_ptr<SatelliteStatus>& statusInfo)
{
    satelliteStatusInfo.satellitesNumber = statusInfo->GetSatellitesNumber();
    satelliteStatusInfo.satelliteIds =
        ::taihe::array<int>{taihe::copy_data_t{},
        statusInfo->GetSatelliteIds().data(), statusInfo->GetSatelliteIds().size()};
    satelliteStatusInfo.carrierToNoiseDensitys =
        ::taihe::array<double>{taihe::copy_data_t{},
        statusInfo->GetCarrierToNoiseDensitys().data(), statusInfo->GetCarrierToNoiseDensitys().size()};
    satelliteStatusInfo.altitudes =
        ::taihe::array<double>{taihe::copy_data_t{},
        statusInfo->GetAltitudes().data(), statusInfo->GetAltitudes().size()};
    satelliteStatusInfo.azimuths =
        ::taihe::array<double>{taihe::copy_data_t{},
        statusInfo->GetAzimuths().data(), statusInfo->GetAzimuths().size()};
    satelliteStatusInfo.carrierFrequencies =
        ::taihe::array<double>{taihe::copy_data_t{},
        statusInfo->GetCarrierFrequencies().data(), statusInfo->GetCarrierFrequencies().size()};
    std::vector<::ohos::geoLocationManager::SatelliteConstellationCategory> satelliteConstellationCategoryList;
    for (auto item : statusInfo->GetConstellationTypes()) {
        satelliteConstellationCategoryList.push_back(
            static_cast<::ohos::geoLocationManager::SatelliteConstellationCategory::key_t>(item));
    }
    satelliteStatusInfo.satelliteConstellation =
        ::taihe::optional<::taihe::array<::ohos::geoLocationManager::SatelliteConstellationCategory>>{
            std::in_place_t{},
            ::taihe::array<::ohos::geoLocationManager::SatelliteConstellationCategory>{
                taihe::copy_data_t{},
                satelliteConstellationCategoryList.data(),
                satelliteConstellationCategoryList.size()}
        };
    satelliteStatusInfo.satelliteAdditionalInfo =
        ::taihe::optional<::taihe::array<int>>{
            std::in_place_t{},
            ::taihe::array<int>{taihe::copy_data_t{},
            statusInfo->GetSatelliteAdditionalInfoList().data(),
            statusInfo->GetSatelliteAdditionalInfoList().size()}
        };
}

void Util::BluetoothScanResultToTaihe(::ohos::geoLocationManager::BluetoothScanResult& bluetoothScanResultTaihe,
    const std::unique_ptr<BluetoothScanResult>& bluetoothScanResult)
{
    bluetoothScanResultTaihe.deviceId = bluetoothScanResult->GetDeviceId();
    bluetoothScanResultTaihe.deviceName = bluetoothScanResult->GetDeviceName();
    bluetoothScanResultTaihe.rssi = bluetoothScanResult->GetRssi();
    bluetoothScanResultTaihe.connectable = bluetoothScanResult->GetConnectable();
}

void Util::TaiheToRevGeocodeMock(::taihe::array_view<::ohos::geoLocationManager::ReverseGeocodingMockInfo>& mockInfos,
    std::vector<std::shared_ptr<GeocodingMockInfo>>& mockInfo)
{
    // todo
}
}
}