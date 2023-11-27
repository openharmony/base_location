/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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

#include "locator_agent.h"
#include "locationhub_ipc_interface_code.h"
#include "location_sa_load_manager.h"
#include "system_ability_definition.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "location_log.h"

namespace OHOS {
namespace Location {
LocatorAgentManager::LocatorAgentManager()
{
    nmeaCallbackHost_ =
        sptr<NativeNmeaCallbackHost>(new (std::nothrow) NativeNmeaCallbackHost());
    gnssCallbackHost_ =
        sptr<NativeSvCallbackHost>(new (std::nothrow) NativeSvCallbackHost());
    locationCallbackHost_ =
        sptr<NativeLocationCallbackHost>(new (std::nothrow) NativeLocationCallbackHost());
}

LocatorAgentManager::~LocatorAgentManager()
{}

void LocatorAgentManager::StartGnssLocating(const LocationCallbackIfaces& callback)
{
    if (locationCallbackHost_ == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s callback is nullptr", __func__);
        return;
    }
    auto proxy = GetLocatorAgent();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return;
    }
    locationCallbackHost_->SetCallback(callback);
    auto locatorCallback = sptr<ILocatorCallback>(locationCallbackHost_);
    LocationErrCode ret = proxy->StartGnssLocating(locatorCallback);
    LBSLOGE(LOCATOR_STANDARD, "%{public}s ret = %{public}d", __func__, ret);
}

void LocatorAgentManager::StopGnssLocating()
{
    auto proxy = GetLocatorAgent();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return;
    }
    auto locatorCallback = sptr<ILocatorCallback>(locationCallbackHost_);
    LocationErrCode ret = proxy->StopGnssLocating(locatorCallback);
    LBSLOGE(LOCATOR_STANDARD, "%{public}s ret = %{public}d", __func__, ret);
}

void LocatorAgentManager::RegisterGnssStatusCallback(const SvStatusCallbackIfaces& callback)
{
    if (gnssCallbackHost_ == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s callback is nullptr", __func__);
        return;
    }
    auto proxy = GetLocatorAgent();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return;
    }
    gnssCallbackHost_->SetCallback(callback);
    auto gnssCallback = sptr<IGnssStatusCallback>(gnssCallbackHost_);
    LocationErrCode ret = proxy->RegisterGnssStatusCallback(gnssCallback);
    LBSLOGE(LOCATOR_STANDARD, "%{public}s ret = %{public}d", __func__, ret);
}

void LocatorAgentManager::UnregisterGnssStatusCallback()
{
    auto proxy = GetLocatorAgent();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return;
    }
    auto gnssCallback = sptr<IGnssStatusCallback>(gnssCallbackHost_);
    LocationErrCode ret = proxy->UnregisterGnssStatusCallback(gnssCallback);
    LBSLOGE(LOCATOR_STANDARD, "%{public}s ret = %{public}d", __func__, ret);
}

void LocatorAgentManager::RegisterNmeaMessageCallback(const GnssNmeaCallbackIfaces& callback)
{
    if (nmeaCallbackHost_ == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s callback is nullptr", __func__);
        return;
    }
    auto proxy = GetLocatorAgent();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return;
    }
    nmeaCallbackHost_->SetCallback(callback);
    auto nmeaCallback = sptr<INmeaMessageCallback>(nmeaCallbackHost_);
    LocationErrCode ret = proxy->RegisterNmeaMessageCallback(nmeaCallback);
    LBSLOGE(LOCATOR_STANDARD, "%{public}s ret = %{public}d", __func__, ret);
}

void LocatorAgentManager::UnregisterNmeaMessageCallback()
{
    auto proxy = GetLocatorAgent();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return;
    }
    auto nmeaCallback = sptr<INmeaMessageCallback>(nmeaCallbackHost_);
    LocationErrCode ret = proxy->UnregisterNmeaMessageCallback(nmeaCallback);
    LBSLOGE(LOCATOR_STANDARD, "%{public}s ret = %{public}d", __func__, ret);
}

sptr<LocatorAgent> LocatorAgentManager::GetLocatorAgent()
{
    std::unique_lock<std::mutex> lock(mutex_);
    if (client_ != nullptr) {
        LBSLOGI(LOCATOR_STANDARD, "get proxy success.");
        return client_;
    }

    sptr<ISystemAbilityManager> sam =
        SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (sam == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s: get samgr failed.", __func__);
        return nullptr;
    }

    sptr<IRemoteObject> obj = sam->CheckSystemAbility(LOCATION_LOCATOR_SA_ID);
    if (obj == nullptr) {
        // reload sa
        auto instance = DelayedSingleton<LocationSaLoadManager>::GetInstance();
        if (instance == nullptr ||
            instance->LoadLocationSa(LOCATION_LOCATOR_SA_ID) != ERRCODE_SUCCESS) {
            LBSLOGE(LOCATOR_STANDARD, "locator sa load failed.");
            return nullptr;
        }
    }

    obj = sam->CheckSystemAbility(LOCATION_LOCATOR_SA_ID);
    if (obj == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "get sa obj failed.");
        return nullptr;
    }

    recipient_ = sptr<LocatorAgentDeathRecipient>(new (std::nothrow) LocatorAgentDeathRecipient(*this));
    if ((obj->IsProxyObject()) && (!obj->AddDeathRecipient(recipient_))) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s: deathRecipient add failed.", __func__);
        return nullptr;
    }
    client_ = sptr<LocatorAgent>(new (std::nothrow) LocatorAgent(obj));
    return client_;
}

