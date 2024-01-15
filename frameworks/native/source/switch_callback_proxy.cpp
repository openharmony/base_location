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

#include "switch_callback_proxy.h"

#include "message_option.h"
#include "message_parcel.h"

#include "location_log.h"

namespace OHOS {
namespace Location {
SwitchCallbackProxy::SwitchCallbackProxy(const sptr<IRemoteObject> &impl)
    : IRemoteProxy<ISwitchCallback>(impl)
{
}

void SwitchCallbackProxy::OnSwitchChange(const int state)
{
    MessageParcel data;
    MessageParcel reply;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        return;
    }
    data.WriteInt32(state);
    MessageOption option = { MessageOption::TF_ASYNC };
    int error = Remote()->SendRequest(RECEIVE_SWITCH_STATE_EVENT, data, reply, option);
    LBSLOGD(SWITCH_CALLBACK, "SwitchCallbackProxy::OnSwitchChange Transact ErrCode = %{public}d", error);
}
} // namespace Location
} // namespace OHOS