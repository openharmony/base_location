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
napi_value NotificationNapi::GetNotificationRequestByNumber(
    const napi_env &env, const napi_value &value, NotificationRequest &request)
{
    LBSLOGD(NAPI_UTILS, "enter");
    // id?: number
    if (GetNotificationId(env, value, request) == nullptr) {
        return nullptr;
    }
    // deliveryTime?: number
    if (GetNotificationDeliveryTime(env, value, request) == nullptr) {
        return nullptr;
    }
    // autoDeletedTime?: number
    if (GetNotificationAutoDeletedTime(env, value, request) == nullptr) {
        return nullptr;
    }
    // color?: number
    if (GetNotificationColor(env, value, request) == nullptr) {
        return nullptr;
    }
    // badgeIconStyle?: number
    if (GetNotificationBadgeIconStyle(env, value, request) == nullptr) {
        return nullptr;
    }
    // badgeNumber?: number
    if (GetNotificationBadgeNumber(env, value, request) == nullptr) {
        return nullptr;
    }
    // notificationControlFlags?: number
    if (GetNotificationControlFlags(env, value, request) == nullptr) {
        return nullptr;
    }

    return NapiGetNull(env);
}

napi_value NotificationNapi::GetNotificationRequestByString(
    const napi_env &env, const napi_value &value, NotificationRequest &request)
{
    LBSLOGD(NAPI_UTILS, "enter");
    // classification?: string
    if (GetNotificationClassification(env, value, request) == nullptr) {
        return nullptr;
    }
    // statusBarText?: string
    if (GetNotificationStatusBarText(env, value, request) == nullptr) {
        return nullptr;
    }
    // label?: string
    if (GetNotificationLabel(env, value, request) == nullptr) {
        return nullptr;
    }
    // groupName?: string
    if (GetNotificationGroupName(env, value, request) == nullptr) {
        return nullptr;
    }
    // appMessageId?: string
    if (GetNotificationAppMessageId(env, value, request) == nullptr) {
        return nullptr;
    }
    return NapiGetNull(env);
}

napi_value NotificationNapi::GetNotificationRequestByBool(
    const napi_env &env, const napi_value &value, NotificationRequest &request)
{
    LBSLOGD(NAPI_UTILS, "enter");
    // isOngoing?: boolean
    if (GetNotificationIsOngoing(env, value, request) == nullptr) {
        return nullptr;
    }
    // isUnremovable?: boolean
    if (GetNotificationIsUnremovable(env, value, request) == nullptr) {
        return nullptr;
    }
    // tapDismissed?: boolean
    if (GetNotificationtapDismissed(env, value, request) == nullptr) {
        return nullptr;
    }
    // colorEnabled?: boolean
    if (GetNotificationColorEnabled(env, value, request) == nullptr) {
        return nullptr;
    }
    // isAlertOnce?: boolean
    if (GetNotificationIsAlertOnce(env, value, request) == nullptr) {
        return nullptr;
    }
    // isStopwatch?: boolean
    if (GetNotificationIsStopwatch(env, value, request) == nullptr) {
        return nullptr;
    }
    // isCountDown?: boolean
    if (GetNotificationIsCountDown(env, value, request) == nullptr) {
        return nullptr;
    }
    // showDeliveryTime?: boolean
    if (GetNotificationShowDeliveryTime(env, value, request) == nullptr) {
        return nullptr;
    }

    GetNotificationIsRemoveAllowed(env, value, request);

    return NapiGetNull(env);
}

napi_value NotificationNapi::GetNotificationRequestByCustom(
    const napi_env &env, const napi_value &value, NotificationRequest &request)
{
    LBSLOGD(NAPI_UTILS, "enter");
    // content: NotificationContent
    if (GetNotificationContent(env, value, request) == nullptr) {
        return nullptr;
    }
    // slotType?: notification.SlotType
    if (GetNotificationSlotType(env, value, request) == nullptr) {
        return nullptr;
    }
    // wantAgent?: WantAgent
    if (GetNotificationWantAgent(env, value, request) == nullptr) {
        return nullptr;
    }
    // extraInfo?: {[key: string]: any}
    if (GetNotificationExtraInfo(env, value, request) == nullptr) {
        return nullptr;
    }
    // removalWantAgent?: WantAgent
    if (GetNotificationRemovalWantAgent(env, value, request) == nullptr) {
        return nullptr;
    }
    // maxScreenWantAgent?: WantAgent
    if (GetNotificationMaxScreenWantAgent(env, value, request) == nullptr) {
        return nullptr;
    }
    // actionButtons?: Array<NotificationActionButton>
    if (GetNotificationActionButtons(env, value, request) == nullptr) {
        return nullptr;
    }
    // smallIcon?: image.PixelMap
    if (GetNotificationSmallIcon(env, value, request) == nullptr) {
        return nullptr;
    }
    // largeIcon?: image.PixelMap
    if (GetNotificationLargeIcon(env, value, request) == nullptr) {
        return nullptr;
    }
    // overlayIcon?: image.PixelMap
    if (GetNotificationOverlayIcon(env, value, request) == nullptr) {
        return nullptr;
    }
    // distributedOption?:DistributedOptions
    if (GetNotificationRequestDistributedOptions(env, value, request) == nullptr) {
        return nullptr;
    }
    // template?: NotificationTemplate
    if (GetNotificationTemplate(env, value, request) == nullptr) {
        return nullptr;
    }
    // representativeBundle?: BundleOption
    if (GetNotificationBundleOption(env, value, request) == nullptr) {
        return nullptr;
    }
    // unifiedGroupInfo?: NotificationUnifiedGroupInfo
    if (GetNotificationUnifiedGroupInfo(env, value, request) == nullptr) {
        return nullptr;
    }
    return NapiGetNull(env);
}

napi_value NotificationNapi::GetNotificationRequest(
    const napi_env &env, const napi_value &value, NotificationRequest &request)
{
    LBSLOGD(NAPI_UTILS, "enter");
    if (!GetNotificationRequestByNumber(env, value, request)) {
        return nullptr;
    }
    if (!GetNotificationRequestByString(env, value, request)) {
        return nullptr;
    }
    if (!GetNotificationRequestByBool(env, value, request)) {
        return nullptr;
    }
    if (!GetNotificationRequestByCustom(env, value, request)) {
        return nullptr;
    }
    return NapiGetNull(env);
}

