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

#ifndef LOCATOR_SKELETON_H
#define LOCATOR_SKELETON_H

#include <map>
#include <string>

#include "iremote_object.h"
#include "iremote_stub.h"
#include "message_option.h"
#include "message_parcel.h"

#include "app_identity.h"
#include "i_locator.h"
#include "locationhub_ipc_interface_code.h"

namespace OHOS {
namespace Location {
class ScanCallbackDeathRecipient : public IRemoteObject::DeathRecipient {
public:
    void OnRemoteDied(const wptr<IRemoteObject> &remote) override;
    ScanCallbackDeathRecipient();
    ~ScanCallbackDeathRecipient() override;
};

class LocatorAbilityStub : public IRemoteStub<ILocator> {
public:
    using LocatorMsgHandle = std::function<int(MessageParcel &, MessageParcel &, AppIdentity &)>;
    using LocatorMsgHandleMap = std::map<LocatorInterfaceCode, LocatorMsgHandle>;

    LocatorAbilityStub();
    virtual ~LocatorAbilityStub() = default;
    void InitLocatorHandleMap();
    int32_t OnRemoteRequest(uint32_t code,
        MessageParcel &data, MessageParcel &reply, MessageOption &option) override;
    int32_t Dump(int32_t fd, const std::vector<std::u16string>& args) override;

private:
    int PreGetSwitchState(MessageParcel &data, MessageParcel &reply, AppIdentity &identity);
    int PreRegisterSwitchCallback(MessageParcel &data, MessageParcel &reply, AppIdentity &identity);
    int PreUnregisterSwitchCallback(MessageParcel &data, MessageParcel &reply, AppIdentity &identity);
    int PreStartLocating(MessageParcel &data, MessageParcel &reply, AppIdentity &identity);
    int PreStopLocating(MessageParcel &data, MessageParcel &reply, AppIdentity &identity);
    int PreGetCacheLocation(MessageParcel &data, MessageParcel &reply, AppIdentity &identity);
    int PreEnableAbility(MessageParcel &data, MessageParcel &reply, AppIdentity &identity);
    int PreUpdateSaAbility(MessageParcel &data, MessageParcel &reply, AppIdentity &identity);
#ifdef FEATURE_GEOCODE_SUPPORT
    int PreIsGeoConvertAvailable(MessageParcel &data, MessageParcel &reply, AppIdentity &identity);
    int PreGetAddressByCoordinate(MessageParcel &data, MessageParcel &reply, AppIdentity &identity);
    int PreGetAddressByLocationName(MessageParcel &data, MessageParcel &reply, AppIdentity &identity);
    int PreEnableReverseGeocodingMock(MessageParcel &data, MessageParcel &reply, AppIdentity &identity);
    int PreDisableReverseGeocodingMock(MessageParcel &data, MessageParcel &reply, AppIdentity &identity);
    int PreSetReverseGeocodingMockInfo(MessageParcel &data, MessageParcel &reply, AppIdentity &identity);
#endif
#ifdef FEATURE_GNSS_SUPPORT
    int PreRegisterGnssStatusCallback(MessageParcel &data, MessageParcel &reply, AppIdentity &identity);
    int PreUnregisterGnssStatusCallback(MessageParcel &data, MessageParcel &reply, AppIdentity &identity);
    int PreRegisterNmeaMessageCallback(MessageParcel &data, MessageParcel &reply, AppIdentity &identity);
    int PreUnregisterNmeaMessageCallback(MessageParcel &data, MessageParcel &reply, AppIdentity &identity);
    int PreRegisterNmeaMessageCallbackV9(MessageParcel &data, MessageParcel &reply, AppIdentity &identity);
    int PreUnregisterNmeaMessageCallbackV9(MessageParcel &data, MessageParcel &reply, AppIdentity &identity);
    int PreStartCacheLocating(MessageParcel &data, MessageParcel &reply, AppIdentity &identity);
    int PreStopCacheLocating(MessageParcel &data, MessageParcel &reply, AppIdentity &identity);
    int PreGetCachedGnssLocationsSize(MessageParcel &data, MessageParcel &reply, AppIdentity &identity);
    int PreFlushCachedGnssLocations(MessageParcel &data, MessageParcel &reply, AppIdentity &identity);
    int PreSendCommand(MessageParcel &data, MessageParcel &reply, AppIdentity &identity);
    int PreAddFence(MessageParcel &data, MessageParcel &reply, AppIdentity &identity);
    int PreRemoveFence(MessageParcel &data, MessageParcel &reply, AppIdentity &identity);
    int PreAddGnssGeofence(MessageParcel &data, MessageParcel &reply, AppIdentity &identity);
    int PreRemoveGnssGeofence(MessageParcel &data, MessageParcel &reply, AppIdentity &identity);
    int PreQuerySupportCoordinateSystemType(MessageParcel &data, MessageParcel &reply, AppIdentity &identity);
#endif
    int PreIsLocationPrivacyConfirmed(MessageParcel &data, MessageParcel &reply, AppIdentity &identity);
    int PreSetLocationPrivacyConfirmStatus(MessageParcel &data, MessageParcel &reply, AppIdentity &identity);
    int PreEnableLocationMock(MessageParcel &data, MessageParcel &reply, AppIdentity &identity);
    int PreDisableLocationMock(MessageParcel &data, MessageParcel &reply, AppIdentity &identity);
    int PreSetMockedLocations(MessageParcel &data, MessageParcel &reply, AppIdentity &identity);
    int PreProxyForFreeze(MessageParcel &data, MessageParcel &reply, AppIdentity &identity);
    int PreResetAllProxy(MessageParcel &data, MessageParcel &reply, AppIdentity &identity);
    int PreReportLocation(MessageParcel &data, MessageParcel &reply, AppIdentity &identity);
    int PreRegisterLocatingRequiredDataCallback(MessageParcel &data, MessageParcel &reply, AppIdentity &identity);
    int PreUnregisterLocatingRequiredDataCallback(MessageParcel &data, MessageParcel &reply, AppIdentity &identity);
    int PreRegisterLocationError(MessageParcel &data, MessageParcel &reply, AppIdentity &identity);
    int PreUnregisterLocationError(MessageParcel &data, MessageParcel &reply, AppIdentity &identity);
    int PreReportLocationError(MessageParcel &data, MessageParcel &reply, AppIdentity &identity);

private:
    LocatorMsgHandleMap locatorHandleMap_;
    bool CheckRssProcessName(MessageParcel &reply, AppIdentity &identity);
    bool CheckLocationPermission(MessageParcel &reply, AppIdentity &identity);
    bool CheckSettingsPermission(MessageParcel &reply, AppIdentity &identity);
    bool CheckPreciseLocationPermissions(MessageParcel &reply, AppIdentity &identity);
    bool CheckLocationSwitchState(MessageParcel &reply);
    static void SaDumpInfo(std::string& result);
    bool RemoveUnloadTask(uint32_t code);
    bool PostUnloadTask(uint32_t code);
    void WriteLocationDenyReportEvent(uint32_t code, int errCode, MessageParcel &data, AppIdentity &identity);
    int DoProcessFenceRequest(
        LocatorInterfaceCode code, MessageParcel &data, MessageParcel &reply, AppIdentity &identity);
    void ConstructLocatorHandleMap();
    void ConstructLocatorEnhanceHandleMap();
    void ConstructLocatorMockHandleMap();
    void ConstructGeocodeHandleMap();
    void ConstructGnssHandleMap();
    void ConstructGnssEnhanceHandleMap();
    sptr<IRemoteObject::DeathRecipient> scanRecipient_ = new (std::nothrow) ScanCallbackDeathRecipient();
};

class SwitchCallbackDeathRecipient : public IRemoteObject::DeathRecipient {
public:
    void OnRemoteDied(const wptr<IRemoteObject> &remote) override;
    SwitchCallbackDeathRecipient();
    ~SwitchCallbackDeathRecipient() override;
};
} // namespace Location
} // namespace OHOS
#endif // LOCATOR_SKELETON_H
