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

#include "ipc_skeleton.h"
#include "ipc_types.h"

#include "common_utils.h"
#include "locator_ability.h"

namespace OHOS {
namespace Location {
void LocatorAbilityStub::ParseDataAndStartLocating(MessageParcel& data, MessageParcel& reply, pid_t pid, pid_t uid)
{
    std::unique_ptr<RequestConfig> requestConfig = RequestConfig::Unmarshalling(data);
    sptr<IRemoteObject> remoteObject = data.ReadRemoteObject();
    std::string bundleName = Str16ToStr8(data.ReadString16());
    if (remoteObject == nullptr) {
        LBSLOGE(LOCATOR, "StartLocating remote object nullptr");
        return;
    }
    if (bundleName.empty()) {
        LBSLOGE(LOCATOR, "StartLocating get empty bundle name");
        return;
    }

    sptr<IRemoteObject::DeathRecipient> death(new (std::nothrow) LocatorCallbackDeathRecipient());
    remoteObject->AddDeathRecipient(death.GetRefPtr());
    sptr<ILocatorCallback> callback = iface_cast<ILocatorCallback>(remoteObject);
    StartLocating(requestConfig, callback, bundleName, pid, uid);
}

void LocatorAbilityStub::ParseDataAndStopLocating(MessageParcel& data, MessageParcel& reply)
{
    sptr<IRemoteObject> remoteObject = data.ReadRemoteObject();
    if (remoteObject == nullptr) {
        LBSLOGE(LOCATOR, "LocatorAbility::StartLocating remote object nullptr");
        return;
    }
    sptr<ILocatorCallback> callback = iface_cast<ILocatorCallback>(remoteObject);
    StopLocating(callback);
}

int LocatorAbilityStub::ReportStatus(MessageParcel& data, int type)
{
    sptr<IRemoteObject> remoteObject = data.ReadRemoteObject();
    if (remoteObject == nullptr) {
        LBSLOGE(LOCATOR, "LocatorAbility::StartLocating remote object nullptr");
        return EXCEPTION;
    }
    sptr<ILocatorCallback> callback = iface_cast<ILocatorCallback>(remoteObject);
    int result = data.ReadInt32();
    switch (type) {
        case REPORT_LOCATION_STATUS: {
            ReportLocationStatus(callback, result);
            break;
        }
        case REPORT_ERROR_STATUS: {
            ReportErrorStatus(callback, result);
            break;
        }
        default:
            break;
    }
    return REPLY_NO_EXCEPTION;
}

void LocatorAbilityStub::ParseDataAndStartCacheLocating(MessageParcel& data, MessageParcel& reply)
{
    std::unique_ptr<CachedGnssLocationsRequest> requestConfig = std::make_unique<CachedGnssLocationsRequest>();
    requestConfig->reportingPeriodSec = data.ReadInt32();
    requestConfig->wakeUpCacheQueueFull = data.ReadBool();
    sptr<IRemoteObject> remoteObject = data.ReadRemoteObject();
    std::string bundleName = Str16ToStr8(data.ReadString16());
    if (remoteObject == nullptr) {
        LBSLOGE(LOCATOR, "ParseDataAndStartCacheLocating remote object nullptr");
        return;
    }
    if (bundleName.empty()) {
        LBSLOGE(LOCATOR, "ParseDataAndStartCacheLocating get empty bundle name");
        return;
    }

    sptr<ICachedLocationsCallback> callback = iface_cast<ICachedLocationsCallback>(remoteObject);
    RegisterCachedLocationCallback(requestConfig, callback, bundleName);
}

void LocatorAbilityStub::ParseDataAndStopCacheLocating(MessageParcel& data, MessageParcel& reply)
{
    sptr<IRemoteObject> remoteObject = data.ReadRemoteObject();
    if (remoteObject == nullptr) {
        LBSLOGE(LOCATOR, "LocatorAbility::ParseDataAndStopCacheLocating remote object nullptr");
        return;
    }
    sptr<ICachedLocationsCallback> callback = iface_cast<ICachedLocationsCallback>(remoteObject);
    UnregisterCachedLocationCallback(callback);
}

int32_t LocatorAbilityStub::ProcessMsgRequirLocationPermission(uint32_t &code,
    MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    pid_t callingPid = IPCSkeleton::GetCallingPid();
    pid_t callingUid = IPCSkeleton::GetCallingUid();
    int ret = REPLY_NO_EXCEPTION;
    if (!CommonUtils::CheckLocationPermission()) {
        LBSLOGI(LOCATOR, "pid:%{public}d uid:%{public}d has no access permission,CheckLocationPermission return false",
                callingPid, callingUid);
        reply.WriteInt32(SECURITY_EXCEPTION);
        reply.WriteString("should grant location permission");
        ret = SECURITY_EXCEPTION;
        return ret;
    }
    std::string identity = IPCSkeleton::ResetCallingIdentity();
    switch (code) {
        case REG_GNSS_STATUS_CALLBACK: {
            sptr<IRemoteObject> client = data.ReadObject<IRemoteObject>();
            RegisterGnssStatusCallback(client, callingUid);
            break;
        }
        case UNREG_GNSS_STATUS_CALLBACK: {
            sptr<IRemoteObject> client = data.ReadObject<IRemoteObject>();
            UnregisterGnssStatusCallback(client);
            break;
        }
        case REG_NMEA_CALLBACK: {
            sptr<IRemoteObject> client = data.ReadObject<IRemoteObject>();
            RegisterNmeaMessageCallback(client, callingUid);
            break;
        }
        case UNREG_NMEA_CALLBACK: {
            sptr<IRemoteObject> client = data.ReadObject<IRemoteObject>();
            UnregisterNmeaMessageCallback(client);
            break;
        }
        case START_LOCATING: {
            if (GetSwitchState() == DISABLED) {
                ret = SWITCH_OFF_EXCEPTION;
                break;
            }
            ParseDataAndStartLocating(data, reply, callingPid, callingUid);
            break;
        }
        case STOP_LOCATING: {
            ParseDataAndStopLocating(data, reply);
            break;
        }
        case GET_CACHE_LOCATION: {
            if (GetSwitchState() == DISABLED) {
                ret = SWITCH_OFF_EXCEPTION;
                break;
            }
            ret = GetCacheLocation(data, reply);
            break;
        }
        case GEO_IS_AVAILABLE: {
            if (GetSwitchState() == DISABLED) {
                ret = SWITCH_OFF_EXCEPTION;
                break;
            }
            ret = IsGeoConvertAvailable(data, reply);
            break;
        }
        case GET_FROM_COORDINATE: {
            if (GetSwitchState() == DISABLED) {
                ret = SWITCH_OFF_EXCEPTION;
                break;
            }
            ret = GetAddressByCoordinate(data, reply);
            break;
        }
        case GET_FROM_LOCATION_NAME: {
            if (GetSwitchState() == DISABLED) {
                ret = SWITCH_OFF_EXCEPTION;
                break;
            }
            ret = GetAddressByLocationName(data, reply);
            break;
        }
        case IS_PRIVACY_COMFIRMED: {
            reply.WriteInt32(IsLocationPrivacyConfirmed(CommonUtils::GetPrivacyTypeByInt(data.ReadInt32())));
            break;
        }
        case SET_PRIVACY_COMFIRM_STATUS: {
            SetLocationPrivacyConfirmStatus(CommonUtils::GetPrivacyTypeByInt(data.ReadInt32()), data.ReadBool());
            break;
        }
        case REG_CACHED_CALLBACK: {
            if (GetSwitchState() == DISABLED) {
                ret = SWITCH_OFF_EXCEPTION;
                break;
            }
            ParseDataAndStartCacheLocating(data, reply);
            break;
        }
        case UNREG_CACHED_CALLBACK: {
            ParseDataAndStopCacheLocating(data, reply);
            break;
        }
        case GET_CACHED_LOCATION_SIZE: {
            if (GetSwitchState() == DISABLED) {
                ret = SWITCH_OFF_EXCEPTION;
                break;
            }
            reply.WriteInt32(GetCachedGnssLocationsSize());
            break;
        }
        case FLUSH_CACHED_LOCATIONS: {
            if (GetSwitchState() == DISABLED) {
                ret = SWITCH_OFF_EXCEPTION;
                break;
            }
            FlushCachedGnssLocations();
            break;
        }
        case SEND_COMMAND: {
            if (GetSwitchState() == DISABLED) {
                ret = SWITCH_OFF_EXCEPTION;
                break;
            }
            std::unique_ptr<LocationCommand> locationCommand = std::make_unique<LocationCommand>();
            locationCommand->scenario =  data.ReadInt32();
            locationCommand->command = data.ReadBool();
            SendCommand(locationCommand);
            break;
        }
        case ADD_FENCE: {
            if (GetSwitchState() == DISABLED) {
                ret = SWITCH_OFF_EXCEPTION;
                break;
            }
            std::unique_ptr<GeofenceRequest> request = std::make_unique<GeofenceRequest>();
            request->priority = data.ReadInt32();
            request->scenario = data.ReadInt32();
            request->geofence.latitude = data.ReadDouble();
            request->geofence.longitude = data.ReadDouble();
            request->geofence.radius = data.ReadDouble();
            request->geofence.expiration = data.ReadDouble();
            AddFence(request);
            break;
        }
        case REMOVE_FENCE: {
            if (GetSwitchState() == DISABLED) {
                ret = SWITCH_OFF_EXCEPTION;
                break;
            }
            std::unique_ptr<GeofenceRequest> request = std::make_unique<GeofenceRequest>();
            request->priority = data.ReadInt32();
            request->scenario = data.ReadInt32();
            request->geofence.latitude = data.ReadDouble();
            request->geofence.longitude = data.ReadDouble();
            request->geofence.radius = data.ReadDouble();
            request->geofence.expiration = data.ReadDouble();
            RemoveFence(request);
            break;
        }
        default:
            ret = IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
    IPCSkeleton::SetCallingIdentity(identity);
    return ret;
}

int32_t LocatorAbilityStub::ProcessMsgRequirSecureSettingsPermission(uint32_t &code,
    MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    pid_t callingUid = IPCSkeleton::GetCallingUid();
    int ret = REPLY_NO_EXCEPTION;
    if (code == REG_SWITCH_CALLBACK || code == UNREG_SWITCH_CALLBACK || code == ENABLE_ABILITY) {
        if (!CommonUtils::CheckSecureSettings()) {
            LBSLOGI(LOCATOR, "has no access permission,CheckSecureSettings return false");
            reply.WriteInt32(SECURITY_EXCEPTION);
            reply.WriteString("should grant location permission");
            ret = SECURITY_EXCEPTION;
            return ret;
        }
    }
    std::string identity = IPCSkeleton::ResetCallingIdentity();
    switch (code) {
        case GET_SWITCH_STATE: {
            reply.WriteInt32(GetSwitchState());
            break;
        }
        case REG_SWITCH_CALLBACK: {
            sptr<IRemoteObject> client = data.ReadObject<IRemoteObject>();
            RegisterSwitchCallback(client, callingUid);
            break;
        }
        case UNREG_SWITCH_CALLBACK: {
            sptr<IRemoteObject> client = data.ReadObject<IRemoteObject>();
            UnregisterSwitchCallback(client);
            break;
        }
        case ENABLE_ABILITY: {
            if (!CommonUtils::CheckSystemCalling(callingUid)) {
                ret = SECURITY_EXCEPTION;
                break;
            }
            bool isEnabled = data.ReadBool();
            EnableAbility(isEnabled);
            break;
        }
        default:
            ret = MSG_UNPROCESSED;
    }
    IPCSkeleton::SetCallingIdentity(identity);
    return ret;
}

int32_t LocatorAbilityStub::ProcessMsg(uint32_t &code,
    MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    pid_t callingUid = IPCSkeleton::GetCallingUid();
    int ret = REPLY_NO_EXCEPTION;
    std::string identity = IPCSkeleton::ResetCallingIdentity();
    switch (code) {
        case REPORT_LOCATION: {
            if (!CommonUtils::CheckSystemCalling(callingUid)) {
                ret = SECURITY_EXCEPTION;
                break;
            }
            if (GetSwitchState() == DISABLED) {
                ret = SWITCH_OFF_EXCEPTION;
                break;
            }
            std::string abilityName = data.ReadString();
            std::unique_ptr<Location> location = Location::Unmarshalling(data);
            ReportLocation(location, abilityName);
            break;
        }
        case REPORT_GNSS_SESSION_STATUS: {
            if (!CommonUtils::CheckSystemCalling(callingUid)) {
                ret = SECURITY_EXCEPTION;
                break;
            }
            if (GetSwitchState() == DISABLED) {
                ret = SWITCH_OFF_EXCEPTION;
                break;
            }
            int status = data.ReadInt32();
            ReportGnssSessionStatus(status);
            break;
        }
        case REPORT_SV: {
            if (!CommonUtils::CheckSystemCalling(callingUid)) {
                ret = SECURITY_EXCEPTION;
                break;
            }
            if (GetSwitchState() == DISABLED) {
                ret = SWITCH_OFF_EXCEPTION;
                break;
            }
            std::unique_ptr<SatelliteStatus> sv = SatelliteStatus::Unmarshalling(data);
            ReportSv(sv);
            break;
        }
        case REPORT_NMEA: {
            if (!CommonUtils::CheckSystemCalling(callingUid)) {
                ret = SECURITY_EXCEPTION;
                break;
            }
            if (GetSwitchState() == DISABLED) {
                ret = SWITCH_OFF_EXCEPTION;
                break;
            }
            std::string nmea = data.ReadString();
            ReportNmea(nmea);
            break;
        }
        case REPORT_LOCATION_STATUS: {
            if (!CommonUtils::CheckSystemCalling(callingUid)) {
                ret = SECURITY_EXCEPTION;
                break;
            }
            if (GetSwitchState() == DISABLED) {
                ret = SWITCH_OFF_EXCEPTION;
                break;
            }
            ReportStatus(data, REPORT_LOCATION_STATUS);
            break;
        }
        case REPORT_ERROR_STATUS: {
            if (!CommonUtils::CheckSystemCalling(callingUid)) {
                ret = SECURITY_EXCEPTION;
                break;
            }
            if (GetSwitchState() == DISABLED) {
                ret = SWITCH_OFF_EXCEPTION;
                break;
            }
            ReportStatus(data, REPORT_ERROR_STATUS);
            break;
        }
        case UPDATE_SA_ABILITY: {
            if (!CommonUtils::CheckSystemCalling(callingUid)) {
                ret = SECURITY_EXCEPTION;
                break;
            }
            UpdateSaAbility();
            break;
        }
        default:
            ret = MSG_UNPROCESSED;
    }
    IPCSkeleton::SetCallingIdentity(identity);
    return ret;
}

int32_t LocatorAbilityStub::OnRemoteRequest(uint32_t code,
    MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    pid_t callingPid = IPCSkeleton::GetCallingPid();
    pid_t callingUid = IPCSkeleton::GetCallingUid();

    LBSLOGI(LOCATOR, "OnReceived cmd = %{public}u, flags= %{public}d, pid= %{public}d, uid= %{public}d",
        code, option.GetFlags(), callingPid, callingUid);
    int ret = REPLY_NO_EXCEPTION;
    if (data.ReadInterfaceToken() != GetDescriptor()) {
        LBSLOGE(LOCATOR, "invalid token.");
        return -1;
    }

    ret = ProcessMsg(code, data, reply, option);
    if (ret == MSG_UNPROCESSED) {
        ret = ProcessMsgRequirSecureSettingsPermission(code, data, reply, option);
    }
    if (ret == MSG_UNPROCESSED) {
        ret = ProcessMsgRequirLocationPermission(code, data, reply, option);
    }
    return ret;
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
    sptr<LocatorAbility> locatorAbility = DelayedSingleton<LocatorAbility>::GetInstance().get();
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
    sptr<LocatorAbility> locatorAbility = DelayedSingleton<LocatorAbility>::GetInstance().get();
    if (locatorAbility != nullptr) {
        locatorAbility->UnregisterSwitchCallback(remote.promote());
        LBSLOGI(LOCATOR, "switch callback OnRemoteDied");
    }
}
} // namespace Location
} // namespace OHOS