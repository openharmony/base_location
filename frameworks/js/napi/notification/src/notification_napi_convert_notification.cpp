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

#include "notification_napi.h"
#include "ans_inner_errors.h"
#include "location_log.h"
#include "js_native_api.h"
#include "js_native_api_types.h"
#include "napi_common.h"
#include "napi_common_util.h"
#include "notification_action_button.h"
#include "notification_capsule.h"
#include "notification_constant.h"
#include "notification_local_live_view_content.h"
#include "notification_progress.h"
#include "notification_time.h"
#include "pixel_map_napi.h"

namespace OHOS {
namespace Location {
napi_value NotificationNapi::GetNotificationRequestDistributedOptions(const napi_env &env,
    const napi_value &value, NotificationRequest &request)
{
    LBSLOGD(NAPI_UTILS, "enter");
    napi_valuetype valuetype = napi_undefined;
    napi_value result = nullptr;
    bool hasProperty = false;

    // distributedOption?: DistributedOptions
    NAPI_CALL(env, napi_has_named_property(env, value, "distributedOption", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, value, "distributedOption", &result);
        NAPI_CALL(env, napi_typeof(env, result, &valuetype));
        if (valuetype != napi_object) {
            LBSLOGE(NAPI_UTILS, "Wrong argument type. Object expected.");
            return nullptr;
        }

        // isDistributed?: boolean
        if (GetNotificationIsDistributed(env, result, request) == nullptr) {
            return nullptr;
        }

        // supportDisplayDevices?: Array<string>
        if (GetNotificationSupportDisplayDevices(env, result, request) == nullptr) {
            return nullptr;
        }

        // supportOperateDevices?: Array<string>
        if (GetNotificationSupportOperateDevices(env, result, request) == nullptr) {
            return nullptr;
        }
    }

    return NapiGetNull(env);
}

napi_value NotificationNapi::GetNotificationIsDistributed(
    const napi_env &env, const napi_value &value, NotificationRequest &request)
{
    LBSLOGD(NAPI_UTILS, "enter");

    napi_valuetype valuetype = napi_undefined;
    napi_value result = nullptr;
    bool hasProperty = false;
    bool isDistributed = false;

    NAPI_CALL(env, napi_has_named_property(env, value, "isDistributed", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, value, "isDistributed", &result);
        NAPI_CALL(env, napi_typeof(env, result, &valuetype));
        if (valuetype != napi_boolean) {
            LBSLOGE(NAPI_UTILS, "Wrong argument type. Bool expected.");
            return nullptr;
        }
        napi_get_value_bool(env, result, &isDistributed);
        request.SetDistributed(isDistributed);
    }

    return NapiGetNull(env);
}
}
}
