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

#include "country_code_callback_proxy.h"
#include "common_utils.h"
#include "ipc_skeleton.h"
#include "location_log.h"

namespace OHOS {
namespace Location {
CountryCodeCallbackProxy::CountryCodeCallbackProxy(const sptr<IRemoteObject> &impl)
    : IRemoteProxy<CountryCodeCallbackProxy>(impl)
{
}

void CountryCodeCallbackProxy::OnCountryCodeChange(const std::shared_ptr<CountryCode>& country)
{
    MessageParcel data;
    MessageParcel reply;
    if (country == nullptr) {
        LBSLOGI(COUNTRY_CODE_CALLBACK, "CountryCodeCallbackProxy::country is nullptr");
        return;
    }
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        return;
    }
    country->Marshalling(data);
    MessageOption option = { MessageOption::TF_ASYNC };
    int error = Remote()->SendRequest(COUNTRY_CODE_CHANGE_EVENT, data, reply, option);
    LBSLOGI(COUNTRY_CODE_CALLBACK, "CountryCodeCallbackProxy::OnCountryCodeChange Transact ErrCode = %{public}d", error);
}
} // namespace Location
} // namespace OHOS
