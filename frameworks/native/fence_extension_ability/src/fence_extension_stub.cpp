/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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

#include "fence_extension_stub.h"
#include "ipc_types.h"
#include "location_log.h"

namespace OHOS {
namespace Location {
int FenceExtensionStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    LBSLOGI(FENCE_EXTENSION, "stub on message:%{public}d", code);
    if (data.ReadInterfaceToken() != GetDescriptor()) {
        return ERR_TRANSACTION_FAILED;
    }

    switch (code) {
        case ON_FENCE_STATUS_CHANGE: {
            std::vector<std::string> dataKeys;
            data.ReadStringVector(&dataKeys);
            std::vector<std::string> dataValues;
            data.ReadStringVector(&dataValues);
            std::map<std::string, std::string> extraData;
            auto keysItr = dataKeys.begin();
            auto valuesItr = dataValues.begin();
            while (keysItr != dataKeys.end() && valuesItr != dataValues.end()) {
                extraData[*keysItr] = *valuesItr;
                keysItr++;
                valuesItr++;
            }
            FenceExtensionErrCode result = OnFenceStatusChange(extraData);
            LBSLOGI(FENCE_EXTENSION, "stub on message end:%{public}d", result);
            if (result != FenceExtensionErrCode::EXTENSION_SUCCESS) {
                return ERR_TRANSACTION_FAILED;
            }
            return ERR_NONE;
        }
        default:
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
    LBSLOGE(FENCE_EXTENSION, "stub on message trans failed");
    return ERR_TRANSACTION_FAILED;
}
}  // namespace Location
}  // namespace OHOS