/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "location_data_rdb_helper.h"

#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "location_sa_load_manager.h"
#include "system_ability_definition.h"

#include "datashare_predicates.h"
#include "datashare_values_bucket.h"
#include "common_utils.h"
#include "ilocator_service.h"
#include "location_log.h"
#include "location_data_rdb_manager.h"

namespace OHOS {
namespace Location {
constexpr const char* LOCATION_DATA_COLUMN_KEYWORD = "KEYWORD";
constexpr const char* LOCATION_DATA_COLUMN_VALUE = "VALUE";

LocationDataRdbHelper* LocationDataRdbHelper::GetInstance()
{
    static LocationDataRdbHelper data;
    return &data;
}

LocationDataRdbHelper::LocationDataRdbHelper()
{
    Initialize();
}

LocationDataRdbHelper::~LocationDataRdbHelper()
{
    remoteObj_ = nullptr;
}

void LocationDataRdbHelper::Initialize()
{
    sptr<IRemoteObject> objectLocator =
        CommonUtils::GetRemoteObject(LOCATION_LOCATOR_SA_ID, CommonUtils::InitDeviceId());
    if (objectLocator == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get locator sa failed", __func__);
        return;
    }
    sptr<ILocatorService> client = iface_cast<ILocatorService>(objectLocator);
    if (!client || !client->AsObject()) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s: get locator service failed.", __func__);
        return;
    }
    remoteObj_ = client->AsObject();
}

std::shared_ptr<DataShare::DataShareHelper> LocationDataRdbHelper::CreateDataShareHelper()
{
    if (remoteObj_ == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s: remoteObject is nullptr, reInitialize", __func__);
        Initialize();
    }
    return DataShare::DataShareHelper::Creator(remoteObj_,
        LocationDataRdbManager::GetLocationDataUriByCurrentUserId("location_enable"));
}

void LocationDataRdbHelper::ReleaseDataShareHelper(std::shared_ptr<DataShare::DataShareHelper>& dataShareHelper)
{
    if (dataShareHelper == nullptr) {
        return;
    }
    dataShareHelper->Release();
}

LocationErrCode LocationDataRdbHelper::RegisterDataObserver(
    const Uri &uri, const sptr<AAFwk::IDataAbilityObserver> &dataObserver)
{
    auto dataShareHelper = CreateDataShareHelper();
    if (dataShareHelper == nullptr) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    dataShareHelper->RegisterObserver(uri, dataObserver);
    ReleaseDataShareHelper(dataShareHelper);
    return ERRCODE_SUCCESS;
}

LocationErrCode LocationDataRdbHelper::UnregisterDataObserver(
    const Uri &uri, const sptr<AAFwk::IDataAbilityObserver> &dataObserver)
{
    auto dataShareHelper = CreateDataShareHelper();
    if (dataShareHelper == nullptr) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    dataShareHelper->UnregisterObserver(uri, dataObserver);
    ReleaseDataShareHelper(dataShareHelper);
    return ERRCODE_SUCCESS;
}

LocationErrCode LocationDataRdbHelper::GetValue(Uri &uri, const std::string &column, int32_t &value)
{
    std::string valueStr;
    LocationErrCode errCode = GetStringValue(uri, column, valueStr);
    value = atoi(valueStr.c_str());
    LBSLOGD(LOCATOR_STANDARD, "LocationDataRdbHelper:%{public}s success, value = %{public}d", __func__, value);
    return errCode;
}

LocationErrCode LocationDataRdbHelper::GetStringValue(Uri &uri, const std::string &column, std::string &value)
{
    auto dataShareHelper = CreateDataShareHelper();
    if (dataShareHelper == nullptr) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    DataShare::DataSharePredicates predicates;
    std::vector<std::string> columns;
    predicates.EqualTo(LOCATION_DATA_COLUMN_KEYWORD, column);
    auto rows = dataShareHelper->Query(uri, predicates, columns);
    if (rows == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s can not get rows", __func__);
        ReleaseDataShareHelper(dataShareHelper);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    rows->GoToFirstRow();
    int32_t columnIndex;
    rows->GetColumnIndex(LOCATION_DATA_COLUMN_VALUE, columnIndex);
    std::string valueStr;
    int32_t ret = rows->GetString(columnIndex, valueStr);
    if (ret != 0) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s can not get value", __func__);
        rows->Close();
        ReleaseDataShareHelper(dataShareHelper);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    rows->Close();
    ReleaseDataShareHelper(dataShareHelper);
    value = valueStr;
    return ERRCODE_SUCCESS;
}

LocationErrCode LocationDataRdbHelper::SetValue(Uri &uri, const std::string &column, int &value)
{
    auto dataShareHelper = CreateDataShareHelper();
    if (dataShareHelper == nullptr) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    int32_t oldVal = 0;
    LocationErrCode errorCode = GetValue(uri, column, oldVal);
    DataShare::DataShareValueObject keyObj(column);
    DataShare::DataShareValueObject valueObj(std::to_string(value));
    DataShare::DataShareValuesBucket bucket;
    bucket.Put(LOCATION_DATA_COLUMN_VALUE, valueObj);
    bucket.Put(LOCATION_DATA_COLUMN_KEYWORD, keyObj);
    int32_t err;
    if (errorCode != ERRCODE_SUCCESS) {
        err = dataShareHelper->Insert(uri, bucket);
    } else {
        DataShare::DataSharePredicates predicates;
        predicates.EqualTo(LOCATION_DATA_COLUMN_KEYWORD, column);
        err = dataShareHelper->Update(uri, predicates, bucket);
    }
    if (err < 0) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s: can not set value", __func__);
        ReleaseDataShareHelper(dataShareHelper);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    dataShareHelper->NotifyChange(uri);
    ReleaseDataShareHelper(dataShareHelper);
    return ERRCODE_SUCCESS;
}

LocationErrCode LocationDataRdbHelper::GetIntelligentValueValue(
    Uri &uri, const std::string &column, std::string &value)
{
    if (remoteObj_ == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s: remoteObject is nullptr, reInitialize", __func__);
        Initialize();
    }
    auto dataShareHelper = DataShare::DataShareHelper::Creator(remoteObj_,
        "datashare:///com.huawei.hmsapp.intelligent");
    if (dataShareHelper == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "create dataShareHelper failed");
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    DataShare::DataSharePredicates predicates;
    std::vector<std::string> columns;
    auto rows = dataShareHelper->Query(uri, predicates, columns);
    if (rows == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s can not get rows", __func__);
        ReleaseDataShareHelper(dataShareHelper);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    rows->GoToFirstRow();
    int32_t columnIndex;
    rows->GetColumnIndex("locationStatus", columnIndex);
    std::string valueStr;
    int32_t ret = rows->GetString(columnIndex, valueStr);
    if (ret != 0) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s can not get value", __func__);
        rows->Close();
        ReleaseDataShareHelper(dataShareHelper);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    rows->Close();
    ReleaseDataShareHelper(dataShareHelper);
    value = valueStr;
    return ERRCODE_SUCCESS;
}
} // namespace Location
} // namespace OHOS