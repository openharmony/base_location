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

#include "locating_required_data_callback_proxy.h"
#include "message_option.h"
#include "location_log.h"

namespace OHOS {
namespace Location {
LocatingRequiredDataCallbackProxy::LocatingRequiredDataCallbackProxy(const sptr<IRemoteObject> &impl)
    : IRemoteProxy<ILocatingRequiredDataCallback>(impl)
{
}

void LocatingRequiredDataCallbackProxy::OnLocatingDataChange(
    const std::vector<std::shared_ptr<LocatingRequiredData>>& data)
{
    MessageParcel dataParcel;
    MessageParcel reply;
    if (!dataParcel.WriteInterfaceToken(GetDescriptor())) {
        return;
    }
    dataParcel.WriteInt32(data.size());
    for (unsigned int i = 0; i < data.size(); i++) {
        data[i]->Marshalling(dataParcel);
    }
    MessageOption option = { MessageOption::TF_ASYNC };
    int error = Remote()->SendRequest(RECEIVE_INFO_EVENT, dataParcel, reply, option);
    LBSLOGD(LOCATING_DATA_CALLBACK,
        "LocatingRequiredDataCallbackProxy::OnLocatingDataChange Transact ErrCode = %{public}d", error);
}
} // namespace Location
} // namespace OHOS
