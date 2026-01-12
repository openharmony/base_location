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
#ifndef UTIL_H
#define UTIL_H

#include "ohos.geoLocationManager.proj.hpp"
#include "ohos.geoLocationManager.impl.hpp"
#include "taihe/runtime.hpp"
#include "stdexcept"
#include "location.h"
#include "common_utils.h"
#include "geo_address.h"
#include "request_config.h"
#include "ipc_skeleton.h"
#include "common_utils.h"
#include "permission_manager.h"
#include "locating_required_data.h"
#include "satellite_status.h"
#include "bluetooth_scan_result.h"
#include "geocoding_mock_info.h"
#include <list>
#include <vector>
namespace OHOS {
namespace Location {
class Util {
public:
    static std::map<int, std::string> GetErrorCodeMapTaihe();
    static int ConvertErrorCodeTaihe(int errorCode);
    static std::string GetErrorMsgByCodeTaihe(int& code);
    static void ThrowBussinessError(int code);
    static void LocationToTaihe(::ohos::geoLocationManager::Location& location,
        std::unique_ptr<Location>& lastlocation);
    static void TaiheToLocation(const ::ohos::geoLocationManager::Location& location,
        std::shared_ptr<Location>& lastlocation);
    static void TaiheCurrentRequestObjToRequestConfig(
        ::taihe::optional_view<::ohos::geoLocationManager::CurrentRequest> request,
        std::unique_ptr<RequestConfig>& requestConfig);
    static void GeoAddressToTaihe(std::vector<::ohos::geoLocationManager::GeoAddress>& geoAddressList,
        std::list<std::shared_ptr<GeoAddress>> replyList);
    static void TaiheCurrentRequestObjToRequestConfig(::ohos::geoLocationManager::OnRequest const& request,
        std::unique_ptr<RequestConfig>& requestConfig);
    static void LocatingRequiredDataToTaihe(
        std::vector<::ohos::geoLocationManager::LocatingRequiredData>& locatingRequiredDataList,
        const std::vector<std::shared_ptr<LocatingRequiredData>>& replyList);
    static void SatelliteStatusInfoToTaihe(::ohos::geoLocationManager::SatelliteStatusInfo& satelliteStatusInfo,
        const std::unique_ptr<SatelliteStatus>& statusInfo);
    static void BluetoothScanResultToTaihe(::ohos::geoLocationManager::BluetoothScanResult& bluetoothScanResultTaihe,
        const std::unique_ptr<BluetoothScanResult>& bluetoothScanResult);
    static void TaiheToRevGeocodeMock(
        ::taihe::array_view<::ohos::geoLocationManager::ReverseGeocodingMockInfo>& mockInfos,
        std::vector<std::shared_ptr<GeocodingMockInfo>>& mockInfo);
    static void TaiheToGeoAddress(::ohos::geoLocationManager::GeoAddress& geoAddressTaihe,
        std::shared_ptr<GeoAddress>& geoAddress);
    static bool NeedReportLastLocation(const std::unique_ptr<<OHOS::Location::RequestConfig>& config,
        const std::unique_ptr<<OHOS::Location::Location>& location);
};
}
}
#endif