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

#include "locator_skeleton.h"

#include <file_ex.h>
#include "ipc_skeleton.h"
#include "common_utils.h"
#include "i_locator_callback.h"
#include "location.h"
#include "locator_ability.h"
#include "location_dumper.h"
#include "request_config.h"
#include "system_ability_definition.h"
#include "location_sa_load_manager.h"

namespace OHOS {
namespace Location {
void LocatorAbilityStub::InitLocatorHandleMap()
{
    if (locatorHandleMap_.size() != 0 || locatorHandleMap_.size() != 0) {
        return;
    }
    locatorHandleMap_[GET_SWITCH_STATE] = &LocatorAbilityStub::PreGetSwitchState;
    locatorHandleMap_[REG_SWITCH_CALLBACK] = &LocatorAbilityStub::PreRegisterSwitchCallback;
    locatorHandleMap_[UNREG_SWITCH_CALLBACK] = &LocatorAbilityStub::PreUnregisterSwitchCallback;
    locatorHandleMap_[START_LOCATING] = &LocatorAbilityStub::PreStartLocating;
    locatorHandleMap_[STOP_LOCATING] = &LocatorAbilityStub::PreStopLocating;
    locatorHandleMap_[GET_CACHE_LOCATION] = &LocatorAbilityStub::PreGetCacheLocation;
    locatorHandleMap_[ENABLE_ABILITY] = &LocatorAbilityStub::PreEnableAbility;
    locatorHandleMap_[UPDATE_SA_ABILITY] = &LocatorAbilityStub::PreUpdateSaAbility;
    locatorHandleMap_[IS_PRIVACY_COMFIRMED] = &LocatorAbilityStub::PreIsLocationPrivacyConfirmed;
    locatorHandleMap_[SET_PRIVACY_COMFIRM_STATUS] = &LocatorAbilityStub::PreSetLocationPrivacyConfirmStatus;
    locatorHandleMap_[GET_ISO_COUNTRY_CODE] = &LocatorAbilityStub::PreGetIsoCountryCode;
    locatorHandleMap_[ENABLE_LOCATION_MOCK] = &LocatorAbilityStub::PreEnableLocationMock;
    locatorHandleMap_[DISABLE_LOCATION_MOCK] = &LocatorAbilityStub::PreDisableLocationMock;
    locatorHandleMap_[SET_MOCKED_LOCATIONS] = &LocatorAbilityStub::PreSetMockedLocations;
    locatorHandleMap_[REG_COUNTRY_CODE_CALLBACK] = &LocatorAbilityStub::PreRegisterCountryCodeCallback;
    locatorHandleMap_[UNREG_COUNTRY_CODE_CALLBACK] = &LocatorAbilityStub::PreUnregisterCountryCodeCallback;
    locatorHandleMap_[PROXY_UID_FOR_FREEZE] = &LocatorAbilityStub::PreProxyUidForFreeze;
    locatorHandleMap_[RESET_ALL_PROXY] = &LocatorAbilityStub::PreResetAllProxy;
    locatorHandleMap_[REPORT_LOCATION] = &LocatorAbilityStub::PreReportLocation;
#ifdef FEATURE_GEOCODE_SUPPORT
    locatorHandleMap_[GEO_IS_AVAILABLE] = &LocatorAbilityStub::PreIsGeoConvertAvailable;
    locatorHandleMap_[GET_FROM_COORDINATE] = &LocatorAbilityStub::PreGetAddressByCoordinate;
    locatorHandleMap_[GET_FROM_LOCATION_NAME] = &LocatorAbilityStub::PreGetAddressByLocationName;
    locatorHandleMap_[ENABLE_REVERSE_GEOCODE_MOCK] = &LocatorAbilityStub::PreEnableReverseGeocodingMock;
    locatorHandleMap_[DISABLE_REVERSE_GEOCODE_MOCK] = &LocatorAbilityStub::PreDisableReverseGeocodingMock;
    locatorHandleMap_[SET_REVERSE_GEOCODE_MOCKINFO] = &LocatorAbilityStub::PreSetReverseGeocodingMockInfo;
#endif
#ifdef FEATURE_GNSS_SUPPORT
    locatorHandleMap_[REG_GNSS_STATUS_CALLBACK] = &LocatorAbilityStub::PreRegisterGnssStatusCallback;
    locatorHandleMap_[UNREG_GNSS_STATUS_CALLBACK] = &LocatorAbilityStub::PreUnregisterGnssStatusCallback;
    locatorHandleMap_[REG_NMEA_CALLBACK] = &LocatorAbilityStub::PreRegisterNmeaMessageCallback;
    locatorHandleMap_[UNREG_NMEA_CALLBACK] = &LocatorAbilityStub::PreUnregisterNmeaMessageCallback;
    locatorHandleMap_[REG_CACHED_CALLBACK] = &LocatorAbilityStub::PreStartCacheLocating;
    locatorHandleMap_[UNREG_CACHED_CALLBACK] = &LocatorAbilityStub::PreStopCacheLocating;
    locatorHandleMap_[GET_CACHED_LOCATION_SIZE] = &LocatorAbilityStub::PreGetCachedGnssLocationsSize;
    locatorHandleMap_[FLUSH_CACHED_LOCATIONS] = &LocatorAbilityStub::PreFlushCachedGnssLocations;
    locatorHandleMap_[SEND_COMMAND] = &LocatorAbilityStub::PreSendCommand;
    locatorHandleMap_[ADD_FENCE] = &LocatorAbilityStub::PreAddFence;
    locatorHandleMap_[REMOVE_FENCE] = &LocatorAbilityStub::PreRemoveFence;
    locatorHandleMap_[REG_NMEA_CALLBACK_v9] = &LocatorAbilityStub::PreRegisterNmeaMessageCallbackV9;
    locatorHandleMap_[UNREG_NMEA_CALLBACK_v9] = &LocatorAbilityStub::PreUnregisterNmeaMessageCallbackV9;
#endif
}


LocatorAbilityStub::LocatorAbilityStub()
{
    InitLocatorHandleMap();
}

int LocatorAbilityStub::PreGetSwitchState(MessageParcel &data, MessageParcel &reply, AppIdentity &identity)
{
    auto locatorAbility = DelayedSingleton<LocatorAbility>::GetInstance();
    if (locatorAbility == nullptr) {
        LBSLOGE(LOCATOR, "PreGetSwitchState: LocatorAbility is nullptr.");
        reply.WriteInt32(ERRCODE_SERVICE_UNAVAILABLE);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    int state = DISABLED;
    LocationErrCode errorCode = locatorAbility->GetSwitchState(state);
    reply.WriteInt32(errorCode);
    if (errorCode == ERRCODE_SUCCESS) {
        reply.WriteInt32(state);
    }
    return errorCode;
}

int LocatorAbilityStub::PreRegisterSwitchCallback(MessageParcel &data, MessageParcel &reply, AppIdentity &identity)
{
    auto locatorAbility = DelayedSingleton<LocatorAbility>::GetInstance();
    if (locatorAbility == nullptr) {
        LBSLOGE(LOCATOR, "PreRegisterSwitchCallback: LocatorAbility is nullptr.");
        reply.WriteInt32(ERRCODE_SERVICE_UNAVAILABLE);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    sptr<IRemoteObject> client = data.ReadObject<IRemoteObject>();
    reply.WriteInt32(locatorAbility->RegisterSwitchCallback(client, identity.GetUid()));
    return ERRCODE_SUCCESS;
}


int LocatorAbilityStub::PreStartLocating(MessageParcel &data, MessageParcel &reply, AppIdentity &identity)
{
    if (!CheckLocationSwitchState(reply)) {
        return ERRCODE_SWITCH_OFF;
    }
    if (!CheckLocationPermission(reply, identity)) {
        return ERRCODE_PERMISSION_DENIED;
    }
    auto locatorAbility = DelayedSingleton<LocatorAbility>::GetInstance();
    if (locatorAbility == nullptr) {
        LBSLOGE(LOCATOR, "PreStartLocating: LocatorAbility is nullptr.");
        reply.WriteInt32(ERRCODE_SERVICE_UNAVAILABLE);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    std::unique_ptr<RequestConfig> requestConfig = RequestConfig::Unmarshalling(data);
    sptr<IRemoteObject> remoteObject = data.ReadRemoteObject();
    if (remoteObject == nullptr) {
        LBSLOGE(LOCATOR, "StartLocating remote object nullptr");
        reply.WriteInt32(ERRCODE_SERVICE_UNAVAILABLE);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }

    sptr<IRemoteObject::DeathRecipient> death(new (std::nothrow) LocatorCallbackDeathRecipient());
    remoteObject->AddDeathRecipient(death);
    sptr<ILocatorCallback> callback = iface_cast<ILocatorCallback>(remoteObject);
    reply.WriteInt32(locatorAbility->StartLocating(requestConfig, callback, identity));
    return ERRCODE_SUCCESS;
}

int LocatorAbilityStub::PreStopLocating(MessageParcel &data, MessageParcel &reply, AppIdentity &identity)
{
    if (!CheckLocationPermission(reply, identity)) {
        return ERRCODE_PERMISSION_DENIED;
    }
    auto locatorAbility = DelayedSingleton<LocatorAbility>::GetInstance();
    if (locatorAbility == nullptr) {
        LBSLOGE(LOCATOR, "PreStopLocating: LocatorAbility is nullptr.");
        reply.WriteInt32(ERRCODE_SERVICE_UNAVAILABLE);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    sptr<IRemoteObject> remoteObject = data.ReadRemoteObject();
    if (remoteObject == nullptr) {
        LBSLOGE(LOCATOR, "LocatorAbility::StopLocating remote object nullptr");
        reply.WriteInt32(ERRCODE_SERVICE_UNAVAILABLE);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    sptr<ILocatorCallback> callback = iface_cast<ILocatorCallback>(remoteObject);
    reply.WriteInt32(locatorAbility->StopLocating(callback));
    return ERRCODE_SUCCESS;
}

int LocatorAbilityStub::PreGetCacheLocation(MessageParcel &data, MessageParcel &reply, AppIdentity &identity)
{
    if (!CheckLocationSwitchState(reply)) {
        return ERRCODE_SWITCH_OFF;
    }
    if (!CheckLocationPermission(reply, identity)) {
        return ERRCODE_PERMISSION_DENIED;
    }
    auto locatorAbility = DelayedSingleton<LocatorAbility>::GetInstance();
    if (locatorAbility == nullptr) {
        LBSLOGE(LOCATOR, "PreGetCacheLocation: LocatorAbility is nullptr.");
        reply.WriteInt32(ERRCODE_SERVICE_UNAVAILABLE);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    std::unique_ptr<Location> loc;
    reply.WriteInt32(locatorAbility->GetCacheLocation(loc, identity));
    if (loc != nullptr) {
        loc->Marshalling(reply);
    }
    return ERRCODE_SUCCESS;
}

int LocatorAbilityStub::PreEnableAbility(MessageParcel &data, MessageParcel &reply, AppIdentity &identity)
{
    if (!CommonUtils::CheckSystemPermission(identity.GetTokenId(), identity.GetTokenIdEx())) {
        LBSLOGE(LOCATOR, "CheckSystemPermission return false, [%{public}s]",
            identity.ToString().c_str());
        reply.WriteInt32(ERRCODE_SYSTEM_PERMISSION_DENIED);
        return ERRCODE_SYSTEM_PERMISSION_DENIED;
    }
    if (!CheckSettingsPermission(reply, identity)) {
        return ERRCODE_PERMISSION_DENIED;
    }
    auto locatorAbility = DelayedSingleton<LocatorAbility>::GetInstance();
    if (locatorAbility == nullptr) {
        LBSLOGE(LOCATOR, "PreEnableAbility: LocatorAbility is nullptr.");
        reply.WriteInt32(ERRCODE_SERVICE_UNAVAILABLE);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    bool isEnabled = data.ReadBool();
    reply.WriteInt32(locatorAbility->EnableAbility(isEnabled));
    return ERRCODE_SUCCESS;
}

int LocatorAbilityStub::PreUpdateSaAbility(MessageParcel &data, MessageParcel &reply, AppIdentity &identity)
{
    if (!CommonUtils::CheckSystemPermission(identity.GetTokenId(), identity.GetTokenIdEx())) {
        LBSLOGE(LOCATOR, "CheckSystemPermission return false, [%{public}s]",
            identity.ToString().c_str());
        reply.WriteInt32(ERRCODE_SYSTEM_PERMISSION_DENIED);
        return ERRCODE_SYSTEM_PERMISSION_DENIED;
    }
    auto locatorAbility = DelayedSingleton<LocatorAbility>::GetInstance();
    if (locatorAbility == nullptr) {
        LBSLOGE(LOCATOR, "PreUpdateSaAbility: LocatorAbility is nullptr.");
        reply.WriteInt32(ERRCODE_SERVICE_UNAVAILABLE);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    reply.WriteInt32(locatorAbility->UpdateSaAbility());
    return ERRCODE_SUCCESS;
}

#ifdef FEATURE_GEOCODE_SUPPORT
int LocatorAbilityStub::PreIsGeoConvertAvailable(MessageParcel &data, MessageParcel &reply, AppIdentity &identity)
{
    auto locatorAbility = DelayedSingleton<LocatorAbility>::GetInstance();
    if (locatorAbility == nullptr) {
        LBSLOGE(LOCATOR, "PreIsGeoConvertAvailable: LocatorAbility is nullptr.");
        reply.WriteInt32(ERRCODE_SERVICE_UNAVAILABLE);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    bool isAvailable = false;
    reply.WriteInt32(locatorAbility->IsGeoConvertAvailable(isAvailable));
    reply.WriteBool(isAvailable);
    return ERRCODE_SUCCESS;
}
#endif

#ifdef FEATURE_GEOCODE_SUPPORT
int LocatorAbilityStub::PreGetAddressByCoordinate(MessageParcel &data, MessageParcel &reply, AppIdentity &identity)
{
    auto locatorAbility = DelayedSingleton<LocatorAbility>::GetInstance();
    if (locatorAbility == nullptr) {
        LBSLOGE(LOCATOR, "PreGetAddressByCoordinate: LocatorAbility is nullptr.");
        reply.WriteInt32(ERRCODE_SERVICE_UNAVAILABLE);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    locatorAbility->GetAddressByCoordinate(data, reply);
    return ERRCODE_SUCCESS;
}
#endif

#ifdef FEATURE_GEOCODE_SUPPORT
int LocatorAbilityStub::PreGetAddressByLocationName(MessageParcel &data, MessageParcel &reply, AppIdentity &identity)
{
    auto locatorAbility = DelayedSingleton<LocatorAbility>::GetInstance();
    if (locatorAbility == nullptr) {
        LBSLOGE(LOCATOR, "PreGetAddressByLocationName: LocatorAbility is nullptr.");
        reply.WriteInt32(ERRCODE_SERVICE_UNAVAILABLE);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    locatorAbility->GetAddressByLocationName(data, reply);
    return ERRCODE_SUCCESS;
}
#endif

int LocatorAbilityStub::PreUnregisterSwitchCallback(MessageParcel &data, MessageParcel &reply, AppIdentity &identity)
{
    auto locatorAbility = DelayedSingleton<LocatorAbility>::GetInstance();
    if (locatorAbility == nullptr) {
        LBSLOGE(LOCATOR, "PreUnregisterSwitchCallback: LocatorAbility is nullptr.");
        reply.WriteInt32(ERRCODE_SERVICE_UNAVAILABLE);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    sptr<IRemoteObject> client = data.ReadObject<IRemoteObject>();
    reply.WriteInt32(locatorAbility->UnregisterSwitchCallback(client));
    return ERRCODE_SUCCESS;
}

#ifdef FEATURE_GNSS_SUPPORT
int LocatorAbilityStub::PreRegisterGnssStatusCallback(MessageParcel &data, MessageParcel &reply, AppIdentity &identity)
{
    if (!CheckLocationPermission(reply, identity)) {
        return ERRCODE_PERMISSION_DENIED;
    }
    auto locatorAbility = DelayedSingleton<LocatorAbility>::GetInstance();
    if (locatorAbility == nullptr) {
        LBSLOGE(LOCATOR, "PreRegisterGnssStatusCallback: LocatorAbility is nullptr.");
        reply.WriteInt32(ERRCODE_SERVICE_UNAVAILABLE);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    sptr<IRemoteObject> client = data.ReadObject<IRemoteObject>();
    reply.WriteInt32(locatorAbility->RegisterGnssStatusCallback(client, identity.GetUid()));
    return ERRCODE_SUCCESS;
}
#endif

#ifdef FEATURE_GNSS_SUPPORT
int LocatorAbilityStub::PreUnregisterGnssStatusCallback(MessageParcel &data,
    MessageParcel &reply, AppIdentity &identity)
{
    if (!CheckLocationPermission(reply, identity)) {
        return ERRCODE_PERMISSION_DENIED;
    }
    auto locatorAbility = DelayedSingleton<LocatorAbility>::GetInstance();
    if (locatorAbility == nullptr) {
        LBSLOGE(LOCATOR, "PreUnregisterGnssStatusCallback: LocatorAbility is nullptr.");
        reply.WriteInt32(ERRCODE_SERVICE_UNAVAILABLE);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    sptr<IRemoteObject> client = data.ReadObject<IRemoteObject>();
    reply.WriteInt32(locatorAbility->UnregisterGnssStatusCallback(client));
    return ERRCODE_SUCCESS;
}
#endif

#ifdef FEATURE_GNSS_SUPPORT
int LocatorAbilityStub::PreRegisterNmeaMessageCallback(MessageParcel &data,
    MessageParcel &reply, AppIdentity &identity)
{
    if (!CheckLocationPermission(reply, identity)) {
        return ERRCODE_PERMISSION_DENIED;
    }
    auto locatorAbility = DelayedSingleton<LocatorAbility>::GetInstance();
    if (locatorAbility == nullptr) {
        LBSLOGE(LOCATOR, "PreRegisterNmeaMessageCallback: LocatorAbility is nullptr.");
        reply.WriteInt32(ERRCODE_SERVICE_UNAVAILABLE);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    sptr<IRemoteObject> client = data.ReadObject<IRemoteObject>();
    reply.WriteInt32(locatorAbility->RegisterNmeaMessageCallback(client, identity.GetUid()));
    return ERRCODE_SUCCESS;
}
#endif

#ifdef FEATURE_GNSS_SUPPORT
int LocatorAbilityStub::PreUnregisterNmeaMessageCallback(MessageParcel &data,
    MessageParcel &reply, AppIdentity &identity)
{
    if (!CheckLocationPermission(reply, identity)) {
        return ERRCODE_PERMISSION_DENIED;
    }
    auto locatorAbility = DelayedSingleton<LocatorAbility>::GetInstance();
    if (locatorAbility == nullptr) {
        LBSLOGE(LOCATOR, "PreUnregisterNmeaMessageCallback: LocatorAbility is nullptr.");
        reply.WriteInt32(ERRCODE_SERVICE_UNAVAILABLE);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    sptr<IRemoteObject> client = data.ReadObject<IRemoteObject>();
    reply.WriteInt32(locatorAbility->UnregisterNmeaMessageCallback(client));
    return ERRCODE_SUCCESS;
}
#endif

#ifdef FEATURE_GNSS_SUPPORT
int LocatorAbilityStub::PreRegisterNmeaMessageCallbackV9(MessageParcel &data,
    MessageParcel &reply, AppIdentity &identity)
{
    if (!CheckPreciseLocationPermissions(reply, identity)) {
        return ERRCODE_PERMISSION_DENIED;
    }
    sptr<IRemoteObject> client = data.ReadObject<IRemoteObject>();
    auto locatorAbility = DelayedSingleton<LocatorAbility>::GetInstance();
    if (locatorAbility == nullptr) {
        LBSLOGE(LOCATOR, "PreRegisterNmeaMessageCallbackV9: LocatorAbility is nullptr.");
        reply.WriteInt32(ERRCODE_SERVICE_UNAVAILABLE);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    reply.WriteInt32(locatorAbility->RegisterNmeaMessageCallback(client, identity.GetUid()));
    return ERRCODE_SUCCESS;
}
#endif

#ifdef FEATURE_GNSS_SUPPORT
int LocatorAbilityStub::PreUnregisterNmeaMessageCallbackV9(MessageParcel &data,
    MessageParcel &reply, AppIdentity &identity)
{
    if (!CheckPreciseLocationPermissions(reply, identity)) {
        return ERRCODE_PERMISSION_DENIED;
    }
    sptr<IRemoteObject> client = data.ReadObject<IRemoteObject>();
    auto locatorAbility = DelayedSingleton<LocatorAbility>::GetInstance();
    if (locatorAbility == nullptr) {
        LBSLOGE(LOCATOR, "PreUnregisterNmeaMessageCallbackV9: LocatorAbility is nullptr.");
        reply.WriteInt32(ERRCODE_SERVICE_UNAVAILABLE);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    reply.WriteInt32(locatorAbility->UnregisterNmeaMessageCallback(client));
    return ERRCODE_SUCCESS;
}
#endif

int LocatorAbilityStub::PreIsLocationPrivacyConfirmed(MessageParcel &data, MessageParcel &reply, AppIdentity &identity)
{
    if (!CommonUtils::CheckSystemPermission(identity.GetTokenId(), identity.GetTokenIdEx())) {
        LBSLOGE(LOCATOR, "CheckSystemPermission return false, [%{public}s]",
            identity.ToString().c_str());
        reply.WriteInt32(ERRCODE_SYSTEM_PERMISSION_DENIED);
        return ERRCODE_SYSTEM_PERMISSION_DENIED;
    }
    auto locatorAbility = DelayedSingleton<LocatorAbility>::GetInstance();
    if (locatorAbility == nullptr) {
        LBSLOGE(LOCATOR, "PreIsLocationPrivacyConfirmed: LocatorAbility is nullptr.");
        reply.WriteInt32(ERRCODE_SERVICE_UNAVAILABLE);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    bool isConfirmed = false;
    reply.WriteInt32(locatorAbility->IsLocationPrivacyConfirmed(data.ReadInt32(), isConfirmed));
    reply.WriteBool(isConfirmed);
    return ERRCODE_SUCCESS;
}

int LocatorAbilityStub::PreSetLocationPrivacyConfirmStatus(MessageParcel &data,
    MessageParcel &reply, AppIdentity &identity)
{
    if (!CommonUtils::CheckSystemPermission(identity.GetTokenId(), identity.GetTokenIdEx())) {
        LBSLOGE(LOCATOR, "CheckSystemPermission return false, [%{public}s]",
            identity.ToString().c_str());
        reply.WriteInt32(ERRCODE_SYSTEM_PERMISSION_DENIED);
        return ERRCODE_SYSTEM_PERMISSION_DENIED;
    }
    if (!CheckSettingsPermission(reply, identity)) {
        return ERRCODE_PERMISSION_DENIED;
    }

    auto locatorAbility = DelayedSingleton<LocatorAbility>::GetInstance();
    if (locatorAbility == nullptr) {
        LBSLOGE(LOCATOR, "PreSetLocationPrivacyConfirmStatus: LocatorAbility is nullptr.");
        reply.WriteInt32(ERRCODE_SERVICE_UNAVAILABLE);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    reply.WriteInt32(locatorAbility->SetLocationPrivacyConfirmStatus(data.ReadInt32(),
        data.ReadBool()));
    return ERRCODE_SUCCESS;
}

#ifdef FEATURE_GNSS_SUPPORT
int LocatorAbilityStub::PreStartCacheLocating(MessageParcel &data, MessageParcel &reply, AppIdentity &identity)
{
    if (!CheckLocationPermission(reply, identity)) {
        return ERRCODE_PERMISSION_DENIED;
    }
    auto locatorAbility = DelayedSingleton<LocatorAbility>::GetInstance();
    if (locatorAbility == nullptr) {
        LBSLOGE(LOCATOR, "PreStartCacheLocating: LocatorAbility is nullptr.");
        reply.WriteInt32(ERRCODE_SERVICE_UNAVAILABLE);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    if (!CheckLocationSwitchState(reply)) {
        return ERRCODE_SWITCH_OFF;
    }
    std::unique_ptr<CachedGnssLocationsRequest> requestConfig = std::make_unique<CachedGnssLocationsRequest>();
    requestConfig->reportingPeriodSec = data.ReadInt32();
    requestConfig->wakeUpCacheQueueFull = data.ReadBool();
    sptr<IRemoteObject> remoteObject = data.ReadRemoteObject();
    std::string bundleName = Str16ToStr8(data.ReadString16());
    if (remoteObject == nullptr) {
        LBSLOGE(LOCATOR, "ParseDataAndStartCacheLocating remote object nullptr");
        reply.WriteInt32(ERRCODE_SERVICE_UNAVAILABLE);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    if (bundleName.empty()) {
        LBSLOGE(LOCATOR, "ParseDataAndStartCacheLocating get empty bundle name");
        reply.WriteInt32(ERRCODE_INVALID_PARAM);
        return ERRCODE_INVALID_PARAM;
    }
    sptr<ICachedLocationsCallback> callback = iface_cast<ICachedLocationsCallback>(remoteObject);
    reply.WriteInt32(locatorAbility->RegisterCachedLocationCallback(requestConfig, callback, bundleName));
    return ERRCODE_SUCCESS;
}
#endif

#ifdef FEATURE_GNSS_SUPPORT
int LocatorAbilityStub::PreStopCacheLocating(MessageParcel &data, MessageParcel &reply, AppIdentity &identity)
{
    if (!CheckLocationPermission(reply, identity)) {
        return ERRCODE_PERMISSION_DENIED;
    }
    auto locatorAbility = DelayedSingleton<LocatorAbility>::GetInstance();
    if (locatorAbility == nullptr) {
        LBSLOGE(LOCATOR, "PreStopCacheLocating: LocatorAbility is nullptr.");
        reply.WriteInt32(ERRCODE_SERVICE_UNAVAILABLE);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    sptr<IRemoteObject> remoteObject = data.ReadRemoteObject();
    if (remoteObject == nullptr) {
        LBSLOGE(LOCATOR, "LocatorAbility::ParseDataAndStopCacheLocating remote object nullptr");
        reply.WriteInt32(ERRCODE_SERVICE_UNAVAILABLE);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    sptr<ICachedLocationsCallback> callback = iface_cast<ICachedLocationsCallback>(remoteObject);
    reply.WriteInt32(locatorAbility->UnregisterCachedLocationCallback(callback));
    return ERRCODE_SUCCESS;
}
#endif

#ifdef FEATURE_GNSS_SUPPORT
int LocatorAbilityStub::PreGetCachedGnssLocationsSize(MessageParcel &data, MessageParcel &reply, AppIdentity &identity)
{
    if (!CheckLocationSwitchState(reply)) {
        return ERRCODE_SWITCH_OFF;
    }
    if (!CheckLocationPermission(reply, identity)) {
        return ERRCODE_PERMISSION_DENIED;
    }
    auto locatorAbility = DelayedSingleton<LocatorAbility>::GetInstance();
    if (locatorAbility == nullptr) {
        LBSLOGE(LOCATOR, "PreGetCachedGnssLocationsSize: LocatorAbility is nullptr.");
        reply.WriteInt32(ERRCODE_SERVICE_UNAVAILABLE);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    int size = -1;
    reply.WriteInt32(locatorAbility->GetCachedGnssLocationsSize(size));
    reply.WriteInt32(size);
    return ERRCODE_SUCCESS;
}
#endif

#ifdef FEATURE_GNSS_SUPPORT
int LocatorAbilityStub::PreFlushCachedGnssLocations(MessageParcel &data, MessageParcel &reply, AppIdentity &identity)
{
    if (!CheckLocationSwitchState(reply)) {
        return ERRCODE_SWITCH_OFF;
    }
    if (!CheckLocationPermission(reply, identity)) {
        return ERRCODE_PERMISSION_DENIED;
    }
    auto locatorAbility = DelayedSingleton<LocatorAbility>::GetInstance();
    if (locatorAbility == nullptr) {
        LBSLOGE(LOCATOR, "PreFlushCachedGnssLocations: LocatorAbility is nullptr.");
        reply.WriteInt32(ERRCODE_SERVICE_UNAVAILABLE);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    reply.WriteInt32(locatorAbility->FlushCachedGnssLocations());
    return ERRCODE_SUCCESS;
}
#endif

#ifdef FEATURE_GNSS_SUPPORT
int LocatorAbilityStub::PreSendCommand(MessageParcel &data, MessageParcel &reply, AppIdentity &identity)
{
    auto locatorAbility = DelayedSingleton<LocatorAbility>::GetInstance();
    if (locatorAbility == nullptr) {
        LBSLOGE(LOCATOR, "PreSendCommand: LocatorAbility is nullptr.");
        reply.WriteInt32(ERRCODE_SERVICE_UNAVAILABLE);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    std::unique_ptr<LocationCommand> locationCommand = std::make_unique<LocationCommand>();
    locationCommand->scenario =  data.ReadInt32();
    locationCommand->command = data.ReadBool();
    reply.WriteInt32(locatorAbility->SendCommand(locationCommand));
    return ERRCODE_SUCCESS;
}
#endif

#ifdef FEATURE_GNSS_SUPPORT
int LocatorAbilityStub::PreAddFence(MessageParcel &data, MessageParcel &reply, AppIdentity &identity)
{
    if (!CheckLocationSwitchState(reply)) {
        return ERRCODE_SWITCH_OFF;
    }
    if (!CheckLocationPermission(reply, identity)) {
        return ERRCODE_PERMISSION_DENIED;
    }
    auto locatorAbility = DelayedSingleton<LocatorAbility>::GetInstance();
    if (locatorAbility == nullptr) {
        LBSLOGE(LOCATOR, "PreAddFence: LocatorAbility is nullptr.");
        reply.WriteInt32(ERRCODE_SERVICE_UNAVAILABLE);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    std::unique_ptr<GeofenceRequest> request = std::make_unique<GeofenceRequest>();
    request->scenario = data.ReadInt32();
    request->geofence.latitude = data.ReadDouble();
    request->geofence.longitude = data.ReadDouble();
    request->geofence.radius = data.ReadDouble();
    request->geofence.expiration = data.ReadDouble();
    reply.WriteInt32(locatorAbility->AddFence(request));
    return ERRCODE_SUCCESS;
}
#endif

#ifdef FEATURE_GNSS_SUPPORT
int LocatorAbilityStub::PreRemoveFence(MessageParcel &data, MessageParcel &reply, AppIdentity &identity)
{
    if (!CheckLocationSwitchState(reply)) {
        return ERRCODE_SWITCH_OFF;
    }
    if (!CheckLocationPermission(reply, identity)) {
        return ERRCODE_PERMISSION_DENIED;
    }
    auto locatorAbility = DelayedSingleton<LocatorAbility>::GetInstance();
    if (locatorAbility == nullptr) {
        LBSLOGE(LOCATOR, "PreRemoveFence: LocatorAbility is nullptr.");
        reply.WriteInt32(ERRCODE_SERVICE_UNAVAILABLE);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    std::unique_ptr<GeofenceRequest> request = std::make_unique<GeofenceRequest>();
    request->scenario = data.ReadInt32();
    request->geofence.latitude = data.ReadDouble();
    request->geofence.longitude = data.ReadDouble();
    request->geofence.radius = data.ReadDouble();
    request->geofence.expiration = data.ReadDouble();
    reply.WriteInt32(locatorAbility->RemoveFence(request));
    return ERRCODE_SUCCESS;
}
#endif

int LocatorAbilityStub::PreGetIsoCountryCode(MessageParcel &data, MessageParcel &reply, AppIdentity &identity)
{
    auto locatorAbility = DelayedSingleton<LocatorAbility>::GetInstance();
    if (locatorAbility == nullptr) {
        LBSLOGE(LOCATOR, "PreGetIsoCountryCode: LocatorAbility is nullptr.");
        reply.WriteInt32(ERRCODE_SERVICE_UNAVAILABLE);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    std::shared_ptr<CountryCode> country;
    reply.WriteInt32(locatorAbility->GetIsoCountryCode(country));
    if (country) {
        reply.WriteString16(Str8ToStr16(country->GetCountryCodeStr()));
        reply.WriteInt32(country->GetCountryCodeType());
    } else {
        reply.WriteString16(Str8ToStr16(""));
        reply.WriteInt32(COUNTRY_CODE_FROM_LOCALE);
    }
    return ERRCODE_SUCCESS;
}

int LocatorAbilityStub::PreEnableLocationMock(MessageParcel &data, MessageParcel &reply, AppIdentity &identity)
{
    if (!CommonUtils::CheckSystemPermission(identity.GetTokenId(), identity.GetTokenIdEx())) {
        LBSLOGE(LOCATOR, "CheckSystemPermission return false, [%{public}s]",
            identity.ToString().c_str());
        reply.WriteInt32(ERRCODE_SYSTEM_PERMISSION_DENIED);
        return ERRCODE_SYSTEM_PERMISSION_DENIED;
    }
    auto locatorAbility = DelayedSingleton<LocatorAbility>::GetInstance();
    if (locatorAbility == nullptr) {
        LBSLOGE(LOCATOR, "PreEnableLocationMock: LocatorAbility is nullptr.");
        reply.WriteInt32(ERRCODE_SERVICE_UNAVAILABLE);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    reply.WriteInt32(locatorAbility->EnableLocationMock());
    return ERRCODE_SUCCESS;
}

int LocatorAbilityStub::PreDisableLocationMock(MessageParcel &data, MessageParcel &reply, AppIdentity &identity)
{
    if (!CommonUtils::CheckSystemPermission(identity.GetTokenId(), identity.GetTokenIdEx())) {
        LBSLOGE(LOCATOR, "CheckSystemPermission return false, [%{public}s]",
            identity.ToString().c_str());
        reply.WriteInt32(ERRCODE_SYSTEM_PERMISSION_DENIED);
        return ERRCODE_SYSTEM_PERMISSION_DENIED;
    }
    auto locatorAbility = DelayedSingleton<LocatorAbility>::GetInstance();
    if (locatorAbility == nullptr) {
        LBSLOGE(LOCATOR, "PreDisableLocationMock: LocatorAbility is nullptr.");
        reply.WriteInt32(ERRCODE_SERVICE_UNAVAILABLE);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    reply.WriteInt32(locatorAbility->DisableLocationMock());
    return ERRCODE_SUCCESS;
}

int LocatorAbilityStub::PreSetMockedLocations(MessageParcel &data, MessageParcel &reply, AppIdentity &identity)
{
    if (!CommonUtils::CheckSystemPermission(identity.GetTokenId(), identity.GetTokenIdEx())) {
        LBSLOGE(LOCATOR, "CheckSystemPermission return false, [%{public}s]",
            identity.ToString().c_str());
        reply.WriteInt32(ERRCODE_SYSTEM_PERMISSION_DENIED);
        return ERRCODE_SYSTEM_PERMISSION_DENIED;
    }
    auto locatorAbility = DelayedSingleton<LocatorAbility>::GetInstance();
    if (locatorAbility == nullptr) {
        LBSLOGE(LOCATOR, "PreSetMockedLocations: LocatorAbility is nullptr.");
        reply.WriteInt32(ERRCODE_SERVICE_UNAVAILABLE);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    int timeInterval = data.ReadInt32();
    int locationSize = data.ReadInt32();
    locationSize = locationSize > INPUT_ARRAY_LEN_MAX ? INPUT_ARRAY_LEN_MAX : locationSize;
    std::vector<std::shared_ptr<Location>> vcLoc;
    for (int i = 0; i < locationSize; i++) {
        vcLoc.push_back(Location::UnmarshallingShared(data));
    }
    reply.WriteInt32(locatorAbility->SetMockedLocations(timeInterval, vcLoc));
    return ERRCODE_SUCCESS;
}

#ifdef FEATURE_GEOCODE_SUPPORT
int LocatorAbilityStub::PreEnableReverseGeocodingMock(MessageParcel &data, MessageParcel &reply, AppIdentity &identity)
{
    if (!CommonUtils::CheckSystemPermission(identity.GetTokenId(), identity.GetTokenIdEx())) {
        LBSLOGE(LOCATOR, "CheckSystemPermission return false, [%{public}s]",
            identity.ToString().c_str());
        reply.WriteInt32(ERRCODE_SYSTEM_PERMISSION_DENIED);
        return ERRCODE_SYSTEM_PERMISSION_DENIED;
    }
    auto locatorAbility = DelayedSingleton<LocatorAbility>::GetInstance();
    if (locatorAbility == nullptr) {
        LBSLOGE(LOCATOR, "PreEnableReverseGeocodingMock: LocatorAbility is nullptr.");
        reply.WriteInt32(ERRCODE_SERVICE_UNAVAILABLE);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    reply.WriteInt32(locatorAbility->EnableReverseGeocodingMock());
    return ERRCODE_SUCCESS;
}
#endif

#ifdef FEATURE_GEOCODE_SUPPORT
int LocatorAbilityStub::PreDisableReverseGeocodingMock(MessageParcel &data,
    MessageParcel &reply, AppIdentity &identity)
{
    if (!CommonUtils::CheckSystemPermission(identity.GetTokenId(), identity.GetTokenIdEx())) {
        LBSLOGE(LOCATOR, "CheckSystemPermission return false, [%{public}s]",
            identity.ToString().c_str());
        reply.WriteInt32(ERRCODE_SYSTEM_PERMISSION_DENIED);
        return ERRCODE_SYSTEM_PERMISSION_DENIED;
    }
    auto locatorAbility = DelayedSingleton<LocatorAbility>::GetInstance();
    if (locatorAbility == nullptr) {
        LBSLOGE(LOCATOR, "PreDisableReverseGeocodingMock: LocatorAbility is nullptr.");
        reply.WriteInt32(ERRCODE_SERVICE_UNAVAILABLE);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    reply.WriteInt32(locatorAbility->DisableReverseGeocodingMock());
    return ERRCODE_SUCCESS;
}
#endif

#ifdef FEATURE_GEOCODE_SUPPORT
int LocatorAbilityStub::PreSetReverseGeocodingMockInfo(MessageParcel &data,
    MessageParcel &reply, AppIdentity &identity)
{
    if (!CommonUtils::CheckSystemPermission(identity.GetTokenId(), identity.GetTokenIdEx())) {
        LBSLOGE(LOCATOR, "CheckSystemPermission return false, [%{public}s]",
            identity.ToString().c_str());
        reply.WriteInt32(ERRCODE_SYSTEM_PERMISSION_DENIED);
        return ERRCODE_SYSTEM_PERMISSION_DENIED;
    }
    auto locatorAbility = DelayedSingleton<LocatorAbility>::GetInstance();
    if (locatorAbility == nullptr) {
        LBSLOGE(LOCATOR, "PreSetReverseGeocodingMockInfo: LocatorAbility is nullptr.");
        reply.WriteInt32(ERRCODE_SERVICE_UNAVAILABLE);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    std::vector<std::shared_ptr<GeocodingMockInfo>> mockInfo;
    int arraySize = data.ReadInt32();
    arraySize = arraySize > INPUT_ARRAY_LEN_MAX ? INPUT_ARRAY_LEN_MAX : arraySize;
    for (int i = 0; i < arraySize; i++) {
        std::shared_ptr<GeocodingMockInfo> info = std::make_shared<GeocodingMockInfo>();
        info->ReadFromParcel(data);
        mockInfo.push_back(info);
    }
    reply.WriteInt32(locatorAbility->SetReverseGeocodingMockInfo(mockInfo));
    return ERRCODE_SUCCESS;
}
#endif

int LocatorAbilityStub::PreRegisterCountryCodeCallback(MessageParcel &data,
    MessageParcel &reply, AppIdentity &identity)
{
    auto locatorAbility = DelayedSingleton<LocatorAbility>::GetInstance();
    if (locatorAbility == nullptr) {
        LBSLOGE(LOCATOR, "PreRegisterCountryCodeCallback: LocatorAbility is nullptr.");
        reply.WriteInt32(ERRCODE_SERVICE_UNAVAILABLE);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    sptr<IRemoteObject> client = data.ReadObject<IRemoteObject>();
    if (client == nullptr) {
        LBSLOGE(LOCATOR, "%{public}s: client is nullptr.", __func__);
        reply.WriteInt32(ERRCODE_SERVICE_UNAVAILABLE);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    sptr<IRemoteObject::DeathRecipient> death(new (std::nothrow) CountryCodeCallbackDeathRecipient());
    client->AddDeathRecipient(death);
    LocationErrCode errorCode = locatorAbility->RegisterCountryCodeCallback(client, identity.GetUid());
    reply.WriteInt32(errorCode);
    isCallbackReg_ = (errorCode == ERRCODE_SUCCESS) ? true : isCallbackReg_;
    return ERRCODE_SUCCESS;
}

int LocatorAbilityStub::PreUnregisterCountryCodeCallback(MessageParcel &data,
    MessageParcel &reply, AppIdentity &identity)
{
    auto locatorAbility = DelayedSingleton<LocatorAbility>::GetInstance();
    if (locatorAbility == nullptr) {
        LBSLOGE(LOCATOR, "PreUnregisterCountryCodeCallback: LocatorAbility is nullptr.");
        reply.WriteInt32(ERRCODE_SERVICE_UNAVAILABLE);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    sptr<IRemoteObject> client = data.ReadObject<IRemoteObject>();
    LocationErrCode errorCode = locatorAbility->UnregisterCountryCodeCallback(client);
    reply.WriteInt32(errorCode);
    isCallbackReg_ = (errorCode == ERRCODE_SUCCESS) ? false : isCallbackReg_;
    return ERRCODE_SUCCESS;
}

int LocatorAbilityStub::PreProxyUidForFreeze(MessageParcel &data, MessageParcel &reply, AppIdentity &identity)
{
    if (!CheckLocationPermission(reply, identity)) {
        return ERRCODE_PERMISSION_DENIED;
    }
    auto locatorAbility = DelayedSingleton<LocatorAbility>::GetInstance();
    if (locatorAbility == nullptr) {
        LBSLOGE(LOCATOR, "PreProxyUidForFreeze: LocatorAbility is nullptr.");
        reply.WriteInt32(ERRCODE_SERVICE_UNAVAILABLE);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    if (identity.GetUid() != ROOT_UID) {
        LBSLOGE(LOCATOR, "check root permission failed, [%{public}s]",
            identity.ToString().c_str());
        reply.WriteInt32(ERRCODE_PERMISSION_DENIED);
        return ERRCODE_PERMISSION_DENIED;
    }
    int32_t uid = data.ReadInt32();
    bool isProxy = data.ReadBool();
    reply.WriteInt32(locatorAbility->ProxyUidForFreeze(uid, isProxy));
    return ERRCODE_SUCCESS;
}

int LocatorAbilityStub::PreResetAllProxy(MessageParcel &data, MessageParcel &reply, AppIdentity &identity)
{
    if (!CheckLocationPermission(reply, identity)) {
        return ERRCODE_PERMISSION_DENIED;
    }
    auto locatorAbility = DelayedSingleton<LocatorAbility>::GetInstance();
    if (locatorAbility == nullptr) {
        LBSLOGE(LOCATOR, "PreResetAllProxy: LocatorAbility is nullptr.");
        reply.WriteInt32(ERRCODE_SERVICE_UNAVAILABLE);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    if (identity.GetUid() != ROOT_UID) {
        LBSLOGE(LOCATOR, "check root permission failed, [%{public}s]",
            identity.ToString().c_str());
        reply.WriteInt32(ERRCODE_PERMISSION_DENIED);
        return ERRCODE_PERMISSION_DENIED;
    }
    reply.WriteInt32(locatorAbility->ResetAllProxy());
    return ERRCODE_SUCCESS;
}

int LocatorAbilityStub::PreReportLocation(MessageParcel &data, MessageParcel &reply, AppIdentity &identity)
{
    pid_t callingPid = IPCSkeleton::GetCallingPid();
    pid_t callingUid = IPCSkeleton::GetCallingUid();
    if (callingUid != static_cast<pid_t>(getuid()) || callingPid != getpid()) {
        LBSLOGE(LOCATOR, "check system permission failed, [%{public}s]",
            identity.ToString().c_str());
        return ERRCODE_PERMISSION_DENIED;
    }
    auto locatorAbility = DelayedSingleton<LocatorAbility>::GetInstance();
    if (locatorAbility == nullptr) {
        LBSLOGE(LOCATOR, "PreReportLocation: LocatorAbility is nullptr.");
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    std::string systemAbility = data.ReadString();
    std::unique_ptr<Location> location = Location::Unmarshalling(data);
    locatorAbility->ReportLocation(location, systemAbility);
    return ERRCODE_SUCCESS;
}

bool LocatorAbilityStub::CheckLocationPermission(MessageParcel &reply, AppIdentity &identity)
{
    uint32_t callingTokenId = identity.GetTokenId();
    uint32_t callingFirstTokenid = identity.GetFirstTokenId();
    if (!CommonUtils::CheckLocationPermission(callingTokenId, callingFirstTokenid) &&
        !CommonUtils::CheckApproximatelyPermission(callingTokenId, callingFirstTokenid)) {
        LBSLOGE(LOCATOR, "CheckLocationPermission return false.");
        reply.WriteInt32(ERRCODE_PERMISSION_DENIED);
        return false;
    } else {
        return true;
    }
}

bool LocatorAbilityStub::CheckPreciseLocationPermissions(MessageParcel &reply, AppIdentity &identity)
{
    uint32_t callingTokenId = identity.GetTokenId();
    uint32_t callingFirstTokenid = identity.GetFirstTokenId();
    if (!CommonUtils::CheckLocationPermission(callingTokenId, callingFirstTokenid) ||
        !CommonUtils::CheckApproximatelyPermission(callingTokenId, callingFirstTokenid)) {
        LBSLOGE(LOCATOR, "CheckPreciseLocationPermissions return false.");
        reply.WriteInt32(ERRCODE_PERMISSION_DENIED);
        return false;
    } else {
        return true;
    }
}

bool LocatorAbilityStub::CheckSettingsPermission(MessageParcel &reply, AppIdentity &identity)
{
    uint32_t callingTokenId = identity.GetTokenId();
    uint32_t callingFirstTokenid = identity.GetFirstTokenId();
    if (!CommonUtils::CheckSecureSettings(callingTokenId, callingFirstTokenid)) {
        LBSLOGE(LOCATOR, "has no access permission, CheckSecureSettings return false");
        reply.WriteInt32(ERRCODE_PERMISSION_DENIED);
        return false;
    } else {
        return true;
    }
}

bool LocatorAbilityStub::CheckLocationSwitchState(MessageParcel &reply)
{
    auto locatorAbility = DelayedSingleton<LocatorAbility>::GetInstance();
    if (locatorAbility == nullptr) {
        LBSLOGE(LOCATOR, "CheckLocationSwitchState: LocatorAbility is nullptr.");
        return false;
    }
    int state = DISABLED;
    locatorAbility->GetSwitchState(state);
    if (state == DISABLED) {
        LBSLOGE(LOCATOR, "switch state is off.");
        reply.WriteInt32(ERRCODE_SWITCH_OFF);
        return false;
    }
    return true;
}

int32_t LocatorAbilityStub::OnRemoteRequest(uint32_t code,
    MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    int ret = ERRCODE_SUCCESS;
    pid_t callingPid = IPCSkeleton::GetCallingPid();
    pid_t callingUid = IPCSkeleton::GetCallingUid();
    uint32_t callingTokenId = IPCSkeleton::GetCallingTokenID();
    uint64_t callingTokenIdEx = IPCSkeleton::GetCallingFullTokenID();
    uint32_t callingFirstTokenid = IPCSkeleton::GetFirstTokenID();

    AppIdentity identity;
    identity.SetPid(callingPid);
    identity.SetUid(callingUid);
    identity.SetTokenId(callingTokenId);
    identity.SetTokenIdEx(callingTokenIdEx);
    identity.SetFirstTokenId(callingFirstTokenid);
    std::string bundleName = "";
    if (!CommonUtils::GetBundleNameByUid(callingUid, bundleName)) {
        LBSLOGD(LOCATOR, "Fail to Get bundle name: uid = %{public}d.", callingUid);
    }
    identity.SetBundleName(bundleName);
    LBSLOGI(LOCATOR, "OnReceived cmd = %{public}u, flags= %{public}d, identity= [%{public}s]",
        code, option.GetFlags(), identity.ToString().c_str());
    std::string callingIdentity = IPCSkeleton::ResetCallingIdentity();

    if (data.ReadInterfaceToken() != GetDescriptor()) {
        LBSLOGE(LOCATOR, "invalid token.");
        IPCSkeleton::SetCallingIdentity(callingIdentity);
        reply.WriteInt32(ERRCODE_SERVICE_UNAVAILABLE);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }

    auto handleFunc = locatorHandleMap_.find(code);
    if (handleFunc != locatorHandleMap_.end() && handleFunc->second != nullptr) {
        auto memberFunc = handleFunc->second;
        ret = (this->*memberFunc)(data, reply, identity);
    } else {
        LBSLOGE(LOCATOR, "OnReceived cmd = %{public}u, unsupport service.", code);
        reply.WriteInt32(ERRCODE_NOT_SUPPORTED);
        ret = ERRCODE_NOT_SUPPORTED;
    }
    IPCSkeleton::SetCallingIdentity(callingIdentity);
    UnloadLocatorSa();
    return ret;
}

void LocatorAbilityStub::SaDumpInfo(std::string& result)
{
    auto locatorAbility = DelayedSingleton<LocatorAbility>::GetInstance();
    if (locatorAbility == nullptr) {
        LBSLOGE(LOCATOR, "SaDumpInfo: LocatorAbility is nullptr.");
        return;
    }
    int state = DISABLED;
    locatorAbility->GetSwitchState(state);
    result += "Location switch state: ";
    std::string status = state ? "on" : "off";
    result += status + "\n";
}

int32_t LocatorAbilityStub::Dump(int32_t fd, const std::vector<std::u16string>& args)
{
    std::vector<std::string> vecArgs;
    std::transform(args.begin(), args.end(), std::back_inserter(vecArgs), [](const std::u16string &arg) {
        return Str16ToStr8(arg);
    });

    LocationDumper dumper;
    std::string result;
    dumper.LocatorDump(SaDumpInfo, vecArgs, result);
    if (!SaveStringToFd(fd, result)) {
        LBSLOGE(LOCATOR, "Gnss save string to fd failed!");
        return ERR_OK;
    }
    return ERR_OK;
}

bool LocatorAbilityStub::UnloadLocatorSa()
{
    if (!isCallbackReg_) {
        auto locatorAbility = DelayedSingleton<LocatorAbility>::GetInstance();
        if (locatorAbility == nullptr) {
            LBSLOGE(LOCATOR, "%{public}s: LocatorAbility is nullptr.", __func__);
            return false;
        }
        locatorAbility->UnloadSaAbility();
    }
    return true;
}

LocatorCallbackDeathRecipient::LocatorCallbackDeathRecipient()
{
}

LocatorCallbackDeathRecipient::~LocatorCallbackDeathRecipient()
{
}

void LocatorCallbackDeathRecipient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    sptr<ILocatorCallback> callback = iface_cast<ILocatorCallback>(remote.promote());
    auto locatorAbility = DelayedSingleton<LocatorAbility>::GetInstance();
    if (locatorAbility != nullptr) {
        locatorAbility->StopLocating(callback);
        LBSLOGI(LOCATOR, "locator callback OnRemoteDied");
    }
}

SwitchCallbackDeathRecipient::SwitchCallbackDeathRecipient()
{
}

SwitchCallbackDeathRecipient::~SwitchCallbackDeathRecipient()
{
}

void SwitchCallbackDeathRecipient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    auto locatorAbility = DelayedSingleton<LocatorAbility>::GetInstance();
    if (locatorAbility != nullptr) {
        locatorAbility->UnregisterSwitchCallback(remote.promote());
        LBSLOGI(LOCATOR, "switch callback OnRemoteDied");
    }
}

CountryCodeCallbackDeathRecipient::CountryCodeCallbackDeathRecipient()
{
}

CountryCodeCallbackDeathRecipient::~CountryCodeCallbackDeathRecipient()
{
}

void CountryCodeCallbackDeathRecipient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    auto locatorAbility = DelayedSingleton<LocatorAbility>::GetInstance();
    if (locatorAbility != nullptr) {
        locatorAbility->UnregisterCountryCodeCallback(remote.promote());
        LBSLOGI(LOCATOR, "countrycode callback OnRemoteDied");
    }
}
} // namespace Location
} // namespace OHOS