napi_value NotificationNapi::GetNotificationSmallIcon(
    const napi_env &env, const napi_value &value, NotificationRequest &request)
{
    LBSLOGD(NAPI_UTILS, "enter");

    napi_valuetype valuetype = napi_undefined;
    napi_value result = nullptr;
    bool hasProperty = false;

    NAPI_CALL(env, napi_has_named_property(env, value, "smallIcon", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, value, "smallIcon", &result);
        NAPI_CALL(env, napi_typeof(env, result, &valuetype));
        if (valuetype != napi_object) {
            LBSLOGE(NAPI_UTILS, "Argument type is not object.");
            return nullptr;
        }
        std::shared_ptr<Media::PixelMap> pixelMap = nullptr;
        pixelMap = Media::PixelMapNapi::GetPixelMap(env, result);
        if (pixelMap == nullptr) {
            LBSLOGE(NAPI_UTILS, "Invalid object pixelMap");
            return nullptr;
        }
        request.SetLittleIcon(pixelMap);
    }

    return NapiGetNull(env);
}

napi_value NotificationNapi::GetNotificationLargeIcon(
    const napi_env &env, const napi_value &value, NotificationRequest &request)
{
    LBSLOGD(NAPI_UTILS, "enter");

    napi_valuetype valuetype = napi_undefined;
    napi_value result = nullptr;
    bool hasProperty = false;

    NAPI_CALL(env, napi_has_named_property(env, value, "largeIcon", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, value, "largeIcon", &result);
        NAPI_CALL(env, napi_typeof(env, result, &valuetype));
        if (valuetype != napi_object) {
            LBSLOGE(NAPI_UTILS, "Wrong argument type. Object expected.");
            return nullptr;
        }
        std::shared_ptr<Media::PixelMap> pixelMap = nullptr;
        pixelMap = Media::PixelMapNapi::GetPixelMap(env, result);
        if (pixelMap == nullptr) {
            LBSLOGE(NAPI_UTILS, "Invalid object pixelMap");
            return nullptr;
        }
        request.SetBigIcon(pixelMap);
    }

    return NapiGetNull(env);
}

napi_value NotificationNapi::GetNotificationOverlayIcon(
    const napi_env &env, const napi_value &value, NotificationRequest &request)
{
    LBSLOGD(NAPI_UTILS, "enter");

    napi_valuetype valuetype = napi_undefined;
    napi_value result = nullptr;
    bool hasProperty = false;

    NAPI_CALL(env, napi_has_named_property(env, value, "overlayIcon", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, value, "overlayIcon", &result);
        NAPI_CALL(env, napi_typeof(env, result, &valuetype));
        if (valuetype != napi_object) {
            LBSLOGE(NAPI_UTILS, "Wrong argument type. Object expected.");
            return nullptr;
        }
        std::shared_ptr<Media::PixelMap> pixelMap = nullptr;
        pixelMap = Media::PixelMapNapi::GetPixelMap(env, result);
        if (pixelMap == nullptr) {
            LBSLOGE(NAPI_UTILS, "Invalid object pixelMap");
            return nullptr;
        }
        request.SetOverlayIcon(pixelMap);
    }

    return NapiGetNull(env);
}

napi_value NotificationNapi::GetNotificationSupportDisplayDevices(
    const napi_env &env, const napi_value &value, NotificationRequest &request)
{
    LBSLOGD(NAPI_UTILS, "enter");

    bool isArray = false;
    bool hasProperty = false;
    napi_valuetype valuetype = napi_undefined;
    napi_value supportDisplayDevices = nullptr;
    size_t strLen = 0;
    uint32_t length = 0;

    NAPI_CALL(env, napi_has_named_property(env, value, "supportDisplayDevices", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, value, "supportDisplayDevices", &supportDisplayDevices);
        napi_is_array(env, supportDisplayDevices, &isArray);
        if (!isArray) {
            LBSLOGE(NAPI_UTILS, "Property supportDisplayDevices is expected to be an array.");
            return nullptr;
        }

        napi_get_array_length(env, supportDisplayDevices, &length);
        if (length == 0) {
            LBSLOGE(NAPI_UTILS, "The array is empty.");
            return nullptr;
        }
        std::vector<std::string> devices;
        for (size_t i = 0; i < length; i++) {
            napi_value line = nullptr;
            napi_get_element(env, supportDisplayDevices, i, &line);
            NAPI_CALL(env, napi_typeof(env, line, &valuetype));
            if (valuetype != napi_string) {
                LBSLOGE(NAPI_UTILS, "Wrong argument type. String expected.");
                return nullptr;
            }
            char str[STR_MAX_SIZE] = {0};
            NAPI_CALL(env, napi_get_value_string_utf8(env, line, str, STR_MAX_SIZE - 1, &strLen));
            devices.emplace_back(str);
            LBSLOGI(NAPI_UTILS, "supportDisplayDevices = %{public}s", str);
        }
        request.SetDevicesSupportDisplay(devices);
    }
    return NapiGetNull(env);
}

