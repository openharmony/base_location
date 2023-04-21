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

namespace OHOS {
namespace Location {
class LocatorAbilityStub : public IRemoteStub<ILocator> {
public:
    using LocatorMsgHandle = int (LocatorAbilityStub::*)(
        MessageParcel &data, MessageParcel &reply, AppIdentity &identity);
    using LocatorMsgHandleMap = std::map<int, LocatorMsgHandle>;

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
#endif
    int PreIsLocationPrivacyConfirmed(MessageParcel &data, MessageParcel &reply, AppIdentity &identity);
    int PreSetLocationPrivacyConfirmStatus(MessageParcel &data, MessageParcel &reply, AppIdentity &identity);
    int PreGetIsoCountryCode(MessageParcel &data, MessageParcel &reply, AppIdentity &identity);
    int PreEnableLocationMock(MessageParcel &data, MessageParcel &reply, AppIdentity &identity);
    int PreDisableLocationMock(MessageParcel &data, MessageParcel &reply, AppIdentity &identity);
    int PreSetMockedLocations(MessageParcel &data, MessageParcel &reply, AppIdentity &identity);
    int PreRegisterCountryCodeCallback(MessageParcel &data, MessageParcel &reply, AppIdentity &identity);
    int PreUnregisterCountryCodeCallback(MessageParcel &data, MessageParcel &reply, AppIdentity &identity);
    int PreProxyUidForFreeze(MessageParcel &data, MessageParcel &reply, AppIdentity &identity);
    int PreResetAllProxy(MessageParcel &data, MessageParcel &reply, AppIdentity &identity);
    int PreReportLocation(MessageParcel &data, MessageParcel &reply, AppIdentity &identity);

private:
    LocatorMsgHandleMap locatorHandleMap_;
    bool CheckLocationPermission(MessageParcel &reply, AppIdentity &identity);
    bool CheckSettingsPermission(MessageParcel &reply, AppIdentity &identity);
    bool CheckPreciseLocationPermissions(MessageParcel &reply, AppIdentity &identity);
    bool CheckLocationSwitchState(MessageParcel &reply);
    static void SaDumpInfo(std::string& result);
    bool UnloadLocatorSa();
};

class LocatorCallbackDeathRecipient : public IRemoteObject::DeathRecipient {
public:
    void OnRemoteDied(const wptr<IRemoteObject> &remote) override;
    LocatorCallbackDeathRecipient();
    ~LocatorCallbackDeathRecipient() override;
};

class SwitchCallbackDeathRecipient : public IRemoteObject::DeathRecipient {
public:
    void OnRemoteDied(const wptr<IRemoteObject> &remote) override;
    SwitchCallbackDeathRecipient();
    ~SwitchCallbackDeathRecipient() override;
};

class CountryCodeCallbackDeathRecipient : public IRemoteObject::DeathRecipient {
public:
    void OnRemoteDied(const wptr<IRemoteObject> &remote) override;
    CountryCodeCallbackDeathRecipient();
    ~CountryCodeCallbackDeathRecipient() override;
};
} // namespace Location
} // namespace OHOS
#endif // LOCATOR_SKELETON_H
