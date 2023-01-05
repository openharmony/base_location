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

bool LocatorImpl::IsLocationEnabled()
{
    return client_->GetSwitchState() == 1;
}

void LocatorImpl::ShowNotification()
{
    LBSLOGI(LOCATION_NAPI, "ShowNotification");
}

void LocatorImpl::RequestPermission()
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
    MessageParcel reply;
    client_->GetCacheLocation(reply);
    int exception = reply.ReadInt32();
    if (exception == ERRCODE_PERMISSION_DENIED) {
        LBSLOGE(LOCATOR_STANDARD, "can not get cached location without location permission.");
    } else if (exception != ERRCODE_SUCCESS) {
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
    MessageParcel reply;
    client_->IsGeoConvertAvailable(reply);
    int exception = reply.ReadInt32();
    if (exception == ERRCODE_PERMISSION_DENIED) {
        LBSLOGE(LOCATOR_STANDARD, "can not get cached location without location permission.");
    } else if (exception != ERRCODE_SUCCESS) {
        LBSLOGE(LOCATOR_STANDARD, "cause some exception happened in lower service.");
    } else {
        result = reply.ReadBool();
    }
    return result;
}

void LocatorImpl::GetAddressByCoordinate(MessageParcel &data, std::list<std::shared_ptr<GeoAddress>>& replyList)
{
    MessageParcel reply;
    client_->GetAddressByCoordinate(data, reply);
    int exception = reply.ReadInt32();
    if (exception == ERRCODE_PERMISSION_DENIED) {
        LBSLOGE(LOCATOR_STANDARD, "can not get cached location without location permission.");
    } else if (exception != ERRCODE_SUCCESS) {
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
    if (exception == ERRCODE_PERMISSION_DENIED) {
        LBSLOGE(LOCATOR_STANDARD, "can not get cached location without location permission.");
    } else if (exception != ERRCODE_SUCCESS) {
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

int LocatorImpl::SetLocationPrivacyConfirmStatus(const int type, bool isConfirmed)
{
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::SetLocationPrivacyConfirmStatus()");
    return client_->SetLocationPrivacyConfirmStatus(type, isConfirmed);
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

bool LocatorImpl::EnableLocationMock()
{
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::EnableLocationMock()");
    return client_->EnableLocationMock();
}

bool LocatorImpl::DisableLocationMock()
{
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::DisableLocationMock()");
    return client_->DisableLocationMock();
}

bool LocatorImpl::SetMockedLocations(
    const int timeInterval, const std::vector<std::shared_ptr<Location>> &location)
{
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::SetMockedLocations()");
    return client_->SetMockedLocations(timeInterval, location);
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

bool LocatorImpl::ProxyUidForFreeze(int32_t uid, bool isProxy)
{
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::ProxyUid()");
    return client_->ProxyUidForFreeze(uid, isProxy);
}

bool LocatorImpl::ResetAllProxy()
{
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::ResetAllProxy()");
    return client_->ResetAllProxy();
}

LocationErrCode LocatorImpl::IsLocationEnabledV9(bool &isEnabled)
{
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::IsLocationEnabledV9()");
    return client_->GetSwitchStateV9(isEnabled);
}

LocationErrCode LocatorImpl::ShowNotificationV9()
{
    LBSLOGI(LOCATION_NAPI, "ShowNotification");
    return ERRCODE_SUCCESS;
}

LocationErrCode LocatorImpl::RequestPermissionV9()
{
    LBSLOGI(LOCATION_NAPI, "permission need to be granted");
    return ERRCODE_SUCCESS;
}

LocationErrCode LocatorImpl::RequestEnableLocationV9()
{
    LBSLOGI(LOCATION_NAPI, "RequestEnableLocation");
    return ERRCODE_SUCCESS;
}

LocationErrCode LocatorImpl::EnableAbilityV9(bool enable)
{
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::EnableAbilityV9()");
    return client_->EnableAbilityV9(enable);
}

LocationErrCode LocatorImpl::StartLocatingV9(std::unique_ptr<RequestConfig>& requestConfig,
    sptr<ILocatorCallback>& callback)
{
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::StartLocatingV9()");
    return client_->StartLocatingV9(requestConfig, callback, "location.ILocator", 0, 0);
}

LocationErrCode LocatorImpl::StopLocatingV9(sptr<ILocatorCallback>& callback)
{
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::StopLocatingV9()");
    return client_->StopLocatingV9(callback);
}

LocationErrCode LocatorImpl::GetCachedLocationV9(std::unique_ptr<Location> &loc)
{
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::GetCachedLocationV9()");
    return client_->GetCacheLocationV9(loc);
}

LocationErrCode LocatorImpl::RegisterSwitchCallbackV9(const sptr<IRemoteObject>& callback, pid_t uid)
{
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::RegisterSwitchCallbackV9()");
    return client_->RegisterSwitchCallbackV9(callback, DEFAULT_UID);
}

LocationErrCode LocatorImpl::UnregisterSwitchCallbackV9(const sptr<IRemoteObject>& callback)
{
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::UnregisterSwitchCallbackV9()");
    return client_->UnregisterSwitchCallbackV9(callback);
}

LocationErrCode LocatorImpl::RegisterGnssStatusCallbackV9(const sptr<IRemoteObject>& callback, pid_t uid)
{
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::RegisterGnssStatusCallbackV9()");
    return client_->RegisterGnssStatusCallbackV9(callback, DEFAULT_UID);
}

LocationErrCode LocatorImpl::UnregisterGnssStatusCallbackV9(const sptr<IRemoteObject>& callback)
{
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::UnregisterGnssStatusCallbackV9()");
    return client_->UnregisterGnssStatusCallbackV9(callback);
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

LocationErrCode LocatorImpl::RegisterCountryCodeCallbackV9(const sptr<IRemoteObject>& callback, pid_t uid)
{
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::RegisterCountryCodeCallbackV9()");
    return client_->RegisterCountryCodeCallbackV9(callback, uid);
}

LocationErrCode LocatorImpl::UnregisterCountryCodeCallbackV9(const sptr<IRemoteObject>& callback)
{
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::UnregisterCountryCodeCallbackV9()");
    return client_->UnregisterCountryCodeCallbackV9(callback);
}

LocationErrCode LocatorImpl::RegisterCachedLocationCallbackV9(std::unique_ptr<CachedGnssLocationsRequest>& request,
    sptr<ICachedLocationsCallback>& callback)
{
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::RegisterCachedLocationCallbackV9()");
    return client_->RegisterCachedLocationCallbackV9(request, callback, "location.ILocator");
}

LocationErrCode LocatorImpl::UnregisterCachedLocationCallbackV9(sptr<ICachedLocationsCallback>& callback)
{
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::UnregisterCachedLocationCallbackV9()");
    return client_->UnregisterCachedLocationCallbackV9(callback);
}

LocationErrCode LocatorImpl::IsGeoServiceAvailableV9(bool &isAvailable)
{
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::IsGeoServiceAvailableV9()");
    return client_->IsGeoConvertAvailableV9(isAvailable);
}

LocationErrCode LocatorImpl::GetAddressByCoordinateV9(MessageParcel &data,
    std::list<std::shared_ptr<GeoAddress>>& replyList)
{
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::GetAddressByCoordinateV9()");
    return client_->GetAddressByCoordinateV9(data, replyList);
}

LocationErrCode LocatorImpl::GetAddressByLocationNameV9(MessageParcel &data,
    std::list<std::shared_ptr<GeoAddress>>& replyList)
{
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::GetAddressByLocationNameV9()");
    return client_->GetAddressByLocationNameV9(data, replyList);
}

LocationErrCode LocatorImpl::IsLocationPrivacyConfirmedV9(const int type, bool &isConfirmed)
{
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::IsLocationPrivacyConfirmedV9()");
    return client_->IsLocationPrivacyConfirmedV9(type, isConfirmed);
}

LocationErrCode LocatorImpl::SetLocationPrivacyConfirmStatusV9(const int type, bool isConfirmed)
{
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::SetLocationPrivacyConfirmStatusV9()");
    return client_->SetLocationPrivacyConfirmStatusV9(type, isConfirmed);
}

LocationErrCode LocatorImpl::GetCachedGnssLocationsSizeV9(int &size)
{
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::GetCachedGnssLocationsSizeV9()");
    return client_->GetCachedGnssLocationsSizeV9(size);
}

LocationErrCode LocatorImpl::FlushCachedGnssLocationsV9()
{
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::FlushCachedGnssLocationsV9()");
    return client_->FlushCachedGnssLocationsV9();
}

LocationErrCode LocatorImpl::SendCommandV9(std::unique_ptr<LocationCommand>& commands)
{
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::SendCommandV9()");
    return client_->SendCommandV9(commands);
}

LocationErrCode LocatorImpl::AddFenceV9(std::unique_ptr<GeofenceRequest>& request)
{
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::AddFenceV9()");
    return client_->AddFenceV9(request);
}

LocationErrCode LocatorImpl::RemoveFenceV9(std::unique_ptr<GeofenceRequest>& request)
{
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::RemoveFenceV9()");
    return client_->RemoveFenceV9(request);
}

LocationErrCode LocatorImpl::GetIsoCountryCodeV9(std::shared_ptr<CountryCode>& countryCode)
{
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::GetIsoCountryCodeV9()");
    return client_->GetIsoCountryCodeV9(countryCode);
}

LocationErrCode LocatorImpl::EnableLocationMockV9()
{
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::EnableLocationMockV9()");
    return client_->EnableLocationMockV9();
}

LocationErrCode LocatorImpl::DisableLocationMockV9()
{
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::DisableLocationMockV9()");
    return client_->DisableLocationMockV9();
}

LocationErrCode LocatorImpl::SetMockedLocationsV9(
    const int timeInterval, const std::vector<std::shared_ptr<Location>> &location)
{
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::SetMockedLocationsV9()");
    return client_->SetMockedLocationsV9(timeInterval, location);
}

LocationErrCode LocatorImpl::EnableReverseGeocodingMockV9()
{
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::EnableReverseGeocodingMockV9()");
    return client_->EnableReverseGeocodingMockV9();
}

LocationErrCode LocatorImpl::DisableReverseGeocodingMockV9()
{
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::DisableReverseGeocodingMockV9()");
    return client_->DisableReverseGeocodingMockV9();
}

LocationErrCode LocatorImpl::SetReverseGeocodingMockInfoV9(std::vector<std::shared_ptr<GeocodingMockInfo>>& mockInfo)
{
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::SetReverseGeocodingMockInfoV9()");
    return client_->SetReverseGeocodingMockInfoV9(mockInfo);
}

LocationErrCode LocatorImpl::ProxyUidForFreezeV9(int32_t uid, bool isProxy)
{
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::ProxyUidForFreezeV9()");
    return client_->ProxyUidForFreezeV9(uid, isProxy);
}

LocationErrCode LocatorImpl::ResetAllProxyV9()
{
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::ResetAllProxyV9()");
    return client_->ResetAllProxyV9();
}
}  // namespace Location
}  // namespace OHOS