napi_value NotificationNapi::GetNotificationSupportOperateDevices(
    const napi_env &env, const napi_value &value, NotificationRequest &request)
{
    LBSLOGD(NAPI_UTILS, "enter");

    bool isArray = false;
    bool hasProperty = false;
    napi_valuetype valuetype = napi_undefined;
    napi_value supportOperateDevices = nullptr;
    size_t strLen = 0;
    uint32_t length = 0;

    NAPI_CALL(env, napi_has_named_property(env, value, "supportOperateDevices", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, value, "supportOperateDevices", &supportOperateDevices);
        napi_is_array(env, supportOperateDevices, &isArray);
        if (!isArray) {
            LBSLOGE(NAPI_UTILS, "Property supportOperateDevices is expected to be an array.");
            return nullptr;
        }

        napi_get_array_length(env, supportOperateDevices, &length);
        if (length == 0) {
            LBSLOGE(NAPI_UTILS, "The array is empty.");
            return nullptr;
        }
        std::vector<std::string> devices;
        for (size_t i = 0; i < length; i++) {
            napi_value line = nullptr;
            napi_get_element(env, supportOperateDevices, i, &line);
            NAPI_CALL(env, napi_typeof(env, line, &valuetype));
            if (valuetype != napi_string) {
                LBSLOGE(NAPI_UTILS, "Wrong argument type. String expected.");
                return nullptr;
            }
            char str[STR_MAX_SIZE] = {0};
            NAPI_CALL(env, napi_get_value_string_utf8(env, line, str, STR_MAX_SIZE - 1, &strLen));
            devices.emplace_back(str);
            LBSLOGI(NAPI_UTILS, "supportOperateDevices = %{public}s", str);
        }
        request.SetDevicesSupportOperate(devices);
    }

    return NapiGetNull(env);
}

napi_value NotificationNapi::GetNotificationId(
    const napi_env &env, const napi_value &value, NotificationRequest &request)
{
    LBSLOGD(NAPI_UTILS, "enter");

    napi_valuetype valuetype = napi_undefined;
    napi_value result = nullptr;
    bool hasProperty = false;
    int32_t notificationId = 0;

    NAPI_CALL(env, napi_has_named_property(env, value, "id", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, value, "id", &result);
        NAPI_CALL(env, napi_typeof(env, result, &valuetype));
        if (valuetype != napi_number) {
            LBSLOGE(NAPI_UTILS, "Wrong argument type. Number expected.");
            return nullptr;
        }
        napi_get_value_int32(env, result, &notificationId);
        request.SetNotificationId(notificationId);
        LBSLOGI(NAPI_UTILS, "notificationId = %{public}d", notificationId);
    } else {
        LBSLOGI(NAPI_UTILS, "default notificationId = 0");
        request.SetNotificationId(0);
    }

    return NapiGetNull(env);
}

napi_value NotificationNapi::GetNotificationSlotType(
    const napi_env &env, const napi_value &value, NotificationRequest &request)
{
    LBSLOGD(NAPI_UTILS, "enter");

    napi_valuetype valuetype = napi_undefined;
    napi_value result = nullptr;
    bool hasSlotType = false;
    bool hasNotificationSlotType = false;
    int32_t slotType = 0;

    NAPI_CALL(env, napi_has_named_property(env, value, "notificationSlotType", &hasNotificationSlotType));
    if (hasNotificationSlotType) {
        napi_get_named_property(env, value, "notificationSlotType", &result);
        NAPI_CALL(env, napi_typeof(env, result, &valuetype));
        if (valuetype != napi_number) {
            LBSLOGE(NAPI_UTILS, "Wrong argument type. Number expected.");
            return nullptr;
        }
        napi_get_value_int32(env, result, &slotType);

        NotificationConstant::SlotType outType = NotificationConstant::SlotType::OTHER;
        if (!AnsEnumUtil::SlotTypeJSToC(SlotType(slotType), outType)) {
            return nullptr;
        }
        request.SetSlotType(outType);
        LBSLOGI(NAPI_UTILS, "notificationSlotType = %{public}d", slotType);
        return NapiGetNull(env);
    }

    NAPI_CALL(env, napi_has_named_property(env, value, "slotType", &hasSlotType));
    if (hasSlotType) {
        napi_get_named_property(env, value, "slotType", &result);
        NAPI_CALL(env, napi_typeof(env, result, &valuetype));
        if (valuetype != napi_number) {
            LBSLOGE(NAPI_UTILS, "Wrong argument type. Number expected.");
            return nullptr;
        }
        napi_get_value_int32(env, result, &slotType);

        NotificationConstant::SlotType outType = NotificationConstant::SlotType::OTHER;
        if (!AnsEnumUtil::SlotTypeJSToC(SlotType(slotType), outType)) {
            return nullptr;
        }
        request.SetSlotType(outType);
        LBSLOGI(NAPI_UTILS, "slotType = %{public}d", slotType);
    } else {
        LBSLOGI(NAPI_UTILS, "default slotType = OTHER");
        request.SetSlotType(NotificationConstant::OTHER);
    }

    return NapiGetNull(env);
}

napi_value NotificationNapi::GetNotificationIsOngoing(
    const napi_env &env, const napi_value &value, NotificationRequest &request)
{
    LBSLOGD(NAPI_UTILS, "enter");

    napi_valuetype valuetype = napi_undefined;
    napi_value result = nullptr;
    bool hasProperty = false;
    bool isOngoing = false;

    NAPI_CALL(env, napi_has_named_property(env, value, "isOngoing", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, value, "isOngoing", &result);
        NAPI_CALL(env, napi_typeof(env, result, &valuetype));
        if (valuetype != napi_boolean) {
            LBSLOGE(NAPI_UTILS, "Wrong argument type. Bool expected.");
            return nullptr;
        }
        napi_get_value_bool(env, result, &isOngoing);
        request.SetInProgress(isOngoing);
    }

    return NapiGetNull(env);
}

napi_value NotificationNapi::GetNotificationIsUnremovable(
    const napi_env &env, const napi_value &value, NotificationRequest &request)
{
    LBSLOGD(NAPI_UTILS, "enter");

    napi_valuetype valuetype = napi_undefined;
    napi_value result = nullptr;
    bool hasProperty = false;
    bool isUnremovable = false;

    NAPI_CALL(env, napi_has_named_property(env, value, "isUnremovable", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, value, "isUnremovable", &result);
        NAPI_CALL(env, napi_typeof(env, result, &valuetype));
        if (valuetype != napi_boolean) {
            LBSLOGE(NAPI_UTILS, "Wrong argument type. Bool expected.");
            return nullptr;
        }
        napi_get_value_bool(env, result, &isUnremovable);
        request.SetUnremovable(isUnremovable);
    }

    return NapiGetNull(env);
}

