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

LocationErrCode LocatorImpl::IsLocationEnabled(int &state)
{
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::IsLocationEnabled()");
    return client_->GetSwitchState(state);
}

LocationErrCode LocatorImpl::ShowNotification()
{
    LBSLOGI(LOCATION_NAPI, "ShowNotification");
    return ERRCODE_SUCCESS;
}

LocationErrCode LocatorImpl::RequestPermission()
{
    LBSLOGI(LOCATION_NAPI, "permission need to be granted");
    return ERRCODE_SUCCESS;
}

LocationErrCode LocatorImpl::RequestEnableLocation()
{
    LBSLOGI(LOCATION_NAPI, "RequestEnableLocation");
    return ERRCODE_SUCCESS;
}

LocationErrCode LocatorImpl::EnableAbility(bool enable)
{
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::EnableAbility()");
    return client_->EnableAbility(enable);
}

LocationErrCode LocatorImpl::StartLocating(std::unique_ptr<RequestConfig>& requestConfig,
    sptr<ILocatorCallback>& callback)
{
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::StartLocating()");
    return client_->StartLocating(requestConfig, callback, "location.ILocator", 0, 0);
}

LocationErrCode LocatorImpl::StopLocating(sptr<ILocatorCallback>& callback)
{
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::StopLocating()");
    return client_->StopLocating(callback);
}

LocationErrCode LocatorImpl::GetCachedLocation(std::unique_ptr<Location> &loc)
{
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::GetCachedLocation()");
    return client_->GetCacheLocation(loc);
}

LocationErrCode LocatorImpl::RegisterSwitchCallback(const sptr<IRemoteObject>& callback, pid_t uid)
{
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::RegisterSwitchCallback()");
    return client_->RegisterSwitchCallback(callback, DEFAULT_UID);
}

LocationErrCode LocatorImpl::UnregisterSwitchCallback(const sptr<IRemoteObject>& callback)
{
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::UnregisterSwitchCallback()");
    return client_->UnregisterSwitchCallback(callback);
}

LocationErrCode LocatorImpl::RegisterGnssStatusCallback(const sptr<IRemoteObject>& callback, pid_t uid)
{
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::RegisterGnssStatusCallback()");
    return client_->RegisterGnssStatusCallback(callback, DEFAULT_UID);
}

LocationErrCode LocatorImpl::UnregisterGnssStatusCallback(const sptr<IRemoteObject>& callback)
{
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::UnregisterGnssStatusCallback()");
    return client_->UnregisterGnssStatusCallback(callback);
}

LocationErrCode LocatorImpl::RegisterNmeaMessageCallback(const sptr<IRemoteObject>& callback, pid_t uid)
{
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::RegisterNmeaMessageCallback()");
    return client_->RegisterNmeaMessageCallback(callback, DEFAULT_UID);
}

LocationErrCode LocatorImpl::UnregisterNmeaMessageCallback(const sptr<IRemoteObject>& callback)
{
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::UnregisterNmeaMessageCallback()");
    return client_->UnregisterNmeaMessageCallback(callback);
}

LocationErrCode LocatorImpl::RegisterNmeaMessageCallbackV9(const sptr<IRemoteObject>& callback)
{
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::RegisterNmeaMessageCallbackV9()");
    return client_->RegisterNmeaMessageCallbackV9(callback);
}

LocationErrCode LocatorImpl::UnregisterNmeaMessageCallbackV9(const sptr<IRemoteObject>& callback)
{
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::UnregisterNmeaMessageCallbackV9()");
    return client_->UnregisterNmeaMessageCallbackV9(callback);
}

LocationErrCode LocatorImpl::RegisterCountryCodeCallback(const sptr<IRemoteObject>& callback, pid_t uid)
{
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::RegisterCountryCodeCallback()");
    return client_->RegisterCountryCodeCallback(callback, uid);
}

LocationErrCode LocatorImpl::UnregisterCountryCodeCallback(const sptr<IRemoteObject>& callback)
{
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::UnregisterCountryCodeCallback()");
    return client_->UnregisterCountryCodeCallback(callback);
}

LocationErrCode LocatorImpl::RegisterCachedLocationCallback(std::unique_ptr<CachedGnssLocationsRequest>& request,
    sptr<ICachedLocationsCallback>& callback)
{
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::RegisterCachedLocationCallback()");
    return client_->RegisterCachedLocationCallback(request, callback, "location.ILocator");
}

