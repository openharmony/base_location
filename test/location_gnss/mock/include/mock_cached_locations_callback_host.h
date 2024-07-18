/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef MOCK_CACHED_LOCATIONS_CALLBACK_HOST_H
#define MOCK_CACHED_LOCATIONS_CALLBACK_HOST_H
#ifdef FEATURE_GNSS_SUPPORT

#include "gmock/gmock.h"

#include "message_option.h"
#include "message_parcel.h"

#include "cached_locations_callback_napi.h"
#include "location.h"

namespace OHOS {
namespace Location {
class MockCachedLocationsCallbackHost : public CachedLocationsCallbackNapi {
public:
    MockCachedLocationsCallbackHost() {}
    ~MockCachedLocationsCallbackHost() {}

    MOCK_METHOD(int, OnRemoteRequest, (uint32_t code, MessageParcel& data,
        MessageParcel& reply, MessageOption& option));
    MOCK_METHOD(void, OnCacheLocationsReport, (const std::vector<std::unique_ptr<Location>>& locations));
};
} // namespace Location
} // namespace OHOS
#endif // FEATURE_GNSS_SUPPORT
#endif // MOCK_CACHED_LOCATIONS_CALLBACK_HOST_H
