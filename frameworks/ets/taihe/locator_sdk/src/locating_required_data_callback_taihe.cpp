/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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
#include "locating_required_data_callback_taihe.h"

#include "ipc_skeleton.h"

#include "common_utils.h"
#include "location_log.h"
#include "util.h"

namespace OHOS {
namespace Location {
LocatingRequiredDataCallbackTaihe::LocatingRequiredDataCallbackTaihe()
{
}

LocatingRequiredDataCallbackTaihe::~LocatingRequiredDataCallbackTaihe()
{
}

int LocatingRequiredDataCallbackTaihe::OnRemoteRequest(
    uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option)
{
    LBSLOGD(LOCATING_DATA_CALLBACK, "LocatingRequiredDataCallbackTaihe::OnRemoteRequest!");
    if (data.ReadInterfaceToken() != GetDescriptor()) {
        LBSLOGE(LOCATING_DATA_CALLBACK, "invalid token.");
        return -1;
    }

    switch (code) {
        case RECEIVE_INFO_EVENT: {
            int cnt = data.ReadInt32();
            if (cnt >= 0 && cnt <= MAXIMUM_LOCATING_REQUIRED_DATAS) {
                std::vector<std::shared_ptr<LocatingRequiredData>> res;
                for (int i = 0; cnt > 0 && i < cnt; i++) {
                    res.push_back(LocatingRequiredData::Unmarshalling(data));
                }
                // update wifi info
                if (res.size() > 0 && res[0]->GetType() == LocatingRequiredDataType::WIFI) {
                    SetSingleResult(res);
                }
                OnLocatingDataChange(res);
            }
            break;
        }
        default: {
            IPCObjectStub::OnRemoteRequest(code, data, reply, option);
            break;
        }
    }
    return 0;
}

void LocatingRequiredDataCallbackTaihe::OnLocatingDataChange(
    const std::vector<std::shared_ptr<LocatingRequiredData>>& data)
{
    LBSLOGI(LOCATING_DATA_CALLBACK, "LocatingRequiredDataCallbackTaihe::OnLocatingDataChange");
    std::vector<::ohos::geoLocationManager::LocatingRequiredData> locatingRequiredDataList;
    Util::LocatingRequiredDataToTaihe(locatingRequiredDataList, data);
    if (callback_) {
        (*callback_)(::taihe::array<::ohos::geoLocationManager::LocatingRequiredData>{taihe::copy_data_t{},
            locatingRequiredDataList.data(), locatingRequiredDataList.size()});
    }
}

void LocatingRequiredDataCallbackTaihe::ClearSingleResult()
{
    singleResult_.clear();
}

void LocatingRequiredDataCallbackTaihe::SetSingleResult(
    std::vector<std::shared_ptr<LocatingRequiredData>> singleResult)
{
    singleResult_.assign(singleResult.begin(), singleResult.end());
}
}  // namespace Location
}  // namespace OHOS
