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
#include "locator_impl.h"
#include "common_utils.h"
#include "iservice_registry.h"
#include "location_log.h"
#include "system_ability_definition.h"
#include "country_code.h"

namespace OHOS {
namespace Location {
LocatorImpl::LocatorImpl()
{}

LocatorImpl::~LocatorImpl()
{}

bool LocatorImpl::Init()
{
    client_ = std::make_unique<LocatorProxy>(CommonUtils::GetRemoteObject(LOCATION_LOCATOR_SA_ID,
        CommonUtils::InitDeviceId()));
    if (client_ == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "locator init failed.");
        return false;
    }

    return true;
}

bool LocatorImpl::IsLocationEnabled()
{
    return client_->GetSwitchState() == 1;
}

void LocatorImpl::ShowNotification()
{
    LBSLOGI(LOCATION_NAPI, "ShowNotification");
}

void LocatorImpl::RequestPermission(napi_env env)
{
    LBSLOGI(LOCATION_NAPI, "permission need to be granted");
}

void LocatorImpl::RequestEnableLocation()
{
    LBSLOGI(LOCATION_NAPI, "RequestEnableLocation");
}

void LocatorImpl::EnableAbility(bool enable)
{
    client_->EnableAbility(enable);
}

void LocatorImpl::StartLocating(std::unique_ptr<RequestConfig>& requestConfig,
    sptr<ILocatorCallback>& callback)
{
    client_->StartLocating(requestConfig, callback, "location.ILocator", 0, 0);
}

void LocatorImpl::StopLocating(sptr<ILocatorCallback>& callback)
{
    client_->StopLocating(callback);
}

std::unique_ptr<Location> LocatorImpl::GetCachedLocation()
{
    std::unique_ptr<Location> location = nullptr;
    MessageParcel data;
    MessageParcel reply;
    client_->GetCacheLocation(data, reply);
    int exception = reply.ReadInt32();
    if (exception == REPLY_CODE_SECURITY_EXCEPTION) {
        LBSLOGE(LOCATOR_STANDARD, "can not get cached location without location permission.");
    } else if (exception != REPLY_CODE_NO_EXCEPTION) {
        LBSLOGE(LOCATOR_STANDARD, "cause some exception happened in lower service.");
    } else {
        location = Location::Unmarshalling(reply);
    }

    return location;
}

bool LocatorImpl::RegisterSwitchCallback(const sptr<IRemoteObject>& callback, pid_t uid)
{
    client_->RegisterSwitchCallback(callback, DEFAULT_UID);
    return true;
}

bool LocatorImpl::UnregisterSwitchCallback(const sptr<IRemoteObject>& callback)
{
    client_->UnregisterSwitchCallback(callback);
    return true;
}

bool LocatorImpl::RegisterGnssStatusCallback(const sptr<IRemoteObject>& callback, pid_t uid)
{
    client_->RegisterGnssStatusCallback(callback, DEFAULT_UID);
    return true;
}

bool LocatorImpl::UnregisterGnssStatusCallback(const sptr<IRemoteObject>& callback)
{
    client_->UnregisterGnssStatusCallback(callback);
    return true;
}

bool LocatorImpl::RegisterNmeaMessageCallback(const sptr<IRemoteObject>& callback, pid_t uid)
{
    client_->RegisterNmeaMessageCallback(callback, DEFAULT_UID);
    return true;
}

bool LocatorImpl::UnregisterNmeaMessageCallback(const sptr<IRemoteObject>& callback)
{
    client_->UnregisterNmeaMessageCallback(callback);
    return true;
}

bool LocatorImpl::RegisterCountryCodeCallback(const sptr<IRemoteObject>& callback, pid_t uid)
{
    client_->RegisterCountryCodeCallback(callback, uid);
    return true;
}

bool LocatorImpl::UnregisterCountryCodeCallback(const sptr<IRemoteObject>& callback)
{
    client_->UnregisterCountryCodeCallback(callback);
    return true;
}

void LocatorImpl::RegisterCachedLocationCallback(std::unique_ptr<CachedGnssLocationsRequest>& request,
    sptr<ICachedLocationsCallback>& callback)
{
    client_->RegisterCachedLocationCallback(request, callback, "location.ILocator");
}

void LocatorImpl::UnregisterCachedLocationCallback(sptr<ICachedLocationsCallback>& callback)
{
    client_->UnregisterCachedLocationCallback(callback);
}

bool LocatorImpl::IsGeoServiceAvailable()
{
    bool result = false;
    MessageParcel data;
    MessageParcel reply;
    client_->IsGeoConvertAvailable(data, reply);
    int exception = reply.ReadInt32();
    if (exception == REPLY_CODE_SECURITY_EXCEPTION) {
        LBSLOGE(LOCATOR_STANDARD, "can not get cached location without location permission.");
    } else if (exception != REPLY_CODE_NO_EXCEPTION) {
        LBSLOGE(LOCATOR_STANDARD, "cause some exception happened in lower service.");
    } else {
        result = (reply.ReadInt32() == 1);
    }
    return result;
}

void LocatorImpl::GetAddressByCoordinate(MessageParcel &data, std::list<std::shared_ptr<GeoAddress>>& replyList)
{
    MessageParcel reply;
    client_->GetAddressByCoordinate(data, reply);
    int exception = reply.ReadInt32();
    if (exception == REPLY_CODE_SECURITY_EXCEPTION) {
        LBSLOGE(LOCATOR_STANDARD, "can not get cached location without location permission.");
    } else if (exception != REPLY_CODE_NO_EXCEPTION) {
        LBSLOGE(LOCATOR_STANDARD, "cause some exception happened in lower service.");
    } else {
        int resultSize = reply.ReadInt32();
        if (resultSize > GeoAddress::MAX_RESULT) {
            resultSize = GeoAddress::MAX_RESULT;
        }
        for (int i = 0; i < resultSize; i++) {
            replyList.push_back(GeoAddress::Unmarshalling(reply));
        }
    }
}

void LocatorImpl::GetAddressByLocationName(MessageParcel &data, std::list<std::shared_ptr<GeoAddress>>& replyList)
{
    MessageParcel reply;
    client_->GetAddressByLocationName(data, reply);
    int exception = reply.ReadInt32();
    if (exception == REPLY_CODE_SECURITY_EXCEPTION) {
        LBSLOGE(LOCATOR_STANDARD, "can not get cached location without location permission.");
    } else if (exception != REPLY_CODE_NO_EXCEPTION) {
        LBSLOGE(LOCATOR_STANDARD, "cause some exception happened in lower service.");
    } else {
        int resultSize = reply.ReadInt32();
        if (resultSize > GeoAddress::MAX_RESULT) {
            resultSize = GeoAddress::MAX_RESULT;
        }
        for (int i = 0; i < resultSize; i++) {
            replyList.push_back(GeoAddress::Unmarshalling(reply));
        }
    }
}

bool LocatorImpl::IsLocationPrivacyConfirmed(const int type)
{
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::IsLocationPrivacyConfirmed()");
    return client_->IsLocationPrivacyConfirmed(type);
}

void LocatorImpl::SetLocationPrivacyConfirmStatus(const int type, bool isConfirmed)
{
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::SetLocationPrivacyConfirmStatus()");
    client_->SetLocationPrivacyConfirmStatus(type, isConfirmed);
}

int LocatorImpl::GetCachedGnssLocationsSize()
{
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::GetCachedGnssLocationsSize()");
    return client_->GetCachedGnssLocationsSize();
}

int LocatorImpl::FlushCachedGnssLocations()
{
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::FlushCachedGnssLocations()");
    return client_->FlushCachedGnssLocations();
}

bool LocatorImpl::SendCommand(std::unique_ptr<LocationCommand>& commands)
{
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::SendCommand()");
    client_->SendCommand(commands);
    return true;
}

bool LocatorImpl::AddFence(std::unique_ptr<GeofenceRequest>& request)
{
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::AddFence()");
    client_->AddFence(request);
    return true;
}

bool LocatorImpl::RemoveFence(std::unique_ptr<GeofenceRequest>& request)
{
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::RemoveFence()");
    client_->RemoveFence(request);
    return true;
}

std::shared_ptr<CountryCode> LocatorImpl::GetIsoCountryCode()

{
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::GetIsoCountryCode()");
    return client_->GetIsoCountryCode();
}

bool LocatorImpl::EnableLocationMock(const LocationMockConfig& config)
{
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::EnableLocationMock()");
    return client_->EnableLocationMock(config);
}

bool LocatorImpl::DisableLocationMock(const LocationMockConfig& config)
{
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::DisableLocationMock()");
    return client_->DisableLocationMock(config);
}

bool LocatorImpl::SetMockedLocations(
    const LocationMockConfig& config,  const std::vector<std::shared_ptr<Location>> &location)
{
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::SetMockedLocations()");
    return client_->SetMockedLocations(config, location);
}

bool LocatorImpl::EnableReverseGeocodingMock()
{
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::EnableReverseGeocodingMock()");
    return client_->EnableReverseGeocodingMock();
}

bool LocatorImpl::DisableReverseGeocodingMock()
{
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::DisableReverseGeocodingMock()");
    return client_->DisableReverseGeocodingMock();
}

bool LocatorImpl::SetReverseGeocodingMockInfo(std::vector<std::shared_ptr<GeocodingMockInfo>>& mockInfo)
{
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::SetReverseGeocodingMockInfo()");
    return client_->SetReverseGeocodingMockInfo(mockInfo);
}
}  // namespace Location
}  // namespace OHOS
