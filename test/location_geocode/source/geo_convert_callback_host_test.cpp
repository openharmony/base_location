/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifdef FEATURE_GEOCODE_SUPPORT
#include "geo_convert_callback_host_test.h"

#include <sys/time.h>
#include "common_utils.h"
#include "location_log.h"
#include "i_geocode_callback.h"

#include "mock_geo_convert_callback_host.h"
#include <gtest/gtest.h>
#include "geo_convert_proxy.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Location {
void GeoConvertCallbackHostTest::SetUp()
{
}

void GeoConvertCallbackHostTest::TearDown()
{
}

HWTEST_F(GeoConvertCallbackHostTest, GeoConvertCallbackHostTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GeoConvertCallbackHostTest, GeoConvertCallbackHostTest001, TestSize.Level1";
    LBSLOGI(GEO_CONVERT, "[GeoConvertCallbackHostTest] GeoConvertCallbackHostTest001 begin");
    auto geoConvertCallvackHost = sptr<MockGeoConvertCallbackHost>(new (std::nothrow) MockGeoConvertCallbackHost());
    uint32_t code = 3;
    MessageParcel parcel;
    parcel.WriteInterfaceToken(GeoConvertProxy::GetDescriptor());
    MessageParcel reply;
    MessageOption option;
    geoConvertCallvackHost->OnRemoteRequest(GeoConvertCallbackHost::RECEIVE_GEOCODE_INFO_EVENT, parcel, reply, option);
    geoConvertCallvackHost->OnRemoteRequest(GeoConvertCallbackHost::ERROR_INFO_EVENT, parcel, reply, option);
    geoConvertCallvackHost->OnRemoteRequest(code, parcel, reply, option);
    LBSLOGI(GEO_CONVERT, "[GeoConvertCallbackHostTest] GeoConvertCallbackHostTest001 end");
}

HWTEST_F(GeoConvertCallbackHostTest, GeoConvertCallbackHostTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GeoConvertCallbackHostTest, GeoConvertCallbackHostTest002, TestSize.Level1";
    LBSLOGI(GEO_CONVERT, "[GeoConvertCallbackHostTest] GeoConvertCallbackHostTest002 begin");
    auto geoConvertCallvackHost = sptr<MockGeoConvertCallbackHost>(new (std::nothrow) MockGeoConvertCallbackHost());
    uint32_t code = 3;
    MessageParcel parcel;
    parcel.WriteInterfaceToken(GeoConvertProxy::GetDescriptor());
    MessageParcel reply;
    MessageOption option;
    geoConvertCallvackHost->OnRemoteRequest(GeoConvertCallbackHost::RECEIVE_GEOCODE_INFO_EVENT, parcel, reply, option);
    geoConvertCallvackHost->OnRemoteRequest(GeoConvertCallbackHost::ERROR_INFO_EVENT, parcel, reply, option);
    geoConvertCallvackHost->OnRemoteRequest(code, parcel, reply, option);
    LBSLOGI(GEO_CONVERT, "[GeoConvertCallbackHostTest] GeoConvertCallbackHostTest002 end");
}

HWTEST_F(GeoConvertCallbackHostTest, GeoConvertCallbackHostTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GeoConvertCallbackHostTest, GeoConvertCallbackHostTest003, TestSize.Level1";
    LBSLOGI(GEO_CONVERT, "[GeoConvertCallbackHostTest] GeoConvertCallbackHostTest003 begin");
    auto geoConvertCallvackHost = sptr<GeoConvertCallbackHost>(new (std::nothrow) GeoConvertCallbackHost());
    uint32_t code = 3;
    MessageParcel parcel;
    parcel.WriteInterfaceToken(GeoConvertProxy::GetDescriptor());
    MessageParcel reply;
    MessageOption option;
    geoConvertCallvackHost->OnRemoteRequest
        (GeoConvertCallbackHost::RECEIVE_GEOCODE_INFO_EVENT, parcel, reply, option);
    geoConvertCallvackHost->OnRemoteRequest
        (GeoConvertCallbackHost::ERROR_INFO_EVENT, parcel, reply, option);
    geoConvertCallvackHost->OnRemoteRequest(code, parcel, reply, option);
    LBSLOGI(GEO_CONVERT, "[GeoConvertCallbackHostTest] GeoConvertCallbackHostTest003 end");
}

HWTEST_F(GeoConvertCallbackHostTest, GeoConvertCallbackHostTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GeoConvertCallbackHostTest, GeoConvertCallbackHostTest004, TestSize.Level1";
    LBSLOGI(GEO_CONVERT, "[GeoConvertCallbackHostTest] GeoConvertCallbackHostTest004 begin");
    auto geoConvertCallvackHost = sptr<GeoConvertCallbackHost>(new (std::nothrow) GeoConvertCallbackHost());
    geoConvertCallvackHost->GetResult();
    LBSLOGI(GEO_CONVERT, "[GeoConvertCallbackHostTest] GeoConvertCallbackHostTest004 end");
}

HWTEST_F(GeoConvertCallbackHostTest, GeoConvertCallbackHostTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GeoConvertCallbackHostTest, GeoConvertCallbackHostTest005, TestSize.Level1";
    LBSLOGI(GEO_CONVERT, "[GeoConvertCallbackHostTest] GeoConvertCallbackHostTest005 begin");
    auto geoConvertCallvackHost = sptr<GeoConvertCallbackHost>(new (std::nothrow) GeoConvertCallbackHost());
    std::list<std::shared_ptr<GeoAddress>> results;
    geoConvertCallvackHost->OnResults(results);
    LBSLOGI(GEO_CONVERT, "[GeoConvertCallbackHostTest] GeoConvertCallbackHostTest005 end");
}

HWTEST_F(GeoConvertCallbackHostTest, GeoConvertCallbackHostTest006, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GeoConvertCallbackHostTest, GeoConvertCallbackHostTest006, TestSize.Level1";
    LBSLOGI(GEO_CONVERT, "[GeoConvertCallbackHostTest] GeoConvertCallbackHostTest006 begin");
    auto geoConvertCallvackHost = sptr<GeoConvertCallbackHost>(new (std::nothrow) GeoConvertCallbackHost());
    int errorCode = 1;
    geoConvertCallvackHost->OnErrorReport(errorCode);
    LBSLOGI(GEO_CONVERT, "[GeoConvertCallbackHostTest] GeoConvertCallbackHostTest006 end");
}
} // namespace Location
} // namespace OHOS
#endif // FEATURE_GEOCODE_SUPPORT