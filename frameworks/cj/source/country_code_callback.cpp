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

#include "location_log.h"
#include "cj_lambda.h"
#include "country_code_callback.h"

namespace OHOS {
namespace GeoLocationManager {
CountryCodeCallback::CountryCodeCallback()
{
}

CountryCodeCallback::CountryCodeCallback(int64_t callbackId)
{
    this->callbackId_ = callbackId;
    auto cFunc = reinterpret_cast<void(*)(CJCountryCode countryCode)>(callbackId);
    callback_ = [ lambda = CJLambda::Create(cFunc)](const std::shared_ptr<Location::CountryCode>& country) ->
        void { lambda(CountryCodeToCJCountryCode(country)); };
}

CountryCodeCallback::~CountryCodeCallback()
{
}

int CountryCodeCallback::OnRemoteRequest(
    uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option)
{
    LBSLOGD(Location::COUNTRY_CODE_CALLBACK, "CountryCodeCallback::OnRemoteRequest!");
    if (data.ReadInterfaceToken() != GetDescriptor()) {
        LBSLOGE(Location::COUNTRY_CODE_CALLBACK, "invalid token.");
        return -1;
    }

    switch (code) {
        case Location::ICountryCodeCallback::COUNTRY_CODE_CHANGE_EVENT: {
            auto countryCodePtr = Location::CountryCode::Unmarshalling(data);
            OnCountryCodeChange(countryCodePtr);
            break;
        }
        default: {
            IPCObjectStub::OnRemoteRequest(code, data, reply, option);
            break;
        }
    }
    return 0;
}

void CountryCodeCallback::OnCountryCodeChange(const std::shared_ptr<Location::CountryCode>& country)
{
    LBSLOGD(Location::COUNTRY_CODE_CALLBACK, "CountryCodeCallback::OnCountryCodeChange");
    std::unique_lock<std::mutex> guard(mutex_);
    if (callback_ != nullptr) {
        callback_(country);
    }
}
}
}