napi_value NotificationNapi::GetNotificationDeliveryTime(
    const napi_env &env, const napi_value &value, NotificationRequest &request)
{
    LBSLOGD(NAPI_UTILS, "enter");

    napi_valuetype valuetype = napi_undefined;
    napi_value result = nullptr;
    bool hasProperty = false;
    int64_t deliveryTime = 0;

    NAPI_CALL(env, napi_has_named_property(env, value, "deliveryTime", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, value, "deliveryTime", &result);
        NAPI_CALL(env, napi_typeof(env, result, &valuetype));
        if (valuetype != napi_number) {
            LBSLOGE(NAPI_UTILS, "Wrong argument type. Number expected.");
            return nullptr;
        }
        napi_get_value_int64(env, result, &deliveryTime);
        request.SetDeliveryTime(deliveryTime);
    }

    return NapiGetNull(env);
}

napi_value NotificationNapi::GetNotificationtapDismissed(
    const napi_env &env, const napi_value &value, NotificationRequest &request)
{
    LBSLOGD(NAPI_UTILS, "enter");

    napi_valuetype valuetype = napi_undefined;
    napi_value result = nullptr;
    bool hasProperty = false;
    bool tapDismissed = true;

    NAPI_CALL(env, napi_has_named_property(env, value, "tapDismissed", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, value, "tapDismissed", &result);
        NAPI_CALL(env, napi_typeof(env, result, &valuetype));
        if (valuetype != napi_boolean) {
            LBSLOGE(NAPI_UTILS, "Wrong argument type. Bool expected.");
            return nullptr;
        }
        napi_get_value_bool(env, result, &tapDismissed);
        request.SetTapDismissed(tapDismissed);
    }

    return NapiGetNull(env);
}

napi_value NotificationNapi::GetNotificationWantAgent(
    const napi_env &env, const napi_value &value, NotificationRequest &request)
{
    LBSLOGD(NAPI_UTILS, "enter");

    bool hasProperty = false;
    AbilityRuntime::WantAgent::WantAgent *wantAgent = nullptr;
    napi_value result = nullptr;
    napi_valuetype valuetype = napi_undefined;

    NAPI_CALL(env, napi_has_named_property(env, value, "wantAgent", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, value, "wantAgent", &result);
        NAPI_CALL(env, napi_typeof(env, result, &valuetype));
        if (valuetype != napi_object) {
            LBSLOGE(NAPI_UTILS, "Wrong argument type. Object expected.");
            return nullptr;
        }
        napi_unwrap(env, result, (void **)&wantAgent);
        if (wantAgent == nullptr) {
            LBSLOGE(NAPI_UTILS, "Invalid object wantAgent");
            return nullptr;
        }
        std::shared_ptr<AbilityRuntime::WantAgent::WantAgent> sWantAgent =
            std::make_shared<AbilityRuntime::WantAgent::WantAgent>(*wantAgent);
        request.SetWantAgent(sWantAgent);
    }

    return NapiGetNull(env);
}

napi_value NotificationNapi::GetNotificationExtraInfo(
    const napi_env &env, const napi_value &value, NotificationRequest &request)
{
    LBSLOGD(NAPI_UTILS, "enter");

    napi_valuetype valuetype = napi_undefined;
    napi_value result = nullptr;
    bool hasProperty = false;

    NAPI_CALL(env, napi_has_named_property(env, value, "extraInfo", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, value, "extraInfo", &result);
        NAPI_CALL(env, napi_typeof(env, result, &valuetype));
        if (valuetype != napi_object) {
            LBSLOGE(NAPI_UTILS, "Wrong argument type. Object expected.");
            return nullptr;
        }
        AAFwk::WantParams wantParams;
        if (!OHOS::AppExecFwk::UnwrapWantParams(env, result, wantParams)) {
            return nullptr;
        }

        std::shared_ptr<AAFwk::WantParams> extras = std::make_shared<AAFwk::WantParams>(wantParams);
        request.SetAdditionalData(extras);
    }

    return NapiGetNull(env);
}

napi_value NotificationNapi::GetNotificationGroupName(
    const napi_env &env, const napi_value &value, NotificationRequest &request)
{
    LBSLOGD(NAPI_UTILS, "enter");

    napi_valuetype valuetype = napi_undefined;
    napi_value result = nullptr;
    bool hasProperty = false;
    size_t strLen = 0;

    NAPI_CALL(env, napi_has_named_property(env, value, "groupName", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, value, "groupName", &result);
        NAPI_CALL(env, napi_typeof(env, result, &valuetype));
        if (valuetype != napi_string) {
            LBSLOGE(NAPI_UTILS, "Wrong argument type. String expected.");
            return nullptr;
        }
        char str[STR_MAX_SIZE] = {0};
        NAPI_CALL(env, napi_get_value_string_utf8(env, result, str, STR_MAX_SIZE - 1, &strLen));
        request.SetGroupName(str);
    }

    return NapiGetNull(env);
}

napi_value NotificationNapi::GetNotificationRemovalWantAgent(
    const napi_env &env, const napi_value &value, NotificationRequest &request)
{
    LBSLOGD(NAPI_UTILS, "enter");

    bool hasProperty = false;
    AbilityRuntime::WantAgent::WantAgent *wantAgent = nullptr;
    napi_value result = nullptr;
    napi_valuetype valuetype = napi_undefined;

    NAPI_CALL(env, napi_has_named_property(env, value, "removalWantAgent", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, value, "removalWantAgent", &result);
        NAPI_CALL(env, napi_typeof(env, result, &valuetype));
        if (valuetype != napi_object) {
            LBSLOGE(NAPI_UTILS, "Wrong argument type. Object expected.");
            return nullptr;
        }
        napi_unwrap(env, result, (void **)&wantAgent);
        if (wantAgent == nullptr) {
            LBSLOGE(NAPI_UTILS, "Invalid object removalWantAgent");
            return nullptr;
        }
        std::shared_ptr<AbilityRuntime::WantAgent::WantAgent> removeWantAgent =
            std::make_shared<AbilityRuntime::WantAgent::WantAgent>(*wantAgent);
        if (removeWantAgent->GetPendingWant() != nullptr && (uint32_t)removeWantAgent->GetPendingWant()->GetType(
            removeWantAgent->GetPendingWant()->GetTarget()) >= OPERATION_MAX_TYPE) {
            request.SetRemovalWantAgent(removeWantAgent);
        }
    }

    return NapiGetNull(env);
}