void LocatorAgentManager::ResetLocatorAgent(const wptr<IRemoteObject> &remote)
{
    std::unique_lock<std::mutex> lock(mutex_);
    if (remote == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s: remote is nullptr.", __func__);
        return;
    }
    if (client_ == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s: proxy is nullptr.", __func__);
        return;
    }
    if (remote.promote() != nullptr) {
        remote.promote()->RemoveDeathRecipient(recipient_);
    }
    client_ = nullptr;
}

LocatorAgent::LocatorAgent(const sptr<IRemoteObject> &impl)
    : IRemoteProxy<ILocator>(impl)
{
}

LocationErrCode LocatorAgent::StartGnssLocating(sptr<ILocatorCallback>& callback)
{
    if (callback == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s callback is nullptr", __func__);
        return ERRCODE_INVALID_PARAM;
    }
    auto requestConfig = std::make_unique<RequestConfig>();
    requestConfig->SetPriority(PRIORITY_FAST_FIRST_FIX);
    requestConfig->SetScenario(SCENE_UNSET);
    requestConfig->SetFixNumber(0);

    MessageParcel data;
    MessageParcel reply;
    data.WriteInterfaceToken(GetDescriptor());
    requestConfig->Marshalling(data);
    data.WriteObject<IRemoteObject>(callback->AsObject());
    return SendRequestToStub(static_cast<int>(LocatorInterfaceCode::START_LOCATING),
        data, reply);
}

LocationErrCode LocatorAgent::StopGnssLocating(sptr<ILocatorCallback>& callback)
{
    if (callback == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s callback is nullptr", __func__);
        return ERRCODE_INVALID_PARAM;
    }
    MessageParcel data;
    MessageParcel reply;
    data.WriteInterfaceToken(GetDescriptor());
    data.WriteObject<IRemoteObject>(callback->AsObject());
    return SendRequestToStub(static_cast<int>(LocatorInterfaceCode::STOP_LOCATING),
        data, reply);
}

LocationErrCode LocatorAgent::RegisterNmeaMessageCallback(const sptr<INmeaMessageCallback>& callback)
{
    if (callback == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s callback is nullptr", __func__);
        return ERRCODE_INVALID_PARAM;
    }
    MessageParcel data;
    MessageParcel reply;
    data.WriteInterfaceToken(GetDescriptor());
    data.WriteObject<IRemoteObject>(callback->AsObject());
    return SendRequestToStub(static_cast<int>(LocatorInterfaceCode::REG_NMEA_CALLBACK_V9),
        data, reply);
}

LocationErrCode LocatorAgent::UnregisterNmeaMessageCallback(const sptr<INmeaMessageCallback>& callback)
{
    if (callback == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s callback is nullptr", __func__);
        return ERRCODE_INVALID_PARAM;
    }
    MessageParcel data;
    MessageParcel reply;
    data.WriteInterfaceToken(GetDescriptor());
    data.WriteObject<IRemoteObject>(callback->AsObject());
    return SendRequestToStub(static_cast<int>(LocatorInterfaceCode::UNREG_NMEA_CALLBACK_V9),
        data, reply);
}

LocationErrCode LocatorAgent::RegisterGnssStatusCallback(const sptr<IGnssStatusCallback>& callback)
{
    if (callback == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s callback is nullptr", __func__);
        return ERRCODE_INVALID_PARAM;
    }
    MessageParcel data;
    MessageParcel reply;
    data.WriteInterfaceToken(GetDescriptor());
    data.WriteObject<IRemoteObject>(callback->AsObject());
    return SendRequestToStub(static_cast<int>(LocatorInterfaceCode::REG_GNSS_STATUS_CALLBACK),
        data, reply);
}

LocationErrCode LocatorAgent::UnregisterGnssStatusCallback(const sptr<IGnssStatusCallback>& callback)
{
    if (callback == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s callback is nullptr", __func__);
        return ERRCODE_INVALID_PARAM;
    }
    MessageParcel data;
    MessageParcel reply;
    data.WriteInterfaceToken(GetDescriptor());
    data.WriteObject<IRemoteObject>(callback->AsObject());
    return SendRequestToStub(static_cast<int>(LocatorInterfaceCode::UNREG_GNSS_STATUS_CALLBACK),
        data, reply);
}

LocationErrCode LocatorAgent::SendRequestToStub(const int msgId, MessageParcel& data, MessageParcel& reply)
{
    MessageOption option;
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s remote is null", __func__);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    int error = remote->SendRequest(msgId, data, reply, option);
    if (error != NO_ERROR) {
        LBSLOGE(LOCATOR_STANDARD,
            "msgid = %{public}d, SendRequestToStub error: %{public}d", msgId, error);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    return LocationErrCode(reply.ReadInt32());
}
}
}