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

#include "network_ability_skeleton.h"

#include <vector>

#include "ipc_object_stub.h"
#include "ipc_skeleton.h"

#include "common_utils.h"
#include "location.h"
#include "location_log.h"
#include "location_mock_config.h"
#include "subability_common.h"
#include "work_record.h"

namespace OHOS {
namespace Location {
int NetworkAbilityStub::OnRemoteRequest(uint32_t code,
    MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    pid_t callingPid = IPCSkeleton::GetCallingPid();
    pid_t callingUid = IPCSkeleton::GetCallingUid();
    LBSLOGI(NETWORK, "OnRemoteRequest cmd = %{public}u, flags= %{public}d, pid= %{public}d, uid= %{public}d",
        code, option.GetFlags(), callingPid, callingUid);

    if (data.ReadInterfaceToken() != GetDescriptor()) {
        LBSLOGE(NETWORK, "invalid token.");
        return REPLY_CODE_EXCEPTION;
    }
    if (callingUid != static_cast<pid_t>(getuid()) || callingPid != getpid()) {
        LBSLOGE(NETWORK, "uid pid not match locationhub process.");
        return REPLY_CODE_EXCEPTION;
    }

    int ret = 0;
    switch (code) {
        case SEND_LOCATION_REQUEST: // fall through
        case SET_MOCKED_LOCATIONS: {
            SendMessage(code, data, reply);
            break;
        }
        case SET_ENABLE: {
            SetEnable(data.ReadBool());
            break;
        }
        case SELF_REQUEST: {
            SelfRequest(data.ReadBool());
            break;
        }
        case ENABLE_LOCATION_MOCK: {
            std::unique_ptr<LocationMockConfig> mockConfig = LocationMockConfig::Unmarshalling(data);
            LocationMockConfig config;
            config.Set(*mockConfig);
            bool result = EnableMock(config);
            reply.WriteBool(result);
            break;
        }
        case DISABLE_LOCATION_MOCK: {
            std::unique_ptr<LocationMockConfig> mockConfig = LocationMockConfig::Unmarshalling(data);
            LocationMockConfig config;
            config.Set(*mockConfig);
            bool result = DisableMock(config);
            reply.WriteBool(result);
            break;
        }
        default:
            ret = IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
    return ret;
}
} // namespace Location
} // namespace OHOS