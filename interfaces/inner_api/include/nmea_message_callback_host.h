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

#ifndef NMEA_MESSAGE_CALLBACK_HOST_H
#define NMEA_MESSAGE_CALLBACK_HOST_H

#include <shared_mutex>
#include <string>
#include "i_nmea_message_callback.h"
#include "iremote_stub.h"
#include "napi_util.h"
#include "napi/native_api.h"

namespace OHOS {
namespace Location {
class NmeaMessageCallbackHost : public IRemoteStub<INmeaMessageCallback> {
public:
    NmeaMessageCallbackHost();
    virtual ~NmeaMessageCallbackHost();
    virtual int OnRemoteRequest(
        uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option) override;
    bool IsRemoteDied();
    bool Send(const std::string msg);
    napi_value PackResult(const std::string msg);
    void OnMessageChange(const std::string msg) override;
    void DeleteHandler();
    void UvQueueWork(uv_loop_s* loop, uv_work_t* work);

    napi_env m_env;
    napi_ref m_handlerCb;
    bool m_remoteDied;
    std::shared_mutex m_mutex;
};
} // namespace Location
} // namespace OHOS
#endif // NMEA_MESSAGE_CALLBACK_HOST_H