napi_value NotificationNapi::GetNotificationMaxScreenWantAgent(
    const napi_env &env, const napi_value &value, NotificationRequest &request)
{
    LBSLOGD(NAPI_UTILS, "enter");

    bool hasProperty = false;
    AbilityRuntime::WantAgent::WantAgent *wantAgent = nullptr;
    napi_value result = nullptr;
    napi_valuetype valuetype = napi_undefined;

    NAPI_CALL(env, napi_has_named_property(env, value, "maxScreenWantAgent", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, value, "maxScreenWantAgent", &result);
        NAPI_CALL(env, napi_typeof(env, result, &valuetype));
        if (valuetype != napi_object) {
            LBSLOGE(NAPI_UTILS, "Wrong argument type. Object expected.");
            return nullptr;
        }
        napi_unwrap(env, result, (void **)&wantAgent);
        if (wantAgent == nullptr) {
            LBSLOGE(NAPI_UTILS, "Invalid object maxScreenWantAgent");
            return nullptr;
        }
        std::shared_ptr<AbilityRuntime::WantAgent::WantAgent> maxScreenWantAgent =
            std::make_shared<AbilityRuntime::WantAgent::WantAgent>(*wantAgent);
        request.SetMaxScreenWantAgent(maxScreenWantAgent);
    }

    return NapiGetNull(env);
}

napi_value NotificationNapi::GetNotificationAutoDeletedTime(
    const napi_env &env, const napi_value &value, NotificationRequest &request)
{
    LBSLOGD(NAPI_UTILS, "enter");

    napi_valuetype valuetype = napi_undefined;
    napi_value result = nullptr;
    bool hasProperty = false;
    int64_t autoDeletedTime = 0;

    NAPI_CALL(env, napi_has_named_property(env, value, "autoDeletedTime", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, value, "autoDeletedTime", &result);
        NAPI_CALL(env, napi_typeof(env, result, &valuetype));
        if (valuetype != napi_number) {
            LBSLOGE(NAPI_UTILS, "Wrong argument type. Number expected.");
            return nullptr;
        }
        napi_get_value_int64(env, result, &autoDeletedTime);
        request.SetAutoDeletedTime(autoDeletedTime);
    }

    return NapiGetNull(env);
}

napi_value NotificationNapi::GetNotificationClassification(
    const napi_env &env, const napi_value &value, NotificationRequest &request)
{
    LBSLOGD(NAPI_UTILS, "enter");

    napi_valuetype valuetype = napi_undefined;
    napi_value result = nullptr;
    bool hasProperty = false;
    size_t strLen = 0;

    NAPI_CALL(env, napi_has_named_property(env, value, "classification", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, value, "classification", &result);
        NAPI_CALL(env, napi_typeof(env, result, &valuetype));
        if (valuetype != napi_string) {
            LBSLOGE(NAPI_UTILS, "Wrong argument type. String expected.");
            return nullptr;
        }
        char str[STR_MAX_SIZE] = {0};
        NAPI_CALL(env, napi_get_value_string_utf8(env, result, str, STR_MAX_SIZE - 1, &strLen));
        request.SetClassification(str);
    }

    return NapiGetNull(env);
}

napi_value NotificationNapi::GetNotificationAppMessageId(
    const napi_env &env, const napi_value &value, NotificationRequest &request)
{
    bool hasProperty = false;
    NAPI_CALL(env, napi_has_named_property(env, value, "appMessageId", &hasProperty));
    if (!hasProperty) {
        return NapiGetNull(env);
    }

    auto appMessageIdValue = AppExecFwk::GetPropertyValueByPropertyName(env, value, "appMessageId", napi_string);
    if (appMessageIdValue == nullptr) {
        LBSLOGE(NAPI_UTILS, "Wrong argument type. String expected.");
        return nullptr;
    }

    std::string appMessageId = AppExecFwk::UnwrapStringFromJS(env, appMessageIdValue);
    request.SetAppMessageId(appMessageId);
    return NapiGetNull(env);
}

napi_value NotificationNapi::GetNotificationColor(
    const napi_env &env, const napi_value &value, NotificationRequest &request)
{
    LBSLOGD(NAPI_UTILS, "enter");

    napi_valuetype valuetype = napi_undefined;
    napi_value result = nullptr;
    bool hasProperty = false;
    int32_t color = 0;

    NAPI_CALL(env, napi_has_named_property(env, value, "color", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, value, "color", &result);
        NAPI_CALL(env, napi_typeof(env, result, &valuetype));
        if (valuetype != napi_number) {
            LBSLOGE(NAPI_UTILS, "Wrong argument type. Number expected.");
            return nullptr;
        }
        napi_get_value_int32(env, result, &color);
        if (color < 0) {
            LBSLOGE(NAPI_UTILS, "Wrong argument type. Natural number expected.");
            return nullptr;
        }
        request.SetColor(color);
    }

    return NapiGetNull(env);
}

napi_value NotificationNapi::GetNotificationColorEnabled(
    const napi_env &env, const napi_value &value, NotificationRequest &request)
{
    LBSLOGD(NAPI_UTILS, "enter");

    napi_valuetype valuetype = napi_undefined;
    napi_value result = nullptr;
    bool hasProperty = false;
    bool colorEnabled = false;

    NAPI_CALL(env, napi_has_named_property(env, value, "colorEnabled", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, value, "colorEnabled", &result);
        NAPI_CALL(env, napi_typeof(env, result, &valuetype));
        if (valuetype != napi_boolean) {
            LBSLOGE(NAPI_UTILS, "Wrong argument type. Bool expected.");
            return nullptr;
        }
        napi_get_value_bool(env, result, &colorEnabled);
        request.SetColorEnabled(colorEnabled);
    }

    return NapiGetNull(env);
}

