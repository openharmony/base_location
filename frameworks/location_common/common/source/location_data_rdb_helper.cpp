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
#include "rdb_errno.h"
#include "system_ability_definition.h"

#include "common_utils.h"
#include "i_locator.h"
#include "location_log.h"

namespace OHOS {
namespace Location {
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
    auto remote = sptr<ILocator>(new (std::nothrow) IRemoteStub<ILocator>());
    if (remote == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s: remoteObject is nullptr", __func__);
        return;
    }
    remoteObj_ = remote->AsObject();
    dataShareHelper_ = CreateDataShareHelper();
}

std::shared_ptr<DataShare::DataShareHelper> LocationDataRdbHelper::CreateDataShareHelper()
{
    if (remoteObj_ == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s: remoteObject is nullptr, reInitialize", __func__);
        Initialize();
    }
    return DataShare::DataShareHelper::Creator(remoteObj_, LOCATION_DATA_URI);
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
    if (dataShareHelper_ == nullptr) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    dataShareHelper_->RegisterObserver(uri, dataObserver);
    ReleaseDataShareHelper(dataShareHelper_);
    return ERRCODE_SUCCESS;
}

LocationErrCode LocationDataRdbHelper::UnregisterDataObserver(
    const Uri &uri, const sptr<AAFwk::IDataAbilityObserver> &dataObserver)
{
    if (dataShareHelper_ == nullptr) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    dataShareHelper_->UnregisterObserver(uri, dataObserver);
    ReleaseDataShareHelper(dataShareHelper_);
    return ERRCODE_SUCCESS;
}

LocationErrCode LocationDataRdbHelper::GetValue(Uri &uri, const std::string &column, int32_t &value)
{
    if (dataShareHelper_ == nullptr) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    DataShare::DataSharePredicates predicates;
    std::vector<std::string> columns;
    predicates.EqualTo(LOCATION_DATA_COLUMN_KEYWORD, column);
    auto rows = dataShareHelper_->Query(uri, predicates, columns);
    if (rows == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s can not get rows", __func__);
        ReleaseDataShareHelper(dataShareHelper_);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    ReleaseDataShareHelper(dataShareHelper_);
    rows->GoToFirstRow();
    int32_t columnIndex;
    rows->GetColumnIndex(LOCATION_DATA_COLUMN_VALUE, columnIndex);
    std::string valueStr;
    int32_t ret = rows->GetString(columnIndex, valueStr);
    if (ret != NativeRdb::E_OK) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s can not get value", __func__);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    rows->Close();
    value = atoi(valueStr.c_str());
    LBSLOGI(LOCATOR_STANDARD, "LocationDataRdbHelper:%{public}s success, value = %{public}d", __func__, value);
    return ERRCODE_SUCCESS;
}

LocationErrCode LocationDataRdbHelper::SetValue(Uri &uri, const std::string &column, int &value)
{
    if (dataShareHelper_ == nullptr) {
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
        err = dataShareHelper_->Insert(uri, bucket);
    } else {
        DataShare::DataSharePredicates predicates;
        predicates.EqualTo(LOCATION_DATA_COLUMN_KEYWORD, column);
        err = dataShareHelper_->Update(uri, predicates, bucket);
    }
    if (err == -1) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s: can not set value", __func__);
        ReleaseDataShareHelper(dataShareHelper_);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    dataShareHelper_->NotifyChange(uri);
    ReleaseDataShareHelper(dataShareHelper_);
    return ERRCODE_SUCCESS;
}

} // namespace Location
} // namespace OHOS