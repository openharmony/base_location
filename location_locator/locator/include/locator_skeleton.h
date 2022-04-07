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

#ifndef OHOS_LOCATION_LOCATOR_SKELETON_H
#define OHOS_LOCATION_LOCATOR_SKELETON_H

#include "iremote_broker.h"
#include "iremote_object.h"
#include "iremote_proxy.h"
#include "iremote_stub.h"
#include "location.h"
#include "locator_proxy.h"
#include "request_config.h"
#include "i_locator_callback.h"

namespace OHOS {
namespace Location {
class LocatorAbilityStub : public IRemoteStub<ILocator> {
public:
    int32_t OnRemoteRequest(uint32_t code,
        MessageParcel &data, MessageParcel &reply, MessageOption &option) override;
    int32_t ProcessMsgRequirLocationPermission(uint32_t &code,
        MessageParcel &data, MessageParcel &reply, MessageOption &option);
    int32_t ProcessMsg(uint32_t &code,
        MessageParcel &data, MessageParcel &reply, MessageOption &option);
    int32_t ProcessMsgRequirSecureSettingsPermission(uint32_t &code,
        MessageParcel &data, MessageParcel &reply, MessageOption &option);
    void ParseDataAndStartLocating(MessageParcel& data, MessageParcel& reply, pid_t pid, pid_t uid);
    void ParseDataAndStopLocating(MessageParcel& data, MessageParcel& reply);
    int ReportStatus(MessageParcel& data, int type);
    void ParseDataAndStartCacheLocating(MessageParcel& data, MessageParcel& reply);
    void ParseDataAndStopCacheLocating(MessageParcel& data, MessageParcel& reply);
};

class LocatorCallbackDeathRecipient : public IRemoteObject::DeathRecipient {
public:
    virtual void OnRemoteDied(const wptr<IRemoteObject> &object);
    LocatorCallbackDeathRecipient();
    virtual ~LocatorCallbackDeathRecipient();
};

class SwitchCallbackDeathRecipient : public IRemoteObject::DeathRecipient {
public:
    virtual void OnRemoteDied(const wptr<IRemoteObject> &object);
    SwitchCallbackDeathRecipient();
    virtual ~SwitchCallbackDeathRecipient();
};
} // namespace Location
} // namespace OHOS
#endif // OHOS_LOCATION_LOCATOR_SKELETON_H