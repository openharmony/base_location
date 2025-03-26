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

#ifdef FEATURE_NETWORK_SUPPORT
#include "network_ability_skeleton.h"

#include <vector>

#include "ipc_object_stub.h"
#include "ipc_skeleton.h"

#include "common_utils.h"
#include "location.h"
#include "location_log.h"
#include "subability_common.h"
#include "work_record.h"
#include "locationhub_ipc_interface_code.h"
#include "location_data_rdb_manager.h"
#include "permission_manager.h"

namespace OHOS {
namespace Location {
void NetworkAbilityStub::InitNetworkMsgHandleMap()
{
    if (NetworkMsgHandleMap_.size() != 0) {
        return;
    }
    NetworkMsgHandleMap_[static_cast<uint32_t>(NetworkInterfaceCode::SEND_LOCATION_REQUEST)] =
        [this](MessageParcel &data, MessageParcel &reply, AppIdentity &identity) {
        return SendLocationRequestInner(data, reply, identity);
        };
    NetworkMsgHandleMap_[static_cast<uint32_t>(NetworkInterfaceCode::SET_MOCKED_LOCATIONS)] =
        [this](MessageParcel &data, MessageParcel &reply, AppIdentity &identity) {
        return SetMockLocationsInner(data, reply, identity);
        };
    NetworkMsgHandleMap_[static_cast<uint32_t>(NetworkInterfaceCode::SET_ENABLE)] =
        [this](MessageParcel &data, MessageParcel &reply, AppIdentity &identity) {
        return SetEnableInner(data, reply, identity);
        };
    NetworkMsgHandleMap_[static_cast<uint32_t>(NetworkInterfaceCode::ENABLE_LOCATION_MOCK)] =
        [this](MessageParcel &data, MessageParcel &reply, AppIdentity &identity) {
        return EnableMockInner(data, reply, identity);
        };
    NetworkMsgHandleMap_[static_cast<uint32_t>(NetworkInterfaceCode::DISABLE_LOCATION_MOCK)] =
        [this](MessageParcel &data, MessageParcel &reply, AppIdentity &identity) {
        return DisableMockInner(data, reply, identity);
        };
}

NetworkAbilityStub::NetworkAbilityStub()
{
    InitNetworkMsgHandleMap();
}

int NetworkAbilityStub::SendLocationRequestInner(MessageParcel &data, MessageParcel &reply, AppIdentity &identity)
{
    if (!PermissionManager::CheckCallingPermission(identity.GetUid(), identity.GetPid(), reply)) {
        return LOCATION_ERRCODE_PERMISSION_DENIED;
    }
    SendMessage(static_cast<uint32_t>(NetworkInterfaceCode::SEND_LOCATION_REQUEST), data, reply);
    isMessageRequest_ = true;
    return ERRCODE_SUCCESS;
}

int NetworkAbilityStub::SetMockLocationsInner(MessageParcel &data, MessageParcel &reply, AppIdentity &identity)
{
    if (!PermissionManager::CheckCallingPermission(identity.GetUid(), identity.GetPid(), reply)) {
        return LOCATION_ERRCODE_PERMISSION_DENIED;
    }
    SendMessage(static_cast<uint32_t>(NetworkInterfaceCode::SET_MOCKED_LOCATIONS), data, reply);
    isMessageRequest_ = true;
    return ERRCODE_SUCCESS;
}

int NetworkAbilityStub::SetEnableInner(MessageParcel &data, MessageParcel &reply, AppIdentity &identity)
{
    if (!PermissionManager::CheckCallingPermission(identity.GetUid(), identity.GetPid(), reply)) {
        return LOCATION_ERRCODE_PERMISSION_DENIED;
    }
    reply.WriteInt32(SetEnable(data.ReadBool()));
    return ERRCODE_SUCCESS;
}

int NetworkAbilityStub::EnableMockInner(MessageParcel &data, MessageParcel &reply, AppIdentity &identity)
{
    if (!PermissionManager::CheckCallingPermission(identity.GetUid(), identity.GetPid(), reply)) {
        return LOCATION_ERRCODE_PERMISSION_DENIED;
    }
    reply.WriteInt32(EnableMock());
    return ERRCODE_SUCCESS;
}

int NetworkAbilityStub::DisableMockInner(MessageParcel &data, MessageParcel &reply, AppIdentity &identity)
{
    if (!PermissionManager::CheckCallingPermission(identity.GetUid(), identity.GetPid(), reply)) {
        return LOCATION_ERRCODE_PERMISSION_DENIED;
    }
    reply.WriteInt32(DisableMock());
    return ERRCODE_SUCCESS;
}

int NetworkAbilityStub::OnRemoteRequest(uint32_t code,
    MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    pid_t callingPid = IPCSkeleton::GetCallingPid();
    pid_t callingUid = IPCSkeleton::GetCallingUid();
    AppIdentity identity;
    identity.SetPid(callingPid);
    identity.SetUid(callingUid);
    std::string currentTime = std::to_string(CommonUtils::GetCurrentTimeStamp());
    LBSLOGI(NETWORK,
        "OnRemoteRequest cmd=%{public}u,flags=%{public}d,pid=%{public}d,uid=%{public}d,time=%{public}s",
        code, option.GetFlags(), callingPid, callingUid, currentTime.c_str());

    if (data.ReadInterfaceToken() != GetDescriptor()) {
        LBSLOGE(NETWORK, "invalid token.");
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    CancelIdleState();
    int ret = ERRCODE_SUCCESS;
    isMessageRequest_ = false;
    auto handleFunc = NetworkMsgHandleMap_.find(code);
    if (handleFunc != NetworkMsgHandleMap_.end() && handleFunc->second != nullptr) {
        auto memberFunc = handleFunc->second;
        ret = memberFunc(data, reply, identity);
    } else {
        LBSLOGE(NETWORK, "OnReceived cmd = %{public}u, unsupport service.", code);
        ret = IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
    if (!isMessageRequest_) {
        UnloadNetworkSystemAbility();
    }
    return ret;
}

bool NetworkAbilityStub::CheckLocationSwitchState(MessageParcel &reply)
{
    if (LocationDataRdbManager::QuerySwitchState() != ENABLED) {
        LBSLOGE(NETWORK, "%{public}s: %{public}d switch state is off.", __func__, __LINE__);
        reply.WriteInt32(ERRCODE_SWITCH_OFF);
        return false;
    }
    return true;
}
} // namespace Location
} // namespace OHOS
#endif // FEATURE_NETWORK_SUPPORT
