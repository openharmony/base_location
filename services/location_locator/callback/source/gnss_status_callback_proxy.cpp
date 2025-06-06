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

#include "gnss_status_callback_proxy.h"

#include "message_option.h"

#include "location_log.h"

namespace OHOS {
namespace Location {
GnssStatusCallbackProxy::GnssStatusCallbackProxy(const sptr<IRemoteObject> &impl)
    : IRemoteProxy<IGnssStatusCallback>(impl)
{
}

void GnssStatusCallbackProxy::OnStatusChange(const std::unique_ptr<SatelliteStatus>& statusInfo)
{
    MessageParcel data;
    MessageParcel reply;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        return;
    }
    if (statusInfo != nullptr) {
        statusInfo->Marshalling(data);
    }
    MessageOption option = { MessageOption::TF_ASYNC };
    int error = Remote()->SendRequest(RECEIVE_STATUS_INFO_EVENT, data, reply, option);
    if (error != ERR_OK) {
        LBSLOGI(GNSS_STATUS_CALLBACK, "GnssStatusCallbackProxy::OnStatusChange Transact ErrCode = %{public}d", error);
    }
}
} // namespace Location
} // namespace OHOS