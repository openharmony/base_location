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

#ifndef COUNTRY_CODE_CALLBACK_HOST_H
#define COUNTRY_CODE_CALLBACK_HOST_H

#include <shared_mutex>
#include "common_utils.h"
#include "i_country_code_callback.h"
#include "iremote_stub.h"
#include "napi/native_api.h"
#include "uv.h"
#include "country_code.h"

namespace OHOS {
namespace Location {
class CountryCodeCallbackHost : public IRemoteStub<ICountryCodeCallback> {
public:
    CountryCodeCallbackHost();
    virtual ~CountryCodeCallbackHost();
    virtual int OnRemoteRequest(uint32_t code,
        MessageParcel& data, MessageParcel& reply, MessageOption& option) override;
    void OnCountryCodeChange(const std::shared_ptr<CountryCode>& country) override;
    bool Send(const std::shared_ptr<CountryCode>& country);
    void SetEnv(napi_env env);
    void SetCallback(napi_ref cb);
    void DeleteHandler();
private:
    napi_env env_;
    napi_ref handlerCb_;
    std::shared_mutex m_mutex;
};
} // namespace Location
} // namespace OHOS
#endif // COUNTRY_CODE_CALLBACK_HOST_H
