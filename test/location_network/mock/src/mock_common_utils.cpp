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

#include "mock_i_remote_object.h"
#include "gmock/gmock.h"
#include <gtest/gtest.h>
#include "location_log.h"
#include "common_utils.h"

namespace OHOS {
namespace Location {
using namespace testing;
using namespace testing::ext;
sptr<IRemoteObject> CommonUtils::GetRemoteObject(
    int abilityId, std::string deviceId)
{
    sptr<MockIRemoteObject> iRemoteObject =
      sptr<MockIRemoteObject>(new (std::nothrow) MockIRemoteObject());
    EXPECT_CALL(*iRemoteObject, SendRequest(_, _, _, _)).Times(1).WillRepeatedly(DoAll(Return(ERR_OK)));
    LBSLOGI(COMMON_UTILS, "[MOCK_COMMON_UTILS] after mock, GetRemoteObject will return mock obj");
    return iRemoteObject;
}
} // namespace Location
} // namespace OHOS