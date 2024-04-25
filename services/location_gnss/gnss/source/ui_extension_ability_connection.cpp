/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#ifdef FEATURE_GNSS_SUPPORT
#include "ui_extension_ability_connection.h"

#include "ability_connect_callback_interface.h"
#include "ability_manager_client.h"
#include "location_log.h"

constexpr int32_t SIGNAL_NUM = 3;

namespace OHOS {
namespace Location {
void UIExtensionAbilityConnection::OnAbilityConnectDone(const AppExecFwk::ElementName &element,
    const sptr<IRemoteObject> &remoteObject, int32_t resultCode)
{
    LBSLOGI(GNSS, "on ability connected");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInt32(SIGNAL_NUM);
    data.WriteString16(u"bundleName");
    data.WriteString16(Str8ToStr16(bundleName_));
    data.WriteString16(u"abilityName");
    data.WriteString16(Str8ToStr16(abilityName_));
    data.WriteString16(u"parameters");
    data.WriteString16(Str8ToStr16(commandStr_));

    int32_t errCode = remoteObject->SendRequest(IAbilityConnection::ON_ABILITY_CONNECT_DONE, data, reply, option);
    LBSLOGI(GNSS, "AbilityConnectionWrapperProxy::OnAbilityConnectDone result %{public}d", errCode);
}

void UIExtensionAbilityConnection::OnAbilityDisconnectDone(const AppExecFwk::ElementName &element,
    int32_t resultCode)
{
    LBSLOGI(GNSS, "on ability disconnected");
}
} // namespace Location
} // namespace OHOS
#endif // FEATURE_GNSS_SUPPORT
