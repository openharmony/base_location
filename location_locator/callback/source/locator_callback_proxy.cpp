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

#include "locator_callback_proxy.h"

#include "ipc_skeleton.h"

#include "location_log.h"
#include "locator_background_proxy.h"

namespace OHOS {
namespace Location {
LocatorCallbackProxy::LocatorCallbackProxy(const sptr<IRemoteObject> &impl)
    : IRemoteProxy<ILocatorCallback>(impl)
{
    LBSLOGD(LOCATOR_CALLBACK, "construct");
}

void LocatorCallbackProxy::OnLocationReport(const std::unique_ptr<Location>& location)
{
    if (location == nullptr) {
        return;
    }
    MessageParcel data;
    MessageParcel reply;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        return;
    }
    location->Marshalling(data);
    MessageOption option;
    if (DelayedSingleton<LocatorBackgroundProxy>::GetInstance().get()->IsCallbackInProxy(this)) {
        // if callback is from locatorBackgroundProxy, should send sync message to wake up app
        option = { MessageOption::TF_SYNC };
        LBSLOGD(LOCATOR_CALLBACK, "OnLocationReport Transact TF_SYNC");
    } else {
        option = { MessageOption::TF_ASYNC };
    }
    int error = Remote()->SendRequest(ILocatorCallback::RECEIVE_LOCATION_INFO_EVENT, data, reply, option);
    LBSLOGD(LOCATOR_CALLBACK, "OnLocationReport Transact ErrCode = %{public}d", error);
}

void LocatorCallbackProxy::OnLocatingStatusChange(const int status)
{
    MessageParcel data;
    MessageParcel reply;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        return;
    }
    data.WriteInt32(status);
    MessageOption option = { MessageOption::TF_ASYNC };
    int error = Remote()->SendRequest(RECEIVE_LOCATION_STATUS_EVENT, data, reply, option);
    LBSLOGD(LOCATOR_CALLBACK, "OnLocatingStatusChange Transact ErrCode = %{public}d", error);
}

void LocatorCallbackProxy::OnErrorReport(const int errorCode)
{
    MessageParcel data;
    MessageParcel reply;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        return;
    }
    data.WriteInt32(errorCode);
    MessageOption option = { MessageOption::TF_ASYNC };
    int error = Remote()->SendRequest(RECEIVE_ERROR_INFO_EVENT, data, reply, option);
    LBSLOGD(LOCATOR_CALLBACK, "OnErrorReport:%{public}d, Transact ErrCode = %{public}d", errorCode, error);
}


int LocatorCallbackStub::OnRemoteRequest(uint32_t code,
    MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    if (data.ReadInterfaceToken() != GetDescriptor()) {
        LBSLOGE(LOCATOR_CALLBACK, "invalid token.");
        return -1;
    }
    pid_t callingPid = IPCSkeleton::GetCallingPid();
    pid_t callingUid = IPCSkeleton::GetCallingUid();
    LBSLOGI(LOCATOR_CALLBACK, "OnReceived cmd = %{public}u, flags= %{public}d, pid= %{public}d, uid= %{public}d",
        code, option.GetFlags(), callingPid, callingUid);
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}

void LocatorCallbackStub::OnLocationReport(const std::unique_ptr<Location>& location)
{
}

void LocatorCallbackStub::OnLocatingStatusChange(const int status)
{
}

void LocatorCallbackStub::OnErrorReport(const int errorCode)
{
}
} // namespace Location
} // namespace OHOS
