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
#include <cJSON.h>
#include <sys/time.h>

#include "poi_info_callback_napi.h"
#include "common_utils.h"
#include "location_log.h"
#include "parameters.h"

namespace OHOS {
namespace Location {
const int POI_TIME_OUT = 12;
const double DEFAULT_LON = -181;
const double DEFAULT_LAT = -91;
static constexpr int MAX_POI_ARRAY_SIZE = 20;

PoiInfoCallbackNapi::PoiInfoCallbackNapi()
{}

PoiInfoCallbackNapi::~PoiInfoCallbackNapi()
{}

int PoiInfoCallbackNapi::OnRemoteRequest(
    uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option)
{
    if (data.ReadInterfaceToken() != GetDescriptor()) {
        LBSLOGE(POI, "invalid token.");
        return -1;
    }

    switch (code) {
        case RECEIVE_POI_INFO_EVENT: {
            auto transid = Str16ToStr8(data.ReadString16());
            auto errCode = Str16ToStr8(data.ReadString16());
            if (errCode != "0") {
                OnErrorReport(errCode);
                break;
            }
            auto poiNum = data.ReadInt32();
            LBSLOGI(POI, "Receive PoiInfo Num:%{public}d.", poiNum);
            if (poiNum > MAX_POI_ARRAY_SIZE) {
                poiNum = MAX_POI_ARRAY_SIZE;
            }
            std::shared_ptr<PoiInfo> poiInfos = std::make_shared<PoiInfo>();
            if (poiInfos == nullptr) {
                LBSLOGE(POI, "null poiInfos");
                OnErrorReport(errCode);
                break;
            }
            int64_t currentTime = CommonUtils::GetCurrentTimeMilSec();
            if (currentTime < 0) {
                LBSLOGE(POI, "Negative CurrentTime Set POI Time 0.");
                poiInfos->timestamp = 0;
            } else {
                poiInfos->timestamp = static_cast<uint64_t> (currentTime);
            }
            for (int i = 0; i < poiNum; i++) {
                auto poiInfoStr = Str16ToStr8(data.ReadString16());
                Poi poiInfo = ParsePoiInfoFromJson(poiInfoStr);
                poiInfos->poiArray.push_back(poiInfo);
            }
            OnPoiInfoChange(poiInfos);
            break;
        }
        case ERROR_INFO_EVENT: {
            auto errCode = Str16ToStr8(data.ReadString16());
            OnErrorReport(errCode);
            break;
        }
        default: {
            IPCObjectStub::OnRemoteRequest(code, data, reply, option);
            break;
        }
    }
    return 0;
}

void PoiInfoCallbackNapi::OnPoiInfoChange(std::shared_ptr<PoiInfo> &results)
{
    std::unique_lock<std::mutex> uniqueLock(mutex_);
    result_ = results;
    errorCode_ = ERRCODE_SUCCESS;
    ready_ = true;
    condition_.notify_all();
}

void PoiInfoCallbackNapi::OnErrorReport(const std::string errorCode)
{
    LBSLOGI(POI, "OnErrorReport errorCode:%{public}s", errorCode.c_str());
    std::unique_lock<std::mutex> uniqueLock(mutex_);
    if (errorCode == std::to_string(LocationErrCode::ERRCODE_PERMISSION_DENIED)) {
        errorCode_ = ERRCODE_PERMISSION_DENIED;
    }
    ready_ = true;
    condition_.notify_all();
}

void PoiInfoCallbackNapi::Wait()
{
    std::unique_lock<std::mutex> lock(mutex_);
    condition_.wait_for(lock, std::chrono::seconds(POI_TIME_OUT), [this]() { return ready_; });
}

std::shared_ptr<PoiInfo> PoiInfoCallbackNapi::GetResult()
{
    std::unique_lock<std::mutex> lock(mutex_);
    return result_;
}

ErrCode PoiInfoCallbackNapi::GetErrorCode()
{
    return errorCode_;
}

Poi PoiInfoCallbackNapi::ParsePoiInfoFromJson(std::string &poiJson)
{
    Poi poi;
    poi.latitude = DEFAULT_LAT;
    poi.longitude = DEFAULT_LON;
    cJSON* cJsonObj = cJSON_Parse(poiJson.c_str());
    if (!cJsonObj) {
        return poi;
    }
    cJSON *item = cJSON_GetObjectItem(cJsonObj, "id");
    poi.id = (item && cJSON_IsString(item)) ? item->valuestring : "";
    item = cJSON_GetObjectItem(cJsonObj, "confidence");
    poi.confidence = (item && cJSON_IsNumber(item)) ? item->valuedouble : 0.0;
    item = cJSON_GetObjectItem(cJsonObj, "name");
    poi.name = (item && cJSON_IsString(item)) ? item->valuestring : "";
    item = cJSON_GetObjectItem(cJsonObj, "address");
    poi.address = (item && cJSON_IsString(item)) ? item->valuestring : "";

    cJSON* poisLoc = cJSON_GetObjectItem(cJsonObj, "location");
    if (poisLoc) {
        item = cJSON_GetObjectItem(poisLoc, "lat");
        poi.latitude = (item && cJSON_IsNumber(item)) ? item->valuedouble : DEFAULT_LAT;
        item = cJSON_GetObjectItem(poisLoc, "lon");
        poi.longitude = (item && cJSON_IsNumber(item)) ? item->valuedouble : DEFAULT_LON;
    }

    cJSON* poisArea = cJSON_GetObjectItem(cJsonObj, "admin");
    if (poisArea) {
        item = cJSON_GetObjectItem(poisArea, "level1");
        poi.administrativeArea = (item && cJSON_IsString(item)) ? item->valuestring : "";
        item = cJSON_GetObjectItem(poisArea, "level2");
        poi.subAdministrativeArea = (item && cJSON_IsString(item)) ? item->valuestring : "";
        poi.locality = (item && cJSON_IsString(item)) ? item->valuestring : "";
        item = cJSON_GetObjectItem(poisArea, "level3");
        poi.subLocality = (item && cJSON_IsString(item)) ? item->valuestring : "";
    }
    cJSON_Delete(cJsonObj);
    return poi;
}
} // namespace Location
} // namespace OHOS