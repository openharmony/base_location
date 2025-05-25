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

#include "bluetooh_scan_result_callback_proxy.h"
#include "message_option.h"
#include "location_log.h"

namespace OHOS {
namespace Location {
BluetoohScanResultCallbackProxy::BluetoohScanResultCallbackProxy(const sptr<IRemoteObject> &impl)
    : IRemoteProxy<IBluetoothScanResultCallback>(impl)
{
}

void BluetoohScanResultCallbackProxy::OnBluetoothScanResultChange(
    const std::unique_ptr<BluetoothScanResult>& data)
{
    MessageParcel dataParcel;
    MessageParcel reply;
    if (!dataParcel.WriteInterfaceToken(GetDescriptor())) {
        return;
    }
    data->Marshalling(dataParcel);
    MessageOption option = { MessageOption::TF_ASYNC };
    int error = Remote()->SendRequest(RECEIVE_INFO_EVENT, dataParcel, reply, option);
    if (error != ERR_OK) {
        LBSLOGE(LOCATING_DATA_CALLBACK,
            "BluetoohScanResultCallbackProxy::OnBluetoothScanResultChange Transact ErrCode = %{public}d", error);
    }
}
} // namespace Location
} // namespace OHOS