LocationErrCode LocatorImpl::UnregisterCachedLocationCallback(sptr<ICachedLocationsCallback>& callback)
{
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::UnregisterCachedLocationCallback()");
    return client_->UnregisterCachedLocationCallback(callback);
}

LocationErrCode LocatorImpl::IsGeoServiceAvailable(bool &isAvailable)
{
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::IsGeoServiceAvailable()");
    return client_->IsGeoConvertAvailable(isAvailable);
}

LocationErrCode LocatorImpl::GetAddressByCoordinate(MessageParcel &data,
    std::list<std::shared_ptr<GeoAddress>>& replyList)
{
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::GetAddressByCoordinate()");
    return client_->GetAddressByCoordinate(data, replyList);
}

LocationErrCode LocatorImpl::GetAddressByLocationName(MessageParcel &data,
    std::list<std::shared_ptr<GeoAddress>>& replyList)
{
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::GetAddressByLocationName()");
    return client_->GetAddressByLocationName(data, replyList);
}

LocationErrCode LocatorImpl::IsLocationPrivacyConfirmed(const int type, bool &isConfirmed)
{
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::IsLocationPrivacyConfirmed()");
    return client_->IsLocationPrivacyConfirmed(type, isConfirmed);
}

LocationErrCode LocatorImpl::SetLocationPrivacyConfirmStatus(const int type, bool isConfirmed)
{
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::SetLocationPrivacyConfirmStatus()");
    return client_->SetLocationPrivacyConfirmStatus(type, isConfirmed);
}

LocationErrCode LocatorImpl::GetCachedGnssLocationsSize(int &size)
{
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::GetCachedGnssLocationsSize()");
    return client_->GetCachedGnssLocationsSize(size);
}

LocationErrCode LocatorImpl::FlushCachedGnssLocations()
{
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::FlushCachedGnssLocations()");
    return client_->FlushCachedGnssLocations();
}

LocationErrCode LocatorImpl::SendCommand(std::unique_ptr<LocationCommand>& commands)
{
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::SendCommand()");
    return client_->SendCommand(commands);
}

LocationErrCode LocatorImpl::AddFence(std::unique_ptr<GeofenceRequest>& request)
{
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::AddFence()");
    return client_->AddFence(request);
}

LocationErrCode LocatorImpl::RemoveFence(std::unique_ptr<GeofenceRequest>& request)
{
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::RemoveFence()");
    return client_->RemoveFence(request);
}

LocationErrCode LocatorImpl::GetIsoCountryCode(std::shared_ptr<CountryCode>& countryCode)
{
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::GetIsoCountryCode()");
    return client_->GetIsoCountryCode(countryCode);
}

LocationErrCode LocatorImpl::EnableLocationMock()
{
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::EnableLocationMock()");
    return client_->EnableLocationMock();
}

LocationErrCode LocatorImpl::DisableLocationMock()
{
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::DisableLocationMock()");
    return client_->DisableLocationMock();
}

LocationErrCode LocatorImpl::SetMockedLocations(
    const int timeInterval, const std::vector<std::shared_ptr<Location>> &location)
{
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::SetMockedLocations()");
    return client_->SetMockedLocations(timeInterval, location);
}

LocationErrCode LocatorImpl::EnableReverseGeocodingMock()
{
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::EnableReverseGeocodingMock()");
    return client_->EnableReverseGeocodingMock();
}

LocationErrCode LocatorImpl::DisableReverseGeocodingMock()
{
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::DisableReverseGeocodingMock()");
    return client_->DisableReverseGeocodingMock();
}

LocationErrCode LocatorImpl::SetReverseGeocodingMockInfo(std::vector<std::shared_ptr<GeocodingMockInfo>>& mockInfo)
{
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::SetReverseGeocodingMockInfo()");
    return client_->SetReverseGeocodingMockInfo(mockInfo);
}

LocationErrCode LocatorImpl::ProxyUidForFreeze(int32_t uid, bool isProxy)
{
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::ProxyUid()");
    return client_->ProxyUidForFreeze(uid, isProxy);
}

LocationErrCode LocatorImpl::ResetAllProxy()
{
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::ResetAllProxy()");
    return client_->ResetAllProxy();
}
}  // namespace Location
}  // namespace OHOS