napi_value NotificationNapi::GetNotificationIsAlertOnce(
    const napi_env &env, const napi_value &value, NotificationRequest &request)
{
    LBSLOGD(NAPI_UTILS, "enter");

    napi_valuetype valuetype = napi_undefined;
    napi_value result = nullptr;
    bool hasProperty = false;
    bool isAlertOnce = false;

    NAPI_CALL(env, napi_has_named_property(env, value, "isAlertOnce", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, value, "isAlertOnce", &result);
        NAPI_CALL(env, napi_typeof(env, result, &valuetype));
        if (valuetype != napi_boolean) {
            LBSLOGE(NAPI_UTILS, "Wrong argument type. Bool expected.");
            return nullptr;
        }
        napi_get_value_bool(env, result, &isAlertOnce);
        request.SetAlertOneTime(isAlertOnce);
    }

    return NapiGetNull(env);
}

napi_value NotificationNapi::GetNotificationIsStopwatch(
    const napi_env &env, const napi_value &value, NotificationRequest &request)
{
    LBSLOGD(NAPI_UTILS, "enter");

    napi_valuetype valuetype = napi_undefined;
    napi_value result = nullptr;
    bool hasProperty = false;
    bool isStopwatch = false;

    NAPI_CALL(env, napi_has_named_property(env, value, "isStopwatch", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, value, "isStopwatch", &result);
        NAPI_CALL(env, napi_typeof(env, result, &valuetype));
        if (valuetype != napi_boolean) {
            LBSLOGE(NAPI_UTILS, "Wrong argument type. Bool expected.");
            return nullptr;
        }
        napi_get_value_bool(env, result, &isStopwatch);
        request.SetShowStopwatch(isStopwatch);
    }

    return NapiGetNull(env);
}

napi_value NotificationNapi::GetNotificationIsCountDown(
    const napi_env &env, const napi_value &value, NotificationRequest &request)
{
    LBSLOGD(NAPI_UTILS, "enter");

    napi_valuetype valuetype = napi_undefined;
    napi_value result = nullptr;
    bool hasProperty = false;
    bool isCountDown = false;

    NAPI_CALL(env, napi_has_named_property(env, value, "isCountDown", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, value, "isCountDown", &result);
        NAPI_CALL(env, napi_typeof(env, result, &valuetype));
        if (valuetype != napi_boolean) {
            LBSLOGE(NAPI_UTILS, "Wrong argument type. Bool expected.");
            return nullptr;
        }
        napi_get_value_bool(env, result, &isCountDown);
        request.SetCountdownTimer(isCountDown);
    }

    return NapiGetNull(env);
}

napi_value NotificationNapi::GetNotificationStatusBarText(
    const napi_env &env, const napi_value &value, NotificationRequest &request)
{
    LBSLOGD(NAPI_UTILS, "enter");

    napi_valuetype valuetype = napi_undefined;
    napi_value result = nullptr;
    bool hasProperty = false;
    size_t strLen = 0;

    NAPI_CALL(env, napi_has_named_property(env, value, "statusBarText", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, value, "statusBarText", &result);
        NAPI_CALL(env, napi_typeof(env, result, &valuetype));
        if (valuetype != napi_string) {
            LBSLOGE(NAPI_UTILS, "Wrong argument type. String expected.");
            return nullptr;
        }
        char str[STR_MAX_SIZE] = {0};
        NAPI_CALL(env, napi_get_value_string_utf8(env, result, str, STR_MAX_SIZE - 1, &strLen));
        request.SetStatusBarText(str);
    }

    return NapiGetNull(env);
}

napi_value NotificationNapi::GetNotificationLabel(
    const napi_env &env, const napi_value &value, NotificationRequest &request)
{
    LBSLOGD(NAPI_UTILS, "enter");

    napi_valuetype valuetype = napi_undefined;
    napi_value result = nullptr;
    bool hasProperty = false;
    size_t strLen = 0;

    NAPI_CALL(env, napi_has_named_property(env, value, "label", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, value, "label", &result);
        NAPI_CALL(env, napi_typeof(env, result, &valuetype));
        if (valuetype != napi_string) {
            LBSLOGE(NAPI_UTILS, "Wrong argument type. String expected.");
            return nullptr;
        }
        char str[STR_MAX_SIZE] = {0};
        NAPI_CALL(env, napi_get_value_string_utf8(env, result, str, STR_MAX_SIZE - 1, &strLen));
        request.SetLabel(str);
    }

    return NapiGetNull(env);
}

napi_value NotificationNapi::GetNotificationBadgeIconStyle(
    const napi_env &env, const napi_value &value, NotificationRequest &request)
{
    LBSLOGD(NAPI_UTILS, "enter");

    napi_valuetype valuetype = napi_undefined;
    napi_value result = nullptr;
    bool hasProperty = false;
    int32_t badgeIconStyle = 0;

    NAPI_CALL(env, napi_has_named_property(env, value, "badgeIconStyle", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, value, "badgeIconStyle", &result);
        NAPI_CALL(env, napi_typeof(env, result, &valuetype));
        if (valuetype != napi_number) {
            LBSLOGE(NAPI_UTILS, "Wrong argument type. Number expected.");
            return nullptr;
        }
        napi_get_value_int32(env, result, &badgeIconStyle);
        request.SetBadgeIconStyle(static_cast<NotificationRequest::BadgeStyle>(badgeIconStyle));
    }

    return NapiGetNull(env);
}

napi_value NotificationNapi::GetNotificationShowDeliveryTime(
    const napi_env &env, const napi_value &value, NotificationRequest &request)
{
    LBSLOGD(NAPI_UTILS, "enter");

    napi_valuetype valuetype = napi_undefined;
    napi_value result = nullptr;
    bool hasProperty = false;
    bool showDeliveryTime = false;

    NAPI_CALL(env, napi_has_named_property(env, value, "showDeliveryTime", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, value, "showDeliveryTime", &result);
        NAPI_CALL(env, napi_typeof(env, result, &valuetype));
        if (valuetype != napi_boolean) {
            LBSLOGE(NAPI_UTILS, "Wrong argument type. Bool expected.");
            return nullptr;
        }
        napi_get_value_bool(env, result, &showDeliveryTime);
        request.SetShowDeliveryTime(showDeliveryTime);
    }

    return NapiGetNull(env);
}

