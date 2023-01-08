/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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

#include "geo_convert_service.h"
#include <file_ex.h>
#include "geo_address.h"
#include "location_dumper.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace Location {
const bool REGISTER_RESULT = SystemAbility::MakeAndRegisterAbility(
    DelayedSingleton<GeoConvertService>::GetInstance().get());

GeoConvertService::GeoConvertService() : SystemAbility(LOCATION_GEO_CONVERT_SA_ID, true)
{
    LBSLOGI(GEO_CONVERT, "GeoConvertService constructed.");
}

GeoConvertService::~GeoConvertService()
{
}

void GeoConvertService::OnStart()
{
    if (state_ == ServiceRunningState::STATE_RUNNING) {
        LBSLOGI(GEO_CONVERT, "GeoConvertService has already started.");
        return;
    }
    if (!Init()) {
        LBSLOGE(GEO_CONVERT, "failed to init GeoConvertService");
        OnStop();
        return;
    }
    state_ = ServiceRunningState::STATE_RUNNING;
    LBSLOGI(GEO_CONVERT, "GeoConvertService::OnStart start service success.");
}

void GeoConvertService::OnStop()
{
    state_ = ServiceRunningState::STATE_NOT_START;
    registerToService_ = false;
    LBSLOGI(GEO_CONVERT, "GeoConvertService::OnStop service stopped.");
}

bool GeoConvertService::Init()
{
    if (!registerToService_) {
        bool ret = Publish(AsObject());
        if (!ret) {
            LBSLOGE(GEO_CONVERT, "GeoConvertService::Init Publish failed!");
            return false;
        }
        registerToService_ = true;
    }
    return true;
}

int GeoConvertService::IsGeoConvertAvailable(MessageParcel &reply)
{
    if (!mockEnabled_) {
        reply.WriteInt32(ERRCODE_NOT_SUPPORTED);
        return ERRCODE_NOT_SUPPORTED;
    }
    reply.WriteInt32(ERRCODE_SUCCESS);
    reply.WriteInt32(GEO_CONVERT_AVAILABLE);
    return ERRCODE_SUCCESS;
}

int GeoConvertService::GetAddressByCoordinate(MessageParcel &data, MessageParcel &reply)
{
    LBSLOGD(GEO_CONVERT, "GetAddressByCoordinate");
    if (!mockEnabled_) {
        reply.WriteInt32(ERRCODE_NOT_SUPPORTED);
        return ERRCODE_NOT_SUPPORTED;
    }
    ReportAddressMock(data, reply);
    return ERRCODE_SUCCESS;
}

void GeoConvertService::ReportAddressMock(MessageParcel &data, MessageParcel &reply)
{
    int arraySize = 0;
    std::vector<std::shared_ptr<GeoAddress>> array;
    ReverseGeocodeRequest request;
    request.latitude = data.ReadDouble();
    request.longitude = data.ReadDouble();
    request.maxItems = data.ReadInt32();
    data.ReadInt32(); // locale size
    request.locale = Str16ToStr8(data.ReadString16());
    for (size_t i = 0; i < mockInfo_.size(); i++) {
        std::shared_ptr<GeocodingMockInfo> info = mockInfo_[i];
        if (!CommonUtils::DoubleEqual(request.latitude, info->GetLocation()->latitude) ||
            !CommonUtils::DoubleEqual(request.longitude, info->GetLocation()->longitude)) {
            continue;
        }
        arraySize++;
        array.push_back(info->GetGeoAddressInfo());
    }
    reply.WriteInt32(ERRCODE_SUCCESS);
    if (arraySize > 0) {
        reply.WriteInt32(arraySize);
        for (size_t i = 0; i < array.size(); i++) {
            array[i]->Marshalling(reply);
        }
    } else {
        reply.WriteInt32(0);
    }
}

int GeoConvertService::GetAddressByLocationName(MessageParcel &data, MessageParcel &reply)
{
    LBSLOGD(GEO_CONVERT, "GetAddressByLocationName");
    reply.WriteInt32(ERRCODE_NOT_SUPPORTED);
    return ERRCODE_NOT_SUPPORTED;
}

bool GeoConvertService::EnableReverseGeocodingMock()
{
    LBSLOGD(GEO_CONVERT, "EnableReverseGeocodingMock");
    mockEnabled_ = true;
    return true;
}

bool GeoConvertService::DisableReverseGeocodingMock()
{
    LBSLOGD(GEO_CONVERT, "DisableReverseGeocodingMock");
    mockEnabled_ = false;
    return true;
}

bool GeoConvertService::SetReverseGeocodingMockInfo(std::vector<std::shared_ptr<GeocodingMockInfo>>& mockInfo)
{
    LBSLOGD(GEO_CONVERT, "SetReverseGeocodingMockInfo");
    mockInfo_.assign(mockInfo.begin(), mockInfo.end());
    return true;
}

void GeoConvertService::SaDumpInfo(std::string& result)
{
    result += "GeoConvert enable status: false";
    result += "\n";
}

int32_t GeoConvertService::Dump(int32_t fd, const std::vector<std::u16string>& args)
{
    std::vector<std::string> vecArgs;
    std::transform(args.begin(), args.end(), std::back_inserter(vecArgs), [](const std::u16string &arg) {
        return Str16ToStr8(arg);
    });

    LocationDumper dumper;
    std::string result;
    dumper.GeocodeDump(SaDumpInfo, vecArgs, result);
    if (!SaveStringToFd(fd, result)) {
        LBSLOGE(GEO_CONVERT, "Geocode save string to fd failed!");
        return ERR_OK;
    }
    return ERR_OK;
}
} // namespace Location
} // namespace OHOS
