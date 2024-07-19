/*
 * Copyright (C) 2022-2024 Huawei Device Co., Ltd.
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
#include "common_hisysevent.h"
#include "location.h"
#include "locator_ability.h"
#include "location_dumper.h"
#include "location_config_manager.h"
#include "request_config.h"
#include "system_ability_definition.h"
#include "location_sa_load_manager.h"

#include "locating_required_data_config.h"
#include "locator_required_data_manager.h"
#include "location_log_event_ids.h"
#include "geofence_request.h"
#include "permission_manager.h"

namespace OHOS {
namespace Location {
void LocatorAbilityStub::InitLocatorHandleMap()
{
    if (locatorHandleMap_.size() != 0) {
        return;
    }
    ConstructLocatorHandleMap();
    ConstructLocatorEnhanceHandleMap();
    ConstructLocatorMockHandleMap();
    ConstructGeocodeHandleMap();
    ConstructGnssHandleMap();
    ConstructGnssEnhanceHandleMap();
}

void LocatorAbilityStub::ConstructLocatorHandleMap()
{
    locatorHandleMap_[LocatorInterfaceCode::GET_SWITCH_STATE] =
        [this](MessageParcel &data, MessageParcel &reply, AppIdentity &identity) {
        return PreGetSwitchState(data, reply, identity);
        };
    locatorHandleMap_[LocatorInterfaceCode::REG_SWITCH_CALLBACK] =
        [this](MessageParcel &data, MessageParcel &reply, AppIdentity &identity) {
        return PreRegisterSwitchCallback(data, reply, identity);
        };
    locatorHandleMap_[LocatorInterfaceCode::UNREG_SWITCH_CALLBACK] =
        [this](MessageParcel &data, MessageParcel &reply, AppIdentity &identity) {
        return PreUnregisterSwitchCallback(data, reply, identity);
        };
    locatorHandleMap_[LocatorInterfaceCode::START_LOCATING] =
        [this](MessageParcel &data, MessageParcel &reply, AppIdentity &identity) {
        return PreStartLocating(data, reply, identity);
        };
    locatorHandleMap_[LocatorInterfaceCode::STOP_LOCATING] =
        [this](MessageParcel &data, MessageParcel &reply, AppIdentity &identity) {
        return PreStopLocating(data, reply, identity);
        };
    locatorHandleMap_[LocatorInterfaceCode::GET_CACHE_LOCATION] =
        [this](MessageParcel &data, MessageParcel &reply, AppIdentity &identity) {
        return PreGetCacheLocation(data, reply, identity);
        };
    locatorHandleMap_[LocatorInterfaceCode::ENABLE_ABILITY] =
        [this](MessageParcel &data, MessageParcel &reply, AppIdentity &identity) {
        return PreEnableAbility(data, reply, identity);
        };
}

void LocatorAbilityStub::ConstructLocatorEnhanceHandleMap()
{
    locatorHandleMap_[LocatorInterfaceCode::UPDATE_SA_ABILITY] =
        [this](MessageParcel &data, MessageParcel &reply, AppIdentity &identity) {
        return PreUpdateSaAbility(data, reply, identity);
        };
    locatorHandleMap_[LocatorInterfaceCode::IS_PRIVACY_COMFIRMED] =
        [this](MessageParcel &data, MessageParcel &reply, AppIdentity &identity) {
        return PreIsLocationPrivacyConfirmed(data, reply, identity);
        };
    locatorHandleMap_[LocatorInterfaceCode::SET_PRIVACY_COMFIRM_STATUS] =
        [this](MessageParcel &data, MessageParcel &reply, AppIdentity &identity) {
        return PreSetLocationPrivacyConfirmStatus(data, reply, identity);
        };
    locatorHandleMap_[LocatorInterfaceCode::PROXY_PID_FOR_FREEZE] =
        [this](MessageParcel &data, MessageParcel &reply, AppIdentity &identity) {
        return PreProxyForFreeze(data, reply, identity);
        };
    locatorHandleMap_[LocatorInterfaceCode::RESET_ALL_PROXY] =
        [this](MessageParcel &data, MessageParcel &reply, AppIdentity &identity) {
        return PreResetAllProxy(data, reply, identity);
        };
    locatorHandleMap_[LocatorInterfaceCode::REPORT_LOCATION] =
        [this](MessageParcel &data, MessageParcel &reply, AppIdentity &identity) {
        return PreReportLocation(data, reply, identity);
        };
    locatorHandleMap_[LocatorInterfaceCode::REG_LOCATING_REQUIRED_DATA_CALLBACK] =
        [this](MessageParcel &data, MessageParcel &reply, AppIdentity &identity) {
        return PreRegisterLocatingRequiredDataCallback(data, reply, identity);
        };
    locatorHandleMap_[LocatorInterfaceCode::UNREG_LOCATING_REQUIRED_DATA_CALLBACK] =
        [this](MessageParcel &data, MessageParcel &reply, AppIdentity &identity) {
        return PreUnregisterLocatingRequiredDataCallback(data, reply, identity);
        };
    locatorHandleMap_[LocatorInterfaceCode::REG_LOCATION_ERROR] =
        [this](MessageParcel &data, MessageParcel &reply, AppIdentity &identity) {
        return PreRegisterLocationError(data, reply, identity);
        };
    locatorHandleMap_[LocatorInterfaceCode::UNREG_LOCATION_ERROR] =
        [this](MessageParcel &data, MessageParcel &reply, AppIdentity &identity) {
        return PreUnregisterLocationError(data, reply, identity);
        };
    locatorHandleMap_[LocatorInterfaceCode::REPORT_LOCATION_ERROR] =
        [this](MessageParcel &data, MessageParcel &reply, AppIdentity &identity) {
        return PreReportLocationError(data, reply, identity);
        };
}

void LocatorAbilityStub::ConstructLocatorMockHandleMap()
{
    locatorHandleMap_[LocatorInterfaceCode::ENABLE_LOCATION_MOCK] =
        [this](MessageParcel &data, MessageParcel &reply, AppIdentity &identity) {
        return PreEnableLocationMock(data, reply, identity);
        };
    locatorHandleMap_[LocatorInterfaceCode::DISABLE_LOCATION_MOCK] =
        [this](MessageParcel &data, MessageParcel &reply, AppIdentity &identity) {
        return PreDisableLocationMock(data, reply, identity);
        };
    locatorHandleMap_[LocatorInterfaceCode::SET_MOCKED_LOCATIONS] =
        [this](MessageParcel &data, MessageParcel &reply, AppIdentity &identity) {
        return PreSetMockedLocations(data, reply, identity);
        };
}

void LocatorAbilityStub::ConstructGeocodeHandleMap()
{
#ifdef FEATURE_GEOCODE_SUPPORT
    locatorHandleMap_[LocatorInterfaceCode::GEO_IS_AVAILABLE] =
        [this](MessageParcel &data, MessageParcel &reply, AppIdentity &identity) {
        return PreIsGeoConvertAvailable(data, reply, identity);
        };
    locatorHandleMap_[LocatorInterfaceCode::GET_FROM_COORDINATE] =
        [this](MessageParcel &data, MessageParcel &reply, AppIdentity &identity) {
        return PreGetAddressByCoordinate(data, reply, identity);
            };
    locatorHandleMap_[LocatorInterfaceCode::GET_FROM_LOCATION_NAME] =
        [this](MessageParcel &data, MessageParcel &reply, AppIdentity &identity) {
        return PreGetAddressByLocationName(data, reply, identity);
    };
    locatorHandleMap_[LocatorInterfaceCode::ENABLE_REVERSE_GEOCODE_MOCK] =
        [this](MessageParcel &data, MessageParcel &reply, AppIdentity &identity) {
        return PreEnableReverseGeocodingMock(data, reply, identity);
        };
    locatorHandleMap_[LocatorInterfaceCode::DISABLE_REVERSE_GEOCODE_MOCK] =
        [this](MessageParcel &data, MessageParcel &reply, AppIdentity &identity) {
        return PreDisableReverseGeocodingMock(data, reply, identity);
        };
    locatorHandleMap_[LocatorInterfaceCode::SET_REVERSE_GEOCODE_MOCKINFO] =
        [this](MessageParcel &data, MessageParcel &reply, AppIdentity &identity) {
        return PreSetReverseGeocodingMockInfo(data, reply, identity);
        };
#endif
}

void LocatorAbilityStub::ConstructGnssHandleMap()
{
#ifdef FEATURE_GNSS_SUPPORT
    locatorHandleMap_[LocatorInterfaceCode::REG_GNSS_STATUS_CALLBACK] =
        [this](MessageParcel &data, MessageParcel &reply, AppIdentity &identity) {
        return PreRegisterGnssStatusCallback(data, reply, identity);
        };
    locatorHandleMap_[LocatorInterfaceCode::UNREG_GNSS_STATUS_CALLBACK] =
        [this](MessageParcel &data, MessageParcel &reply, AppIdentity &identity) {
        return PreUnregisterGnssStatusCallback(data, reply, identity);
        };
    locatorHandleMap_[LocatorInterfaceCode::REG_NMEA_CALLBACK] =
        [this](MessageParcel &data, MessageParcel &reply, AppIdentity &identity) {
        return PreRegisterNmeaMessageCallback(data, reply, identity);
        };
    locatorHandleMap_[LocatorInterfaceCode::UNREG_NMEA_CALLBACK] =
        [this](MessageParcel &data, MessageParcel &reply, AppIdentity &identity) {
        return PreUnregisterNmeaMessageCallback(data, reply, identity);
        };
    locatorHandleMap_[LocatorInterfaceCode::REG_CACHED_CALLBACK] =
        [this](MessageParcel &data, MessageParcel &reply, AppIdentity &identity) {
        return PreStartCacheLocating(data, reply, identity);
        };
    locatorHandleMap_[LocatorInterfaceCode::UNREG_CACHED_CALLBACK] =
        [this](MessageParcel &data, MessageParcel &reply, AppIdentity &identity) {
        return PreStopCacheLocating(data, reply, identity);
        };
    locatorHandleMap_[LocatorInterfaceCode::GET_CACHED_LOCATION_SIZE] =
        [this](MessageParcel &data, MessageParcel &reply, AppIdentity &identity) {
        return PreGetCachedGnssLocationsSize(data, reply, identity);
        };
    locatorHandleMap_[LocatorInterfaceCode::FLUSH_CACHED_LOCATIONS] =
        [this](MessageParcel &data, MessageParcel &reply, AppIdentity &identity) {
        return PreFlushCachedGnssLocations(data, reply, identity);
        };
    locatorHandleMap_[LocatorInterfaceCode::SEND_COMMAND] =
        [this](MessageParcel &data, MessageParcel &reply, AppIdentity &identity) {
        return PreSendCommand(data, reply, identity);
        };
    locatorHandleMap_[LocatorInterfaceCode::REG_NMEA_CALLBACK_V9] =
        [this](MessageParcel &data, MessageParcel &reply, AppIdentity &identity) {
        return PreRegisterNmeaMessageCallbackV9(data, reply, identity);
        };
    locatorHandleMap_[LocatorInterfaceCode::UNREG_NMEA_CALLBACK_V9] =
        [this](MessageParcel &data, MessageParcel &reply, AppIdentity &identity) {
        return PreUnregisterNmeaMessageCallbackV9(data, reply, identity);
        };
#endif
}

void LocatorAbilityStub::ConstructGnssEnhanceHandleMap()
{
#ifdef FEATURE_GNSS_SUPPORT
    locatorHandleMap_[LocatorInterfaceCode::ADD_FENCE] =
        [this](MessageParcel &data, MessageParcel &reply, AppIdentity &identity) {
        return PreAddFence(data, reply, identity);
        };
    locatorHandleMap_[LocatorInterfaceCode::REMOVE_FENCE] =
        [this](MessageParcel &data, MessageParcel &reply, AppIdentity &identity) {
        return PreRemoveFence(data, reply, identity);
        };
    locatorHandleMap_[LocatorInterfaceCode::ADD_GNSS_GEOFENCE] =
        [this](MessageParcel &data, MessageParcel &reply, AppIdentity &identity) {
        return PreAddGnssGeofence(data, reply, identity);
        };
    locatorHandleMap_[LocatorInterfaceCode::REMOVE_GNSS_GEOFENCE] =
        [this](MessageParcel &data, MessageParcel &reply, AppIdentity &identity) {
        return PreRemoveGnssGeofence(data, reply, identity);
        };
    locatorHandleMap_[LocatorInterfaceCode::GET_GEOFENCE_SUPPORT_COORDINATE_SYSTEM_TYPE] =
        [this](MessageParcel &data, MessageParcel &reply, AppIdentity &identity) {
        return PreQuerySupportCoordinateSystemType(data, reply, identity);
        };
#endif
}

LocatorAbilityStub::LocatorAbilityStub()
{
    InitLocatorHandleMap();
}

int LocatorAbilityStub::PreGetSwitchState(MessageParcel &data, MessageParcel &reply, AppIdentity &identity)
{
    auto locatorAbility = LocatorAbility::GetInstance();
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
    auto locatorAbility = LocatorAbility::GetInstance();
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
    auto reportManager = ReportManager::GetInstance();
    if (reportManager != nullptr) {
        if (reportManager->IsAppBackground(identity.GetBundleName(), identity.GetTokenId(),
            identity.GetTokenIdEx(), identity.GetUid()) &&
            !PermissionManager::CheckBackgroundPermission(identity.GetTokenId(), identity.GetFirstTokenId())) {
            reply.WriteInt32(ERRCODE_PERMISSION_DENIED);
            return ERRCODE_PERMISSION_DENIED;
        }
    }
    auto locatorAbility = LocatorAbility::GetInstance();
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

    sptr<ILocatorCallback> callback = iface_cast<ILocatorCallback>(remoteObject);
    reply.WriteInt32(locatorAbility->StartLocating(requestConfig, callback, identity));
    return ERRCODE_SUCCESS;
}

int LocatorAbilityStub::PreStopLocating(MessageParcel &data, MessageParcel &reply, AppIdentity &identity)
{
    if (!CheckLocationPermission(reply, identity)) {
        return ERRCODE_PERMISSION_DENIED;
    }
    auto locatorAbility = LocatorAbility::GetInstance();
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
    auto locatorAbility = LocatorAbility::GetInstance();
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
    if (!PermissionManager::CheckSystemPermission(identity.GetTokenId(), identity.GetTokenIdEx())) {
        LBSLOGE(LOCATOR, "CheckSystemPermission return false, [%{public}s]",
            identity.ToString().c_str());
        reply.WriteInt32(ERRCODE_SYSTEM_PERMISSION_DENIED);
        return ERRCODE_SYSTEM_PERMISSION_DENIED;
    }
    if (!CheckSettingsPermission(reply, identity)) {
        return ERRCODE_PERMISSION_DENIED;
    }
    auto locatorAbility = LocatorAbility::GetInstance();
    if (locatorAbility == nullptr) {
        LBSLOGE(LOCATOR, "PreEnableAbility: LocatorAbility is nullptr.");
        reply.WriteInt32(ERRCODE_SERVICE_UNAVAILABLE);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    bool isEnabled = data.ReadBool();
    bool privacyState = false;
    LocationErrCode code =
        LocationConfigManager::GetInstance()->GetPrivacyTypeState(PRIVACY_TYPE_STARTUP, privacyState);
    if (code == ERRCODE_SUCCESS && isEnabled && !privacyState && identity.GetBundleName() == "com.ohos.sceneboard") {
        LocationConfigManager::GetInstance()->OpenPrivacyDialog();
    }
    reply.WriteInt32(locatorAbility->EnableAbility(isEnabled));
    return ERRCODE_SUCCESS;
}

int LocatorAbilityStub::PreUpdateSaAbility(MessageParcel &data, MessageParcel &reply, AppIdentity &identity)
{
    pid_t callingPid = IPCSkeleton::GetCallingPid();
    pid_t callingUid = IPCSkeleton::GetCallingUid();
    if (callingUid != static_cast<pid_t>(getuid()) || callingPid != getpid()) {
        LBSLOGE(LOCATOR, "check permission failed, [%{public}s]", identity.ToString().c_str());
        reply.WriteInt32(ERRCODE_PERMISSION_DENIED);
        return ERRCODE_PERMISSION_DENIED;
    }
    auto locatorAbility = LocatorAbility::GetInstance();
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
    auto locatorAbility = LocatorAbility::GetInstance();
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
    auto locatorAbility = LocatorAbility::GetInstance();
    if (locatorAbility == nullptr) {
        LBSLOGE(LOCATOR, "PreGetAddressByCoordinate: LocatorAbility is nullptr.");
        reply.WriteInt32(ERRCODE_SERVICE_UNAVAILABLE);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    locatorAbility->GetAddressByCoordinate(data, reply, identity.GetBundleName());
    return ERRCODE_SUCCESS;
}
#endif

#ifdef FEATURE_GEOCODE_SUPPORT
int LocatorAbilityStub::PreGetAddressByLocationName(MessageParcel &data, MessageParcel &reply, AppIdentity &identity)
{
    auto locatorAbility = LocatorAbility::GetInstance();
    if (locatorAbility == nullptr) {
        LBSLOGE(LOCATOR, "PreGetAddressByLocationName: LocatorAbility is nullptr.");
        reply.WriteInt32(ERRCODE_SERVICE_UNAVAILABLE);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    locatorAbility->GetAddressByLocationName(data, reply, identity.GetBundleName());
    return ERRCODE_SUCCESS;
}
#endif

int LocatorAbilityStub::PreUnregisterSwitchCallback(MessageParcel &data, MessageParcel &reply, AppIdentity &identity)
{
    auto locatorAbility = LocatorAbility::GetInstance();
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
    if (!CheckLocationSwitchState(reply)) {
        return ERRCODE_SWITCH_OFF;
    }
    if (!CheckLocationPermission(reply, identity)) {
        return ERRCODE_PERMISSION_DENIED;
    }
    auto locatorAbility = LocatorAbility::GetInstance();
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
    auto locatorAbility = LocatorAbility::GetInstance();
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
    if (!CheckLocationSwitchState(reply)) {
        return ERRCODE_SWITCH_OFF;
    }
    if (!CheckLocationPermission(reply, identity)) {
        return ERRCODE_PERMISSION_DENIED;
    }
    auto locatorAbility = LocatorAbility::GetInstance();
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
    auto locatorAbility = LocatorAbility::GetInstance();
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
    if (!CheckLocationSwitchState(reply)) {
        return ERRCODE_SWITCH_OFF;
    }
    if (!CheckPreciseLocationPermissions(reply, identity)) {
        return ERRCODE_PERMISSION_DENIED;
    }
    sptr<IRemoteObject> client = data.ReadObject<IRemoteObject>();
    auto locatorAbility = LocatorAbility::GetInstance();
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
    auto locatorAbility = LocatorAbility::GetInstance();
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
    if (!PermissionManager::CheckSystemPermission(identity.GetTokenId(), identity.GetTokenIdEx())) {
        LBSLOGE(LOCATOR, "CheckSystemPermission return false, [%{public}s]",
            identity.ToString().c_str());
        reply.WriteInt32(ERRCODE_SYSTEM_PERMISSION_DENIED);
        return ERRCODE_SYSTEM_PERMISSION_DENIED;
    }
    auto locatorAbility = LocatorAbility::GetInstance();
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
    if (!PermissionManager::CheckSystemPermission(identity.GetTokenId(), identity.GetTokenIdEx())) {
        LBSLOGE(LOCATOR, "CheckSystemPermission return false, [%{public}s]",
            identity.ToString().c_str());
        reply.WriteInt32(ERRCODE_SYSTEM_PERMISSION_DENIED);
        return ERRCODE_SYSTEM_PERMISSION_DENIED;
    }
    if (!CheckSettingsPermission(reply, identity)) {
        return ERRCODE_PERMISSION_DENIED;
    }

    auto locatorAbility = LocatorAbility::GetInstance();
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
    if (!CheckLocationSwitchState(reply)) {
        return ERRCODE_SWITCH_OFF;
    }
    if (!CheckLocationPermission(reply, identity)) {
        return ERRCODE_PERMISSION_DENIED;
    }
    auto locatorAbility = LocatorAbility::GetInstance();
    if (locatorAbility == nullptr) {
        LBSLOGE(LOCATOR, "PreStartCacheLocating: LocatorAbility is nullptr.");
        reply.WriteInt32(ERRCODE_SERVICE_UNAVAILABLE);
        return ERRCODE_SERVICE_UNAVAILABLE;
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
    auto locatorAbility = LocatorAbility::GetInstance();
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
    auto locatorAbility = LocatorAbility::GetInstance();
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
    auto locatorAbility = LocatorAbility::GetInstance();
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
    if (!CheckLocationPermission(reply, identity)) {
        return ERRCODE_PERMISSION_DENIED;
    }
    auto locatorAbility = LocatorAbility::GetInstance();
    if (locatorAbility == nullptr) {
        LBSLOGE(LOCATOR, "PreSendCommand: LocatorAbility is nullptr.");
        reply.WriteInt32(ERRCODE_SERVICE_UNAVAILABLE);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    std::unique_ptr<LocationCommand> locationCommand = std::make_unique<LocationCommand>();
    locationCommand->scenario =  data.ReadInt32();
    locationCommand->command = Str16ToStr8(data.ReadString16());
    reply.WriteInt32(locatorAbility->SendCommand(locationCommand));
    return ERRCODE_SUCCESS;
}
#endif

#ifdef FEATURE_GNSS_SUPPORT
int LocatorAbilityStub::PreAddFence(MessageParcel &data, MessageParcel &reply, AppIdentity &identity)
{
    return DoProcessFenceRequest(LocatorInterfaceCode::ADD_FENCE, data, reply, identity);
}
#endif

#ifdef FEATURE_GNSS_SUPPORT
int LocatorAbilityStub::PreRemoveFence(MessageParcel &data, MessageParcel &reply, AppIdentity &identity)
{
    return DoProcessFenceRequest(LocatorInterfaceCode::REMOVE_FENCE, data, reply, identity);
}
#endif

#ifdef FEATURE_GNSS_SUPPORT
int LocatorAbilityStub::DoProcessFenceRequest(
    LocatorInterfaceCode code, MessageParcel &data, MessageParcel &reply, AppIdentity &identity)
{
    if (!CheckLocationSwitchState(reply)) {
        return ERRCODE_SWITCH_OFF;
    }
    if (!CheckLocationPermission(reply, identity)) {
        return ERRCODE_PERMISSION_DENIED;
    }
    auto locatorAbility = LocatorAbility::GetInstance();
    if (locatorAbility == nullptr) {
        LBSLOGE(LOCATOR, "PreRemoveFence: LocatorAbility is nullptr.");
        reply.WriteInt32(ERRCODE_SERVICE_UNAVAILABLE);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    auto request = GeofenceRequest::Unmarshalling(data);
    if (code == LocatorInterfaceCode::ADD_FENCE) {
        reply.WriteInt32(locatorAbility->AddFence(request));
    } else if (code == LocatorInterfaceCode::REMOVE_FENCE) {
        reply.WriteInt32(locatorAbility->RemoveFence(request));
    }
    return ERRCODE_SUCCESS;
}
#endif

#ifdef FEATURE_GNSS_SUPPORT
int LocatorAbilityStub::PreAddGnssGeofence(MessageParcel &data, MessageParcel &reply, AppIdentity &identity)
{
    LBSLOGI(LOCATOR, "PreAddGnssGeofence enter.");
    if (!CheckLocationSwitchState(reply)) {
        return ERRCODE_SWITCH_OFF;
    }
    if (!CheckPreciseLocationPermissions(reply, identity)) {
        return ERRCODE_PERMISSION_DENIED;
    }
    auto locatorAbility = LocatorAbility::GetInstance();
    if (locatorAbility == nullptr) {
        LBSLOGE(LOCATOR, "LocatorAbility is nullptr.");
        reply.WriteInt32(ERRCODE_SERVICE_UNAVAILABLE);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    auto request = GeofenceRequest::Unmarshalling(data);
    request->SetBundleName(identity.GetBundleName());
    reply.WriteInt32(locatorAbility->AddGnssGeofence(request));
    return ERRCODE_SUCCESS;
}
#endif

#ifdef FEATURE_GNSS_SUPPORT
int LocatorAbilityStub::PreRemoveGnssGeofence(MessageParcel &data, MessageParcel &reply, AppIdentity &identity)
{
    LBSLOGI(LOCATOR, "PreRemoveGnssGeofence enter.");
    if (!CheckLocationSwitchState(reply)) {
        return ERRCODE_SWITCH_OFF;
    }
    if (!CheckPreciseLocationPermissions(reply, identity)) {
        return ERRCODE_PERMISSION_DENIED;
    }
    auto locatorAbility = LocatorAbility::GetInstance();
    if (locatorAbility == nullptr) {
        LBSLOGE(LOCATOR, "LocatorAbility is nullptr.");
        reply.WriteInt32(ERRCODE_SERVICE_UNAVAILABLE);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    std::shared_ptr<GeofenceRequest> request = std::make_shared<GeofenceRequest>();
    request->SetFenceId(data.ReadInt32());
    request->SetBundleName(identity.GetBundleName());
    reply.WriteInt32(locatorAbility->RemoveGnssGeofence(request));
    return ERRCODE_SUCCESS;
}
#endif

int LocatorAbilityStub::PreEnableLocationMock(MessageParcel &data, MessageParcel &reply, AppIdentity &identity)
{
    if (!PermissionManager::CheckSystemPermission(identity.GetTokenId(), identity.GetTokenIdEx())) {
        LBSLOGE(LOCATOR, "CheckSystemPermission return false, [%{public}s]",
            identity.ToString().c_str());
        reply.WriteInt32(ERRCODE_SYSTEM_PERMISSION_DENIED);
        return ERRCODE_SYSTEM_PERMISSION_DENIED;
    }
    if (!PermissionManager::CheckMockLocationPermission(identity.GetTokenId(), identity.GetTokenIdEx())) {
        LBSLOGE(LOCATOR, "CheckMockLocationPermission return false, [%{public}s]",
            identity.ToString().c_str());
        reply.WriteInt32(ERRCODE_PERMISSION_DENIED);
        return ERRCODE_PERMISSION_DENIED;
    }
    auto locatorAbility = LocatorAbility::GetInstance();
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
    if (!PermissionManager::CheckSystemPermission(identity.GetTokenId(), identity.GetTokenIdEx())) {
        LBSLOGE(LOCATOR, "CheckSystemPermission return false, [%{public}s]",
            identity.ToString().c_str());
        reply.WriteInt32(ERRCODE_SYSTEM_PERMISSION_DENIED);
        return ERRCODE_SYSTEM_PERMISSION_DENIED;
    }
    if (!PermissionManager::CheckMockLocationPermission(identity.GetTokenId(), identity.GetTokenIdEx())) {
        LBSLOGE(LOCATOR, "CheckMockLocationPermission return false, [%{public}s]",
            identity.ToString().c_str());
        reply.WriteInt32(ERRCODE_PERMISSION_DENIED);
        return ERRCODE_PERMISSION_DENIED;
    }
    auto locatorAbility = LocatorAbility::GetInstance();
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
    if (!PermissionManager::CheckSystemPermission(identity.GetTokenId(), identity.GetTokenIdEx())) {
        LBSLOGE(LOCATOR, "CheckSystemPermission return false, [%{public}s]",
            identity.ToString().c_str());
        reply.WriteInt32(ERRCODE_SYSTEM_PERMISSION_DENIED);
        return ERRCODE_SYSTEM_PERMISSION_DENIED;
    }
    if (!PermissionManager::CheckMockLocationPermission(identity.GetTokenId(), identity.GetTokenIdEx())) {
        LBSLOGE(LOCATOR, "CheckMockLocationPermission return false, [%{public}s]",
            identity.ToString().c_str());
        reply.WriteInt32(ERRCODE_PERMISSION_DENIED);
        return ERRCODE_PERMISSION_DENIED;
    }
    auto locatorAbility = LocatorAbility::GetInstance();
    if (locatorAbility == nullptr) {
        LBSLOGE(LOCATOR, "PreSetMockedLocations: LocatorAbility is nullptr.");
        reply.WriteInt32(ERRCODE_SERVICE_UNAVAILABLE);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    int timeInterval = data.ReadInt32();
    int locationSize = data.ReadInt32();
    timeInterval = timeInterval < 0 ? 1 : timeInterval;
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
    if (!PermissionManager::CheckSystemPermission(identity.GetTokenId(), identity.GetTokenIdEx())) {
        LBSLOGE(LOCATOR, "CheckSystemPermission return false, [%{public}s]",
            identity.ToString().c_str());
        reply.WriteInt32(ERRCODE_SYSTEM_PERMISSION_DENIED);
        return ERRCODE_SYSTEM_PERMISSION_DENIED;
    }
    if (!PermissionManager::CheckMockLocationPermission(identity.GetTokenId(), identity.GetTokenIdEx())) {
        LBSLOGE(LOCATOR, "CheckMockLocationPermission return false, [%{public}s]",
            identity.ToString().c_str());
        reply.WriteInt32(ERRCODE_PERMISSION_DENIED);
        return ERRCODE_PERMISSION_DENIED;
    }
    auto locatorAbility = LocatorAbility::GetInstance();
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
    if (!PermissionManager::CheckSystemPermission(identity.GetTokenId(), identity.GetTokenIdEx())) {
        LBSLOGE(LOCATOR, "CheckSystemPermission return false, [%{public}s]",
            identity.ToString().c_str());
        reply.WriteInt32(ERRCODE_SYSTEM_PERMISSION_DENIED);
        return ERRCODE_SYSTEM_PERMISSION_DENIED;
    }
    if (!PermissionManager::CheckMockLocationPermission(identity.GetTokenId(), identity.GetTokenIdEx())) {
        LBSLOGE(LOCATOR, "CheckMockLocationPermission return false, [%{public}s]",
            identity.ToString().c_str());
        reply.WriteInt32(ERRCODE_PERMISSION_DENIED);
        return ERRCODE_PERMISSION_DENIED;
    }
    auto locatorAbility = LocatorAbility::GetInstance();
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
    if (!PermissionManager::CheckSystemPermission(identity.GetTokenId(), identity.GetTokenIdEx())) {
        LBSLOGE(LOCATOR, "CheckSystemPermission return false, [%{public}s]",
            identity.ToString().c_str());
        reply.WriteInt32(ERRCODE_SYSTEM_PERMISSION_DENIED);
        return ERRCODE_SYSTEM_PERMISSION_DENIED;
    }
    if (!PermissionManager::CheckMockLocationPermission(identity.GetTokenId(), identity.GetTokenIdEx())) {
        LBSLOGE(LOCATOR, "CheckMockLocationPermission return false, [%{public}s]",
            identity.ToString().c_str());
        reply.WriteInt32(ERRCODE_PERMISSION_DENIED);
        return ERRCODE_PERMISSION_DENIED;
    }
    auto locatorAbility = LocatorAbility::GetInstance();
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

int LocatorAbilityStub::PreProxyForFreeze(MessageParcel &data, MessageParcel &reply, AppIdentity &identity)
{
    if (!CheckRssProcessName(reply, identity)) {
        return ERRCODE_PERMISSION_DENIED;
    }
    auto locatorAbility = LocatorAbility::GetInstance();
    if (locatorAbility == nullptr) {
        LBSLOGE(LOCATOR, "PreProxyForFreeze: LocatorAbility is nullptr.");
        reply.WriteInt32(ERRCODE_SERVICE_UNAVAILABLE);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    std::set<int> pidList;
    int size = data.ReadInt32();
    if (size > MAX_BUFF_SIZE) {
        size = MAX_BUFF_SIZE;
    }
    for (int i = 0; i < size; i++) {
        pidList.insert(data.ReadInt32());
    }
    bool isProxy = data.ReadBool();
    reply.WriteInt32(locatorAbility->ProxyForFreeze(pidList, isProxy));
    return ERRCODE_SUCCESS;
}

int LocatorAbilityStub::PreResetAllProxy(MessageParcel &data, MessageParcel &reply, AppIdentity &identity)
{
    if (!CheckRssProcessName(reply, identity)) {
        return ERRCODE_PERMISSION_DENIED;
    }
    auto locatorAbility = LocatorAbility::GetInstance();
    if (locatorAbility == nullptr) {
        LBSLOGE(LOCATOR, "PreResetAllProxy: LocatorAbility is nullptr.");
        reply.WriteInt32(ERRCODE_SERVICE_UNAVAILABLE);
        return ERRCODE_SERVICE_UNAVAILABLE;
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
    auto locatorAbility = LocatorAbility::GetInstance();
    if (locatorAbility == nullptr) {
        LBSLOGE(LOCATOR, "PreReportLocation: LocatorAbility is nullptr.");
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    std::string systemAbility = data.ReadString();
    std::unique_ptr<Location> location = Location::Unmarshalling(data);
    locatorAbility->ReportLocation(location, systemAbility, identity);
#ifdef FEATURE_GNSS_SUPPORT
    if (systemAbility == NETWORK_ABILITY) {
        locatorAbility->SendNetworkLocation(location);
    }
#endif
    return ERRCODE_SUCCESS;
}

int LocatorAbilityStub::PreRegisterLocatingRequiredDataCallback(MessageParcel &data,
    MessageParcel &reply, AppIdentity &identity)
{
    if (!CheckPreciseLocationPermissions(reply, identity)) {
        return ERRCODE_PERMISSION_DENIED;
    }
    if (!PermissionManager::CheckSystemPermission(identity.GetTokenId(), identity.GetTokenIdEx())) {
        LBSLOGE(LOCATOR, "CheckSystemPermission return false, [%{public}s]",
            identity.ToString().c_str());
        reply.WriteInt32(ERRCODE_SYSTEM_PERMISSION_DENIED);
        return ERRCODE_SYSTEM_PERMISSION_DENIED;
    }
    std::shared_ptr<LocatingRequiredDataConfig> dataConfig = LocatingRequiredDataConfig::Unmarshalling(data);
    sptr<IRemoteObject> client = data.ReadObject<IRemoteObject>();
    if (client == nullptr) {
        LBSLOGE(LOCATOR, "%{public}s: client is nullptr.", __func__);
        reply.WriteInt32(ERRCODE_SERVICE_UNAVAILABLE);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    auto locatorDataManager = LocatorRequiredDataManager::GetInstance();
    if (locatorDataManager == nullptr) {
        LBSLOGE(LOCATOR, "%{public}s: locatorDataManager is nullptr.", __func__);
        reply.WriteInt32(ERRCODE_SERVICE_UNAVAILABLE);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    client->AddDeathRecipient(scanRecipient_);
    LocationErrCode errorCode = locatorDataManager->RegisterCallback(dataConfig, client);

    reply.WriteInt32(errorCode);
    return ERRCODE_SUCCESS;
}

int LocatorAbilityStub::PreUnregisterLocatingRequiredDataCallback(MessageParcel &data,
    MessageParcel &reply, AppIdentity &identity)
{
    if (!CheckPreciseLocationPermissions(reply, identity)) {
        return ERRCODE_PERMISSION_DENIED;
    }
    if (!PermissionManager::CheckSystemPermission(identity.GetTokenId(), identity.GetTokenIdEx())) {
        LBSLOGE(LOCATOR, "CheckSystemPermission return false, [%{public}s]",
            identity.ToString().c_str());
        reply.WriteInt32(ERRCODE_SYSTEM_PERMISSION_DENIED);
        return ERRCODE_SYSTEM_PERMISSION_DENIED;
    }
    sptr<IRemoteObject> client = data.ReadObject<IRemoteObject>();
    if (client == nullptr) {
        LBSLOGE(LOCATOR, "%{public}s: client is nullptr.", __func__);
        reply.WriteInt32(ERRCODE_SERVICE_UNAVAILABLE);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    auto locatorDataManager = LocatorRequiredDataManager::GetInstance();
    if (locatorDataManager == nullptr) {
        LBSLOGE(LOCATOR, "%{public}s: locatorDataManager is nullptr.", __func__);
        reply.WriteInt32(ERRCODE_SERVICE_UNAVAILABLE);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    client->RemoveDeathRecipient(scanRecipient_);
    LocationErrCode errorCode = locatorDataManager->UnregisterCallback(client);

    reply.WriteInt32(errorCode);
    return ERRCODE_SUCCESS;
}

#ifdef FEATURE_GNSS_SUPPORT
int LocatorAbilityStub::PreQuerySupportCoordinateSystemType(MessageParcel &data,
    MessageParcel &reply, AppIdentity &identity)
{
    auto locatorAbility = LocatorAbility::GetInstance();
    if (locatorAbility == nullptr) {
        LBSLOGE(LOCATOR, "LocatorAbility is nullptr.");
        reply.WriteInt32(ERRCODE_SERVICE_UNAVAILABLE);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    std::vector<CoordinateSystemType> coordinateSystemTypes;
    auto errCode = locatorAbility->QuerySupportCoordinateSystemType(coordinateSystemTypes);
    reply.WriteInt32(errCode);
    if (errCode != ERRCODE_SUCCESS) {
        return errCode;
    }
    int size = static_cast<int>(coordinateSystemTypes.size()) > COORDINATE_SYSTEM_TYPE_SIZE ?
        COORDINATE_SYSTEM_TYPE_SIZE : static_cast<int>(coordinateSystemTypes.size());
    reply.WriteInt32(size);
    for (int i = 0; i < size; i++) {
        reply.WriteInt32(static_cast<int>(coordinateSystemTypes[i]));
    }
    return ERRCODE_SUCCESS;
}
#endif

bool LocatorAbilityStub::CheckLocationPermission(MessageParcel &reply, AppIdentity &identity)
{
    uint32_t callingTokenId = identity.GetTokenId();
    uint32_t callingFirstTokenid = identity.GetFirstTokenId();
    if (!PermissionManager::CheckLocationPermission(callingTokenId, callingFirstTokenid) &&
        !PermissionManager::CheckApproximatelyPermission(callingTokenId, callingFirstTokenid)) {
        LBSLOGE(LOCATOR, "%{public}d %{public}s failed", callingTokenId, __func__);
        reply.WriteInt32(ERRCODE_PERMISSION_DENIED);
        return false;
    } else {
        return true;
    }
}

bool LocatorAbilityStub::CheckRssProcessName(MessageParcel &reply, AppIdentity &identity)
{
    uint32_t callingTokenId = identity.GetTokenId();
    if (!PermissionManager::CheckRssProcessName(callingTokenId)) {
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
    if (!PermissionManager::CheckLocationPermission(callingTokenId, callingFirstTokenid) ||
        !PermissionManager::CheckApproximatelyPermission(callingTokenId, callingFirstTokenid)) {
        LBSLOGE(LOCATOR, "%{public}d %{public}s failed", callingTokenId, __func__);
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
    if (!PermissionManager::CheckSecureSettings(callingTokenId, callingFirstTokenid)) {
        LBSLOGE(LOCATOR, "has no access permission, CheckSecureSettings return false");
        reply.WriteInt32(ERRCODE_PERMISSION_DENIED);
        return false;
    } else {
        return true;
    }
}

bool LocatorAbilityStub::CheckLocationSwitchState(MessageParcel &reply)
{
    auto locatorAbility = LocatorAbility::GetInstance();
    if (locatorAbility == nullptr) {
        LBSLOGE(LOCATOR, "CheckLocationSwitchState: LocatorAbility is nullptr.");
        return false;
    }
    int state = DISABLED;
    LocationErrCode errorCode = locatorAbility->GetSwitchState(state);
    if (errorCode != ERRCODE_SUCCESS) {
        LBSLOGE(LOCATOR, "GetSwitchState failed errCode = %{public}d.", errorCode);
    }
    if (state == DISABLED) {
        LBSLOGE(LOCATOR, "switch state is off.");
        reply.WriteInt32(ERRCODE_SWITCH_OFF);
        return false;
    }
    return true;
}

int LocatorAbilityStub::PreRegisterLocationError(MessageParcel &data, MessageParcel &reply, AppIdentity &identity)
{
    if (!CheckLocationSwitchState(reply)) {
        return ERRCODE_SWITCH_OFF;
    }
    if (!CheckLocationPermission(reply, identity)) {
        return ERRCODE_PERMISSION_DENIED;
    }
    auto locatorAbility = LocatorAbility::GetInstance();
    if (locatorAbility == nullptr) {
        LBSLOGE(LOCATOR, "PreStartLocating: LocatorAbility is nullptr.");
        reply.WriteInt32(ERRCODE_SERVICE_UNAVAILABLE);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    sptr<IRemoteObject> remoteObject = data.ReadRemoteObject();
    if (remoteObject == nullptr) {
        LBSLOGE(LOCATOR, "StartLocating remote object nullptr");
        reply.WriteInt32(ERRCODE_SERVICE_UNAVAILABLE);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    sptr<ILocatorCallback> callback = iface_cast<ILocatorCallback>(remoteObject);
    reply.WriteInt32(locatorAbility->RegisterLocationError(callback, identity));
    return ERRCODE_SUCCESS;
}

int LocatorAbilityStub::PreUnregisterLocationError(MessageParcel &data, MessageParcel &reply, AppIdentity &identity)
{
    if (!CheckLocationPermission(reply, identity)) {
        return ERRCODE_PERMISSION_DENIED;
    }
    auto locatorAbility = LocatorAbility::GetInstance();
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
    reply.WriteInt32(locatorAbility->UnregisterLocationError(callback, identity));
    return ERRCODE_SUCCESS;
}

int LocatorAbilityStub::PreReportLocationError(MessageParcel &data, MessageParcel &reply, AppIdentity &identity)
{
    pid_t callingPid = IPCSkeleton::GetCallingPid();
    pid_t callingUid = IPCSkeleton::GetCallingUid();
    if (callingUid != static_cast<pid_t>(getuid()) || callingPid != getpid()) {
        LBSLOGE(LOCATOR, "check system permission failed, [%{public}s]",
            identity.ToString().c_str());
        return ERRCODE_PERMISSION_DENIED;
    }
    auto locatorAbility = LocatorAbility::GetInstance();
    if (locatorAbility == nullptr) {
        LBSLOGE(LOCATOR, "PreReportLocation: LocatorAbility is nullptr.");
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    int32_t errCode = data.ReadInt32();
    std::string errMsg = data.ReadString();
    std::string uuid = data.ReadString();
    locatorAbility->ReportLocationError(uuid, errCode);
    return ERRCODE_SUCCESS;
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
    if (code != static_cast<uint32_t>(LocatorInterfaceCode::PROXY_PID_FOR_FREEZE)) {
        LBSLOGI(LOCATOR,
            "OnReceived cmd = %{public}u, flags= %{public}d, identity= [%{public}s], timestamp = %{public}s",
            code, option.GetFlags(), identity.ToString().c_str(),
            std::to_string(CommonUtils::GetCurrentTimeStamp()).c_str());
    }
    std::string callingIdentity = IPCSkeleton::ResetCallingIdentity();

    if (data.ReadInterfaceToken() != GetDescriptor()) {
        LBSLOGE(LOCATOR, "invalid token.");
        IPCSkeleton::SetCallingIdentity(callingIdentity);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    RemoveUnloadTask(code);

    auto handleFunc = locatorHandleMap_.find(static_cast<LocatorInterfaceCode>(code));
    if (handleFunc != locatorHandleMap_.end() && handleFunc->second != nullptr) {
        auto memberFunc = handleFunc->second;
        ret = memberFunc(data, reply, identity);
    } else {
        LBSLOGE(LOCATOR, "OnReceived cmd = %{public}u, unsupport service.", code);
        ret = IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
    IPCSkeleton::SetCallingIdentity(callingIdentity);
    WriteLocationDenyReportEvent(code, ret, data, identity);
    PostUnloadTask(code);
    return ret;
}

void LocatorAbilityStub::SaDumpInfo(std::string& result)
{
    auto locatorAbility = LocatorAbility::GetInstance();
    if (locatorAbility == nullptr) {
        LBSLOGE(LOCATOR, "SaDumpInfo: LocatorAbility is nullptr.");
        return;
    }
    int state = DISABLED;
    LocationErrCode errorCode = locatorAbility->GetSwitchState(state);
    if (errorCode != ERRCODE_SUCCESS) {
        LBSLOGE(LOCATOR, "GetSwitchState failed errCode = %{public}d.", errorCode);
    }
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

bool LocatorAbilityStub::RemoveUnloadTask(uint32_t code)
{
    auto locatorAbility = LocatorAbility::GetInstance();
    if (locatorAbility == nullptr) {
        LBSLOGE(LOCATOR, "%{public}s: LocatorAbility is nullptr.", __func__);
        return false;
    }
    locatorAbility->RemoveUnloadTask(code);
    return true;
}

bool LocatorAbilityStub::PostUnloadTask(uint32_t code)
{
    auto locatorAbility = LocatorAbility::GetInstance();
    if (locatorAbility == nullptr) {
        LBSLOGE(LOCATOR, "%{public}s: LocatorAbility is nullptr.", __func__);
        return false;
    }
    locatorAbility->PostUnloadTask(code);
    return true;
}

void LocatorAbilityStub::WriteLocationDenyReportEvent(uint32_t code, int errCode,
    MessageParcel &data, AppIdentity &identity)
{
    if (code == static_cast<uint32_t>(LocatorInterfaceCode::START_LOCATING) && errCode != ERRCODE_SUCCESS) {
        std::unique_ptr<RequestConfig> requestConfig = std::make_unique<RequestConfig>();
        requestConfig->ReadFromParcel(data);
        auto requestInfo = requestConfig->ToString();
        WriteLocationInnerEvent(LOCATION_REQUEST_DENY, {"errorCode", std::to_string(errCode),
            "requestAppName", identity.GetBundleName(), "requestInfo", requestInfo});
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
    auto locatorAbility = LocatorAbility::GetInstance();
    if (locatorAbility != nullptr) {
        locatorAbility->RemoveUnloadTask(DEFAULT_CODE);
        locatorAbility->UnregisterSwitchCallback(remote.promote());
        locatorAbility->PostUnloadTask(DEFAULT_CODE);
        LBSLOGI(LOCATOR, "switch callback OnRemoteDied");
    }
}

ScanCallbackDeathRecipient::ScanCallbackDeathRecipient()
{
}

ScanCallbackDeathRecipient::~ScanCallbackDeathRecipient()
{
}

void ScanCallbackDeathRecipient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    auto locatorDataManager = LocatorRequiredDataManager::GetInstance();
    if (locatorDataManager != nullptr) {
        locatorDataManager->UnregisterCallback(remote.promote());
        LBSLOGI(LOCATOR, "scan callback OnRemoteDied");
    }
}
} // namespace Location
} // namespace OHOS