napi_value NotificationNapi::GetNotificationIsRemoveAllowed(
    const napi_env &env, const napi_value &value, NotificationRequest &request)
{
    LBSLOGD(NAPI_UTILS, "enter");

    napi_valuetype valuetype = napi_undefined;
    napi_value result = nullptr;
    bool hasProperty = false;
    bool isRemoveAllowed = true;

    NAPI_CALL(env, napi_has_named_property(env, value, "isRemoveAllowed", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, value, "isRemoveAllowed", &result);
        NAPI_CALL(env, napi_typeof(env, result, &valuetype));
        if (valuetype != napi_boolean) {
            LBSLOGE(NAPI_UTILS, "Wrong argument type. Bool expected.");
            return nullptr;
        }
        napi_get_value_bool(env, result, &isRemoveAllowed);
        request.SetRemoveAllowed(isRemoveAllowed);
    }

    return NapiGetNull(env);
}

napi_value NotificationNapi::GetNotificationActionButtons(
    const napi_env &env, const napi_value &value, NotificationRequest &request)
{
    LBSLOGD(NAPI_UTILS, "enter");

    bool isArray = false;
    napi_valuetype valuetype = napi_undefined;
    napi_value actionButtons = nullptr;
    uint32_t length = 0;
    bool hasProperty = false;

    napi_has_named_property(env, value, "actionButtons", &hasProperty);
    if (!hasProperty) {
        return NotificationNapi::NapiGetNull(env);
    }

    request.SetIsCoverActionButtons(true);
    napi_get_named_property(env, value, "actionButtons", &actionButtons);
    napi_is_array(env, actionButtons, &isArray);
    if (!isArray) {
        LBSLOGE(NAPI_UTILS, "Property actionButtons is expected to be an array.");
        return nullptr;
    }
    napi_get_array_length(env, actionButtons, &length);
    if (length == 0) {
        LBSLOGI(NAPI_UTILS, "The array is empty.");
        return NotificationNapi::NapiGetNull(env);
    }
    for (size_t i = 0; i < length; i++) {
        napi_value actionButton = nullptr;
        napi_get_element(env, actionButtons, i, &actionButton);
        NAPI_CALL(env, napi_typeof(env, actionButton, &valuetype));
        if (valuetype != napi_object) {
            LBSLOGE(NAPI_UTILS, "Wrong argument type. Object expected.");
            return nullptr;
        }

        std::shared_ptr<NotificationActionButton> pActionButton = nullptr;
        if (GetNotificationActionButtonsDetailed(env, actionButton, pActionButton) == nullptr) {
            return nullptr;
        }
        request.AddActionButton(pActionButton);
    }

    return NapiGetNull(env);
}

napi_value NotificationNapi::GetNotificationActionButtonsDetailed(
    const napi_env &env, const napi_value &actionButton, std::shared_ptr<NotificationActionButton> &pActionButton)
{
    LBSLOGD(NAPI_UTILS, "enter");

    if (!GetNotificationActionButtonsDetailedBasicInfo(env, actionButton, pActionButton)) {
        return nullptr;
    }
    if (!GetNotificationActionButtonsDetailedByExtras(env, actionButton, pActionButton)) {
        return nullptr;
    }
    if (!GetNotificationUserInput(env, actionButton, pActionButton)) {
        return nullptr;
    }
    return NapiGetNull(env);
}

napi_value NotificationNapi::GetNotificationActionButtonsDetailedBasicInfo(
    const napi_env &env, const napi_value &actionButton, std::shared_ptr<NotificationActionButton> &pActionButton)
{
    LBSLOGD(NAPI_UTILS, "enter");
    napi_valuetype valuetype = napi_undefined;
    bool hasProperty = false;
    char str[STR_MAX_SIZE] = {0};
    size_t strLen = 0;
    napi_value value = nullptr;
    std::string title;
    AbilityRuntime::WantAgent::WantAgent *wantAgentPtr = nullptr;
    std::shared_ptr<Media::PixelMap> pixelMap = nullptr;
    std::shared_ptr<AbilityRuntime::WantAgent::WantAgent> wantAgent;

    // title: string
    NAPI_CALL(env, napi_has_named_property(env, actionButton, "title", &hasProperty));
    if (!hasProperty) {
        LBSLOGE(NAPI_UTILS, "Property title expected.");
        return nullptr;
    }
    napi_get_named_property(env, actionButton, "title", &value);
    NAPI_CALL(env, napi_typeof(env, value, &valuetype));
    if (valuetype != napi_string) {
        LBSLOGE(NAPI_UTILS, "Wrong argument type. String expected.");
        return nullptr;
    }
    NAPI_CALL(env, napi_get_value_string_utf8(env, value, str, STR_MAX_SIZE - 1, &strLen));
    title = str;

    // wantAgent: WantAgent
    NAPI_CALL(env, napi_has_named_property(env, actionButton, "wantAgent", &hasProperty));
    if (!hasProperty) {
        LBSLOGE(NAPI_UTILS, "Property wantAgent expected.");
        return nullptr;
    }
    napi_get_named_property(env, actionButton, "wantAgent", &value);
    NAPI_CALL(env, napi_typeof(env, value, &valuetype));
    if (valuetype != napi_object) {
        LBSLOGE(NAPI_UTILS, "Wrong argument type. Object expected.");
        return nullptr;
    }
    napi_unwrap(env, value, (void **)&wantAgentPtr);
    if (wantAgentPtr == nullptr) {
        LBSLOGE(NAPI_UTILS, "Invalid object wantAgent");
        return nullptr;
    }
    wantAgent = std::make_shared<AbilityRuntime::WantAgent::WantAgent>(*wantAgentPtr);

    // icon?: image.PixelMap
    NAPI_CALL(env, napi_has_named_property(env, actionButton, "icon", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, actionButton, "icon", &value);
        NAPI_CALL(env, napi_typeof(env, value, &valuetype));
        if (valuetype != napi_object) {
            LBSLOGE(NAPI_UTILS, "Wrong argument type. Object expected.");
            return nullptr;
        }
        pixelMap = Media::PixelMapNapi::GetPixelMap(env, value);
        if (pixelMap == nullptr) {
            LBSLOGE(NAPI_UTILS, "Invalid object pixelMap");
            return nullptr;
        }
    }
    pActionButton = NotificationActionButton::Create(pixelMap, title, wantAgent);

    return NapiGetNull(env);
}

