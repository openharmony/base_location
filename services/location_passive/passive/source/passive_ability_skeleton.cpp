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

#include "passive_ability_skeleton.h"

#include "ipc_object_stub.h"
#include "ipc_skeleton.h"
#include "message_option.h"
#include "message_parcel.h"

#include "common_utils.h"
#include "location.h"
#include "location_log.h"
#include "work_record.h"

namespace OHOS {
namespace Location {
int PassiveAbilityStub::OnRemoteRequest(uint32_t code,
    MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    pid_t callingPid = IPCSkeleton::GetCallingPid();
    pid_t callingUid = IPCSkeleton::GetCallingUid();
    LBSLOGI(PASSIVE, "OnRemoteRequest cmd = %{public}u, flags= %{public}d, pid= %{public}d, uid= %{public}d",
        code, option.GetFlags(), callingPid, callingUid);

    if (data.ReadInterfaceToken() != GetDescriptor()) {
        LBSLOGE(PASSIVE, "invalid token.");
        reply.WriteInt32(ERRCODE_SERVICE_UNAVAILABLE);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    if (callingUid != static_cast<pid_t>(getuid()) || callingPid != getpid()) {
        LBSLOGE(PASSIVE, "uid pid not match locationhub process.");
        reply.WriteInt32(ERRCODE_PERMISSION_DENIED);
        return ERRCODE_PERMISSION_DENIED;
    }

    int ret = ERRCODE_SUCCESS;
    switch (code) {
        case SEND_LOCATION_REQUEST: {
            std::unique_ptr<WorkRecord> workrecord = WorkRecord::Unmarshalling(data);
            reply.WriteInt32(SendLocationRequest(*workrecord));
            break;
        }
        case SET_ENABLE: {
            reply.WriteInt32(SetEnable(data.ReadBool()));
            break;
        }
        case ENABLE_LOCATION_MOCK: {
            reply.WriteInt32(EnableMock());
            break;
        }
        case DISABLE_LOCATION_MOCK: {
            reply.WriteInt32(DisableMock());
            break;
        }
        case SET_MOCKED_LOCATIONS: {
            SendMessage(code, data, reply);
            break;
        }
        default:
            ret = IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
    return ret;
}
} // namespace Location
} // namespace OHOS