napi_value NotificationNapi::GetNotificationActionButtonsDetailedByExtras(
    const napi_env &env, const napi_value &actionButton, std::shared_ptr<NotificationActionButton> &pActionButton)
{
    LBSLOGD(NAPI_UTILS, "enter");

    napi_valuetype valuetype = napi_undefined;
    napi_value result = nullptr;
    bool hasProperty = false;

    if (!pActionButton) {
        LBSLOGE(NAPI_UTILS, "pActionButton is nullptr");
        return nullptr;
    }

    // extras?: {[key: string]: any}
    NAPI_CALL(env, napi_has_named_property(env, actionButton, "extras", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, actionButton, "extras", &result);
        NAPI_CALL(env, napi_typeof(env, result, &valuetype));
        if (valuetype != napi_object) {
            LBSLOGE(NAPI_UTILS, "Wrong argument type. Object expected.");
            return nullptr;
        }
        AAFwk::WantParams wantParams;
        if (!OHOS::AppExecFwk::UnwrapWantParams(env, result, wantParams)) {
            return nullptr;
        }
        pActionButton->AddAdditionalData(wantParams);
    }
    return NapiGetNull(env);
}

napi_value NotificationNapi::GetNotificationBadgeNumber(
    const napi_env &env, const napi_value &value, NotificationRequest &request)
{
    LBSLOGD(NAPI_UTILS, "enter");

    napi_valuetype valuetype = napi_undefined;
    napi_value result = nullptr;
    bool hasProperty = false;
    int32_t badgeNumber = 0;

    NAPI_CALL(env, napi_has_named_property(env, value, "badgeNumber", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, value, "badgeNumber", &result);
        NAPI_CALL(env, napi_typeof(env, result, &valuetype));
        if (valuetype != napi_number) {
            LBSLOGE(NAPI_UTILS, "Wrong argument type. Number expected.");
            return nullptr;
        }

        napi_get_value_int32(env, result, &badgeNumber);
        if (badgeNumber < 0) {
            LBSLOGE(NAPI_UTILS, "Wrong badge number.");
            return nullptr;
        }

        request.SetBadgeNumber(badgeNumber);
    }

    return NapiGetNull(env);
}

napi_value NotificationNapi::GetNotificationUnifiedGroupInfo(
    const napi_env &env, const napi_value &value, NotificationRequest &request)
{
    bool hasProperty = false;
    NAPI_CALL(env, napi_has_named_property(env, value, "unifiedGroupInfo", &hasProperty));
    if (!hasProperty) {
        return NapiGetNull(env);
    }

    auto info = AppExecFwk::GetPropertyValueByPropertyName(env, value, "unifiedGroupInfo", napi_object);
    if (info == nullptr) {
        LBSLOGE(NAPI_UTILS, "Wrong argument type. object expected.");
        return nullptr;
    }
    std::shared_ptr<NotificationUnifiedGroupInfo> unifiedGroupInfo = std::make_shared<NotificationUnifiedGroupInfo>();
    // key?: string
    auto jsValue = AppExecFwk::GetPropertyValueByPropertyName(env, info, "key", napi_string);
    if (jsValue != nullptr) {
        std::string key = AppExecFwk::UnwrapStringFromJS(env, jsValue);
        unifiedGroupInfo->SetKey(key);
    }

    // title?: string
    jsValue = AppExecFwk::GetPropertyValueByPropertyName(env, info, "title", napi_string);
    if (jsValue != nullptr) {
        std::string title = AppExecFwk::UnwrapStringFromJS(env, jsValue);
        unifiedGroupInfo->SetTitle(title);
    }

    // content?: string
    jsValue = AppExecFwk::GetPropertyValueByPropertyName(env, info, "content", napi_string);
    if (jsValue != nullptr) {
        std::string content = AppExecFwk::UnwrapStringFromJS(env, jsValue);
        unifiedGroupInfo->SetContent(content);
    }

    // sceneName?: string
    jsValue = AppExecFwk::GetPropertyValueByPropertyName(env, info, "sceneName", napi_string);
    if (jsValue != nullptr) {
        std::string sceneName = AppExecFwk::UnwrapStringFromJS(env, jsValue);
        unifiedGroupInfo->SetSceneName(sceneName);
    }

    // extraInfo?: {[key:string] : any}
    jsValue = AppExecFwk::GetPropertyValueByPropertyName(env, info, "extraInfo", napi_object);
    if (jsValue != nullptr) {
        std::shared_ptr<AAFwk::WantParams> extras = std::make_shared<AAFwk::WantParams>();
        if (!OHOS::AppExecFwk::UnwrapWantParams(env, jsValue, *extras)) {
            return nullptr;
        }
        unifiedGroupInfo->SetExtraInfo(extras);
    }

    request.SetUnifiedGroupInfo(unifiedGroupInfo);
    return NapiGetNull(env);
}

napi_value NotificationNapi::GetNotificationControlFlags(
    const napi_env &env, const napi_value &value, NotificationRequest &request)
{
    LBSLOGD(NAPI_UTILS, "Called.");

    napi_valuetype valuetype = napi_undefined;
    napi_value result = nullptr;
    bool hasProperty = false;
    uint32_t notificationControlFlags = 0;

    NAPI_CALL(env, napi_has_named_property(env, value, "notificationControlFlags", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, value, "notificationControlFlags", &result);
        NAPI_CALL(env, napi_typeof(env, result, &valuetype));
        if (valuetype != napi_number) {
            LBSLOGE(NAPI_UTILS, "Wrong argument type. Number expected.");
            return nullptr;
        }

        napi_get_value_uint32(env, result, &notificationControlFlags);
        if (notificationControlFlags == 0) {
            LBSLOGD(NAPI_UTILS, "Undefined notification control flags.");
            return nullptr;
        }

        request.SetNotificationControlFlags(notificationControlFlags);
    }

    return NapiGetNull(env);
}
}
}
