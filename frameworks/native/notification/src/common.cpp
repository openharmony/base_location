/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "common.h"
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
std::set<std::shared_ptr<AbilityRuntime::WantAgent::WantAgent>> Common::wantAgent_;
std::mutex Common::mutex_;

Common::Common()
{}

Common::~Common()
{}

napi_value Common::GetNotificationSubscriberInfo(
    const napi_env &env, const napi_value &value, NotificationSubscribeInfo &subscriberInfo)
{
    LBSLOGD(NAPI_UTILS, "enter");
    uint32_t length = 0;
    size_t strLen = 0;
    bool hasProperty = false;
    bool isArray = false;
    napi_valuetype valuetype = napi_undefined;

    // bundleNames?: Array<string>
    NAPI_CALL(env, napi_has_named_property(env, value, "bundleNames", &hasProperty));
    if (hasProperty) {
        napi_value nBundleNames = nullptr;
        napi_get_named_property(env, value, "bundleNames", &nBundleNames);
        napi_is_array(env, nBundleNames, &isArray);
        if (!isArray) {
            LBSLOGE(NAPI_UTILS, "Property bundleNames is expected to be an array.");
            return nullptr;
        }
        napi_get_array_length(env, nBundleNames, &length);
        if (length == 0) {
            LBSLOGE(NAPI_UTILS, "The array is empty.");
            return nullptr;
        }
        for (uint32_t i = 0; i < length; ++i) {
            napi_value nBundleName = nullptr;
            char str[STR_MAX_SIZE] = {0};
            napi_get_element(env, nBundleNames, i, &nBundleName);
            NAPI_CALL(env, napi_typeof(env, nBundleName, &valuetype));
            if (valuetype != napi_string) {
                LBSLOGE(NAPI_UTILS, "Wrong argument type. String expected.");
                return nullptr;
            }
            NAPI_CALL(env, napi_get_value_string_utf8(env, nBundleName, str, STR_MAX_SIZE - 1, &strLen));
            subscriberInfo.bundleNames.emplace_back(str);
            subscriberInfo.hasSubscribeInfo = true;
        }
    }

    // userId?: number
    NAPI_CALL(env, napi_has_named_property(env, value, "userId", &hasProperty));
    if (hasProperty) {
        napi_value nUserId = nullptr;
        napi_get_named_property(env, value, "userId", &nUserId);
        NAPI_CALL(env, napi_typeof(env, nUserId, &valuetype));
        if (valuetype != napi_number) {
            LBSLOGE(NAPI_UTILS, "Wrong argument type. Number expected.");
            return nullptr;
        }
        NAPI_CALL(env, napi_get_value_int32(env, nUserId, &subscriberInfo.userId));
        subscriberInfo.hasSubscribeInfo = true;
    }

    return NapiGetNull(env);
}

napi_value Common::GetNotificationUserInput(
    const napi_env &env, const napi_value &actionButton, std::shared_ptr<NotificationActionButton> &pActionButton)
{
    LBSLOGD(NAPI_UTILS, "enter");
    napi_valuetype valuetype = napi_undefined;
    napi_value userInputResult = nullptr;
    bool hasProperty = false;

    // userInput?: NotificationUserInput
    NAPI_CALL(env, napi_has_named_property(env, actionButton, "userInput", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, actionButton, "userInput", &userInputResult);
        NAPI_CALL(env, napi_typeof(env, userInputResult, &valuetype));
        if (valuetype != napi_object) {
            LBSLOGE(NAPI_UTILS, "Wrong argument type. Object expected.");
            return nullptr;
        }
        std::shared_ptr<NotificationUserInput> userInput = nullptr;

        if (!GetNotificationUserInputByInputKey(env, userInputResult, userInput)) {
            return nullptr;
        }
        pActionButton->AddNotificationUserInput(userInput);
    }

    return NapiGetNull(env);
}

napi_value Common::GetNotificationUserInputByInputKey(
    const napi_env &env, const napi_value &userInputResult, std::shared_ptr<NotificationUserInput> &userInput)
{
    LBSLOGD(NAPI_UTILS, "enter");
    napi_valuetype valuetype = napi_undefined;
    napi_value value = nullptr;
    bool hasProperty = false;
    char str[STR_MAX_SIZE] = {0};
    size_t strLen = 0;

    // inputKey: string
    NAPI_CALL(env, napi_has_named_property(env, userInputResult, "inputKey", &hasProperty));
    if (!hasProperty) {
        LBSLOGE(NAPI_UTILS, "Property inputKey expected.");
        return nullptr;
    }
    napi_get_named_property(env, userInputResult, "inputKey", &value);
    NAPI_CALL(env, napi_typeof(env, value, &valuetype));
    if (valuetype != napi_string) {
        LBSLOGE(NAPI_UTILS, "Wrong argument type. String expected.");
        return nullptr;
    }
    NAPI_CALL(env, napi_get_value_string_utf8(env, value, str, STR_MAX_SIZE - 1, &strLen));
    LBSLOGI(NAPI_UTILS, "NotificationUserInput::inputKey = %{public}s", str);
    userInput = NotificationUserInput::Create(str);
    if (!userInput) {
        LBSLOGI(NAPI_UTILS, "Failed to create NotificationUserInput by inputKey=%{public}s", str);
        return nullptr;
    }

    return NapiGetNull(env);
}

napi_value Common::GetNotificationUserInputByTag(
    const napi_env &env, const napi_value &userInputResult, std::shared_ptr<NotificationUserInput> &userInput)
{
    LBSLOGD(NAPI_UTILS, "enter");

    napi_valuetype valuetype = napi_undefined;
    napi_value value = nullptr;
    bool hasProperty = false;
    char str[STR_MAX_SIZE] = {0};
    size_t strLen = 0;

    if (!userInput) {
        LBSLOGE(NAPI_UTILS, "userInput is nullptr");
        return nullptr;
    }
    // tag: string
    NAPI_CALL(env, napi_has_named_property(env, userInputResult, "tag", &hasProperty));
    if (!hasProperty) {
        LBSLOGE(NAPI_UTILS, "Property tag expected.");
        return nullptr;
    }
    napi_get_named_property(env, userInputResult, "tag", &value);
    NAPI_CALL(env, napi_typeof(env, value, &valuetype));
    if (valuetype != napi_string) {
        LBSLOGE(NAPI_UTILS, "Wrong argument type. String expected.");
        return nullptr;
    }
    NAPI_CALL(env, napi_get_value_string_utf8(env, value, str, STR_MAX_SIZE - 1, &strLen));
    userInput->SetTag(str);
    LBSLOGI(NAPI_UTILS, "NotificationUserInput::tag = %{public}s", str);

    return NapiGetNull(env);
}

napi_value Common::GetNotificationUserInputByOptions(
    const napi_env &env, const napi_value &userInputResult, std::shared_ptr<NotificationUserInput> &userInput)
{
    LBSLOGD(NAPI_UTILS, "enter");

    napi_valuetype valuetype = napi_undefined;
    napi_value value = nullptr;
    bool hasProperty = false;
    uint32_t length = 0;
    size_t strLen = 0;
    bool isArray = false;

    if (!userInput) {
        LBSLOGE(NAPI_UTILS, "userInput is nullptr");
        return nullptr;
    }

    // options: Array<string>
    NAPI_CALL(env, napi_has_named_property(env, userInputResult, "options", &hasProperty));

    if (!hasProperty) {
        LBSLOGE(NAPI_UTILS, "Property options expected.");
        return nullptr;
    }
    napi_get_named_property(env, userInputResult, "options", &value);
    napi_is_array(env, value, &isArray);
    if (!isArray) {
        LBSLOGE(NAPI_UTILS, "Property options is expected to be an array.");
        return nullptr;
    }
    napi_get_array_length(env, value, &length);
    if (length == 0) {
        LBSLOGE(NAPI_UTILS, "The array is empty.");
        return nullptr;
    }
    std::vector<std::string> options;
    for (uint32_t i = 0; i < length; ++i) {
        napi_value option = nullptr;
        char str[STR_MAX_SIZE] = {0};
        napi_get_element(env, value, i, &option);
        NAPI_CALL(env, napi_typeof(env, option, &valuetype));
        if (valuetype != napi_string) {
            LBSLOGE(NAPI_UTILS, "Wrong argument type. String expected.");
            return nullptr;
        }
        NAPI_CALL(env, napi_get_value_string_utf8(env, option, str, STR_MAX_SIZE - 1, &strLen));
        options.emplace_back(str);
    }
    userInput->SetOptions(options);

    return NapiGetNull(env);
}

napi_value Common::GetNotificationUserInputByPermitMimeTypes(
    const napi_env &env, const napi_value &userInputResult, std::shared_ptr<NotificationUserInput> &userInput)
{
    LBSLOGD(NAPI_UTILS, "enter");

    napi_valuetype valuetype = napi_undefined;
    napi_value value = nullptr;
    bool hasProperty = false;
    size_t strLen = 0;
    uint32_t length = 0;
    bool isArray = false;

    if (!userInput) {
        LBSLOGE(NAPI_UTILS, "userInput is nullptr");
        return nullptr;
    }

    // permitMimeTypes?: Array<string>
    NAPI_CALL(env, napi_has_named_property(env, userInputResult, "permitMimeTypes", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, userInputResult, "permitMimeTypes", &value);
        napi_is_array(env, value, &isArray);
        if (!isArray) {
            LBSLOGE(NAPI_UTILS, "Property permitMimeTypes is expected to be an array.");
            return nullptr;
        }
        napi_get_array_length(env, value, &length);
        if (length == 0) {
            LBSLOGE(NAPI_UTILS, "The array is empty.");
            return nullptr;
        }
        for (uint32_t i = 0; i < length; ++i) {
            napi_value permitMimeType = nullptr;
            char str[STR_MAX_SIZE] = {0};
            napi_get_element(env, value, i, &permitMimeType);
            NAPI_CALL(env, napi_typeof(env, permitMimeType, &valuetype));
            if (valuetype != napi_string) {
                LBSLOGE(NAPI_UTILS, "Wrong argument type. String expected.");
                return nullptr;
            }
            NAPI_CALL(env, napi_get_value_string_utf8(env, permitMimeType, str, STR_MAX_SIZE - 1, &strLen));
            userInput->SetPermitMimeTypes(str, true);
        }
    }

    return NapiGetNull(env);
}

napi_value Common::GetNotificationUserInputByPermitFreeFormInput(
    const napi_env &env, const napi_value &userInputResult, std::shared_ptr<NotificationUserInput> &userInput)
{
    LBSLOGD(NAPI_UTILS, "enter");
    napi_value value = nullptr;
    napi_valuetype valuetype = napi_undefined;
    bool hasProperty = false;

    if (!userInput) {
        LBSLOGE(NAPI_UTILS, "userInput is nullptr");
        return nullptr;
    }

    // permitFreeFormInput?: boolean
    NAPI_CALL(env, napi_has_named_property(env, userInputResult, "permitFreeFormInput", &hasProperty));
    if (hasProperty) {
        bool permitFreeFormInput = false;
        napi_get_named_property(env, userInputResult, "permitFreeFormInput", &value);
        NAPI_CALL(env, napi_typeof(env, value, &valuetype));
        if (valuetype != napi_boolean) {
            LBSLOGE(NAPI_UTILS, "Wrong argument type. Bool expected.");
            return nullptr;
        }
        napi_get_value_bool(env, value, &permitFreeFormInput);
        LBSLOGI(NAPI_UTILS, "permitFreeFormInput is: %{public}d", permitFreeFormInput);
        userInput->SetPermitFreeFormInput(permitFreeFormInput);
    }

    return NapiGetNull(env);
}

napi_value Common::GetNotificationUserInputByEditType(
    const napi_env &env, const napi_value &userInputResult, std::shared_ptr<NotificationUserInput> &userInput)
{
    LBSLOGD(NAPI_UTILS, "enter");
    napi_value value = nullptr;
    napi_valuetype valuetype = napi_undefined;
    bool hasProperty = false;
    int32_t editType = 0;

    if (!userInput) {
        LBSLOGE(NAPI_UTILS, "userInput is nullptr");
        return nullptr;
    }

    // editType?: number
    NAPI_CALL(env, napi_has_named_property(env, userInputResult, "editType", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, userInputResult, "editType", &value);
        NAPI_CALL(env, napi_typeof(env, value, &valuetype));
        if (valuetype != napi_number) {
            LBSLOGE(NAPI_UTILS, "Wrong argument type. Number expected.");
            return nullptr;
        }
        napi_get_value_int32(env, value, &editType);
        userInput->SetEditType(NotificationConstant::InputEditType(editType));
    }
    return NapiGetNull(env);
}

napi_value Common::GetNotificationUserInputByAdditionalData(
    const napi_env &env, const napi_value &userInputResult, std::shared_ptr<NotificationUserInput> &userInput)
{
    LBSLOGD(NAPI_UTILS, "enter");

    napi_valuetype valuetype = napi_undefined;
    napi_value result = nullptr;
    bool hasProperty = false;

    if (!userInput) {
        LBSLOGE(NAPI_UTILS, "userInput is nullptr");
        return nullptr;
    }

    // additionalData?: {[key: string]: Object}
    NAPI_CALL(env, napi_has_named_property(env, userInputResult, "additionalData", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, userInputResult, "additionalData", &result);
        NAPI_CALL(env, napi_typeof(env, result, &valuetype));
        if (valuetype != napi_object) {
            LBSLOGE(NAPI_UTILS, "Wrong argument type. Object expected.");
            return nullptr;
        }
        AAFwk::WantParams wantParams;
        if (!OHOS::AppExecFwk::UnwrapWantParams(env, result, wantParams)) {
            return nullptr;
        }
        userInput->AddAdditionalData(wantParams);
    }

    return NapiGetNull(env);
}

napi_value Common::GetNotificationContentType(const napi_env &env, const napi_value &result, int32_t &type)
{
    LBSLOGD(NAPI_UTILS, "enter");

    napi_value contentResult = nullptr;
    napi_valuetype valuetype = napi_undefined;
    bool hasNotificationContentType = false;
    bool hasContentType = false;

    NAPI_CALL(env, napi_has_named_property(env, result, "notificationContentType", &hasNotificationContentType));
    if (hasNotificationContentType) {
        napi_get_named_property(env, result, "notificationContentType", &contentResult);
        NAPI_CALL(env, napi_typeof(env, contentResult, &valuetype));
        if (valuetype != napi_number) {
            LBSLOGE(NAPI_UTILS, "Wrong argument type. Number expected.");
            return nullptr;
        }
        napi_get_value_int32(env, contentResult, &type);

        return NapiGetNull(env);
    } else {
        LBSLOGE(NAPI_UTILS, "Property notificationContentType expected.");
    }

    NAPI_CALL(env, napi_has_named_property(env, result, "contentType", &hasContentType));
    if (hasContentType) {
        napi_get_named_property(env, result, "contentType", &contentResult);
        NAPI_CALL(env, napi_typeof(env, contentResult, &valuetype));
        if (valuetype != napi_number) {
            LBSLOGE(NAPI_UTILS, "Wrong argument type. Number expected.");
            return nullptr;
        }
        napi_get_value_int32(env, contentResult, &type);

        return NapiGetNull(env);
    } else {
        LBSLOGE(NAPI_UTILS, "Property contentType expected.");
        return nullptr;
    }
}

napi_value Common::GetNotificationSlot(const napi_env &env, const napi_value &value, NotificationSlot &slot)
{
    LBSLOGD(NAPI_UTILS, "enter");

    napi_value nobj = nullptr;
    napi_valuetype valuetype = napi_undefined;
    bool hasType = false;
    bool hasNotificationType = false;
    int slotType = 0;

    NAPI_CALL(env, napi_has_named_property(env, value, "notificationType", &hasNotificationType));
    NAPI_CALL(env, napi_has_named_property(env, value, "type", &hasType));
    if (hasNotificationType) {
        napi_get_named_property(env, value, "notificationType", &nobj);
        NAPI_CALL(env, napi_typeof(env, nobj, &valuetype));
        if (valuetype != napi_number) {
            LBSLOGE(NAPI_UTILS, "Wrong argument type. Number expected.");
            return nullptr;
        }
    } else if (!hasNotificationType && hasType) {
        napi_get_named_property(env, value, "type", &nobj);
        NAPI_CALL(env, napi_typeof(env, nobj, &valuetype));
        if (valuetype != napi_number) {
            LBSLOGE(NAPI_UTILS, "Wrong argument type. Number expected.");
            return nullptr;
        }
    } else {
        LBSLOGE(NAPI_UTILS, "Property notificationType or type expected.");
        return nullptr;
    }

    if (nobj != nullptr) {
        napi_get_value_int32(env, nobj, &slotType);
    }

    NotificationConstant::SlotType outType = NotificationConstant::SlotType::OTHER;
    if (!AnsEnumUtil::SlotTypeJSToC(SlotType(slotType), outType)) {
        return nullptr;
    }
    slot.SetType(outType);

    if (GetNotificationSlotByString(env, value, slot) == nullptr) {
        return nullptr;
    }
    if (GetNotificationSlotByNumber(env, value, slot) == nullptr) {
        return nullptr;
    }
    if (GetNotificationSlotByVibration(env, value, slot) == nullptr) {
        return nullptr;
    }
    if (GetNotificationSlotByBool(env, value, slot) == nullptr) {
        return nullptr;
    }
    return NapiGetNull(env);
}

napi_value Common::GetNotificationSlotByString(const napi_env &env, const napi_value &value, NotificationSlot &slot)
{
    LBSLOGD(NAPI_UTILS, "enter");

    napi_value nobj = nullptr;
    napi_valuetype valuetype = napi_undefined;
    bool hasProperty = false;
    size_t strLen = 0;

    // desc?: string
    NAPI_CALL(env, napi_has_named_property(env, value, "desc", &hasProperty));
    if (hasProperty) {
        std::string desc;
        char str[STR_MAX_SIZE] = {0};
        napi_get_named_property(env, value, "desc", &nobj);
        NAPI_CALL(env, napi_typeof(env, nobj, &valuetype));
        if (valuetype != napi_string) {
            LBSLOGE(NAPI_UTILS, "Wrong argument type. String expected.");
            return nullptr;
        }
        NAPI_CALL(env, napi_get_value_string_utf8(env, nobj, str, STR_MAX_SIZE - 1, &strLen));
        desc = str;
        LBSLOGI(NAPI_UTILS, "desc is: %{public}s", desc.c_str());
        slot.SetDescription(desc);
    }

    // sound?: string
    NAPI_CALL(env, napi_has_named_property(env, value, "sound", &hasProperty));
    if (hasProperty) {
        std::string sound;
        char str[STR_MAX_SIZE] = {0};
        napi_get_named_property(env, value, "sound", &nobj);
        NAPI_CALL(env, napi_typeof(env, nobj, &valuetype));
        if (valuetype != napi_string) {
            LBSLOGE(NAPI_UTILS, "Wrong argument type. String expected.");
            return nullptr;
        }
        NAPI_CALL(env, napi_get_value_string_utf8(env, nobj, str, STR_MAX_SIZE - 1, &strLen));
        sound = str;
        LBSLOGI(NAPI_UTILS, "sound is: %{public}s", sound.c_str());
        slot.SetSound(Uri(sound));
    }

    return NapiGetNull(env);
}

napi_value Common::GetNotificationSlotByBool(const napi_env &env, const napi_value &value, NotificationSlot &slot)
{
    LBSLOGD(NAPI_UTILS, "enter");
    napi_value nobj = nullptr;
    napi_valuetype valuetype = napi_undefined;
    bool hasProperty = false;

    // badgeFlag?: boolean
    NAPI_CALL(env, napi_has_named_property(env, value, "badgeFlag", &hasProperty));
    if (hasProperty) {
        bool badgeFlag = false;
        napi_get_named_property(env, value, "badgeFlag", &nobj);
        NAPI_CALL(env, napi_typeof(env, nobj, &valuetype));
        if (valuetype != napi_boolean) {
            LBSLOGE(NAPI_UTILS, "Wrong argument type. Bool expected.");
            return nullptr;
        }
        napi_get_value_bool(env, nobj, &badgeFlag);
        LBSLOGI(NAPI_UTILS, "badgeFlag is: %{public}d", badgeFlag);
        slot.EnableBadge(badgeFlag);
    }

    // bypassDnd?: boolean
    NAPI_CALL(env, napi_has_named_property(env, value, "bypassDnd", &hasProperty));
    if (hasProperty) {
        bool bypassDnd = false;
        napi_get_named_property(env, value, "bypassDnd", &nobj);
        NAPI_CALL(env, napi_typeof(env, nobj, &valuetype));
        if (valuetype != napi_boolean) {
            LBSLOGE(NAPI_UTILS, "Wrong argument type. Bool expected.");
            return nullptr;
        }
        napi_get_value_bool(env, nobj, &bypassDnd);
        LBSLOGI(NAPI_UTILS, "bypassDnd is: %{public}d", bypassDnd);
        slot.EnableBypassDnd(bypassDnd);
    }

    // lightEnabled?: boolean
    NAPI_CALL(env, napi_has_named_property(env, value, "lightEnabled", &hasProperty));
    if (hasProperty) {
        bool lightEnabled = false;
        napi_get_named_property(env, value, "lightEnabled", &nobj);
        NAPI_CALL(env, napi_typeof(env, nobj, &valuetype));
        if (valuetype != napi_boolean) {
            LBSLOGE(NAPI_UTILS, "Wrong argument type. Bool expected.");
            return nullptr;
        }
        napi_get_value_bool(env, nobj, &lightEnabled);
        LBSLOGI(NAPI_UTILS, "lightEnabled is: %{public}d", lightEnabled);
        slot.SetEnableLight(lightEnabled);
    }

    return NapiGetNull(env);
}

napi_value Common::GetNotificationSlotByNumber(const napi_env &env, const napi_value &value, NotificationSlot &slot)
{
    LBSLOGD(NAPI_UTILS, "enter");

    napi_value nobj = nullptr;
    napi_valuetype valuetype = napi_undefined;
    bool hasProperty = false;

    // level?: number
    NAPI_CALL(env, napi_has_named_property(env, value, "level", &hasProperty));
    if (hasProperty) {
        int inLevel = 0;
        napi_get_named_property(env, value, "level", &nobj);
        NAPI_CALL(env, napi_typeof(env, nobj, &valuetype));
        if (valuetype != napi_number) {
            LBSLOGE(NAPI_UTILS, "Wrong argument type. Number expected.");
            return nullptr;
        }
        napi_get_value_int32(env, nobj, &inLevel);
        LBSLOGI(NAPI_UTILS, "level is: %{public}d", inLevel);

        NotificationSlot::NotificationLevel outLevel {NotificationSlot::NotificationLevel::LEVEL_NONE};
        if (!AnsEnumUtil::SlotLevelJSToC(SlotLevel(inLevel), outLevel)) {
            return nullptr;
        }
        slot.SetLevel(outLevel);
    }

    // lockscreenVisibility?: number
    NAPI_CALL(env, napi_has_named_property(env, value, "lockscreenVisibility", &hasProperty));
    if (hasProperty) {
        int lockscreenVisibility = 0;
        napi_get_named_property(env, value, "lockscreenVisibility", &nobj);
        NAPI_CALL(env, napi_typeof(env, nobj, &valuetype));
        if (valuetype != napi_number) {
            LBSLOGE(NAPI_UTILS, "Wrong argument type. Number expected.");
            return nullptr;
        }
        napi_get_value_int32(env, nobj, &lockscreenVisibility);
        LBSLOGI(NAPI_UTILS, "lockscreenVisibility is: %{public}d", lockscreenVisibility);
        slot.SetLockscreenVisibleness(NotificationConstant::VisiblenessType(lockscreenVisibility));
    }

    // lightColor?: number
    NAPI_CALL(env, napi_has_named_property(env, value, "lightColor", &hasProperty));
    if (hasProperty) {
        int lightColor = 0;
        napi_get_named_property(env, value, "lightColor", &nobj);
        NAPI_CALL(env, napi_typeof(env, nobj, &valuetype));
        if (valuetype != napi_number) {
            LBSLOGE(NAPI_UTILS, "Wrong argument type. Number expected.");
            return nullptr;
        }
        napi_get_value_int32(env, nobj, &lightColor);
        LBSLOGI(NAPI_UTILS, "lightColor is: %{public}d", lightColor);
        slot.SetLedLightColor(lightColor);
    }
    return NapiGetNull(env);
}

napi_value Common::GetNotificationSlotByVibration(const napi_env &env, const napi_value &value, NotificationSlot &slot)
{
    LBSLOGD(NAPI_UTILS, "enter");
    napi_value nobj = nullptr;
    napi_valuetype valuetype = napi_undefined;
    bool hasProperty = false;
    uint32_t length = 0;

    // vibrationEnabled?: boolean
    bool vibrationEnabled = false;
    NAPI_CALL(env, napi_has_named_property(env, value, "vibrationEnabled", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, value, "vibrationEnabled", &nobj);
        NAPI_CALL(env, napi_typeof(env, nobj, &valuetype));
        if (valuetype != napi_boolean) {
            LBSLOGE(NAPI_UTILS, "Wrong argument type. Bool expected.");
            return nullptr;
        }

        napi_get_value_bool(env, nobj, &vibrationEnabled);
        slot.SetEnableVibration(vibrationEnabled);
    }

    if (!vibrationEnabled) {
        return NapiGetNull(env);
    }

    // vibrationValues?: Array<number>
    NAPI_CALL(env, napi_has_named_property(env, value, "vibrationValues", &hasProperty));
    if (hasProperty) {
        bool isArray = false;
        napi_get_named_property(env, value, "vibrationValues", &nobj);
        napi_is_array(env, nobj, &isArray);
        if (!isArray) {
            LBSLOGE(NAPI_UTILS, "Property vibrationValues is expected to be an array.");
            return nullptr;
        }

        napi_get_array_length(env, nobj, &length);
        std::vector<int64_t> vibrationValues;
        for (size_t i = 0; i < length; i++) {
            napi_value nVibrationValue = nullptr;
            int64_t vibrationValue = 0;
            napi_get_element(env, nobj, i, &nVibrationValue);
            NAPI_CALL(env, napi_typeof(env, nVibrationValue, &valuetype));
            if (valuetype != napi_number) {
                LBSLOGE(NAPI_UTILS, "Wrong argument type. Number expected.");
                return nullptr;
            }
            napi_get_value_int64(env, nVibrationValue, &vibrationValue);
            vibrationValues.emplace_back(vibrationValue);
        }
        slot.SetVibrationStyle(vibrationValues);
    }

    return NapiGetNull(env);
}

napi_value Common::GetBundleOption(const napi_env &env, const napi_value &value, NotificationBundleOption &option)
{
    LBSLOGD(NAPI_UTILS, "enter");

    bool hasProperty {false};
    napi_valuetype valuetype = napi_undefined;
    napi_value result = nullptr;

    char str[STR_MAX_SIZE] = {0};
    size_t strLen = 0;
    // bundle: string
    NAPI_CALL(env, napi_has_named_property(env, value, "bundle", &hasProperty));
    if (!hasProperty) {
        LBSLOGE(NAPI_UTILS, "Property bundle expected.");
        return nullptr;
    }
    napi_get_named_property(env, value, "bundle", &result);
    NAPI_CALL(env, napi_typeof(env, result, &valuetype));
    if (valuetype != napi_string) {
        LBSLOGE(NAPI_UTILS, "Wrong argument type. String expected.");
        return nullptr;
    }
    NAPI_CALL(env, napi_get_value_string_utf8(env, result, str, STR_MAX_SIZE - 1, &strLen));
    option.SetBundleName(str);

    // uid?: number
    NAPI_CALL(env, napi_has_named_property(env, value, "uid", &hasProperty));
    if (hasProperty) {
        int32_t uid = 0;
        napi_get_named_property(env, value, "uid", &result);
        NAPI_CALL(env, napi_typeof(env, result, &valuetype));
        if (valuetype != napi_number) {
            LBSLOGE(NAPI_UTILS, "Wrong argument type. Number expected.");
            return nullptr;
        }
        napi_get_value_int32(env, result, &uid);
        option.SetUid(uid);
    }

    return NapiGetNull(env);
}

napi_value Common::GetButtonOption(const napi_env &env, const napi_value &value, NotificationButtonOption &option)
{
    LBSLOGD(NAPI_UTILS, "enter");

    bool hasProperty {false};
    napi_valuetype valuetype = napi_undefined;
    napi_value result = nullptr;

    char str[STR_MAX_SIZE] = {0};
    size_t strLen = 0;
    // buttonName: string
    NAPI_CALL(env, napi_has_named_property(env, value, "buttonName", &hasProperty));
    if (!hasProperty) {
        LBSLOGE(NAPI_UTILS, "Property buttonName expected.");
        return nullptr;
    }
    napi_get_named_property(env, value, "buttonName", &result);
    NAPI_CALL(env, napi_typeof(env, result, &valuetype));
    if (valuetype != napi_string) {
        LBSLOGE(NAPI_UTILS, "Wrong argument type. String expected.");
        return nullptr;
    }
    NAPI_CALL(env, napi_get_value_string_utf8(env, result, str, STR_MAX_SIZE - 1, &strLen));
    option.SetButtonName(str);

    return NapiGetNull(env);
}

napi_value Common::GetHashCodes(const napi_env &env, const napi_value &value, std::vector<std::string> &hashCodes)
{
    LBSLOGD(NAPI_UTILS, "enter");
    uint32_t length = 0;
    napi_get_array_length(env, value, &length);
    if (length == 0) {
        LBSLOGE(NAPI_UTILS, "The array is empty.");
        return nullptr;
    }
    napi_valuetype valuetype = napi_undefined;
    for (size_t i = 0; i < length; i++) {
        napi_value hashCode = nullptr;
        napi_get_element(env, value, i, &hashCode);
        NAPI_CALL(env, napi_typeof(env, hashCode, &valuetype));
        if (valuetype != napi_string) {
            LBSLOGE(NAPI_UTILS, "Wrong argument type. Object expected.");
            return nullptr;
        }
        char str[STR_MAX_SIZE] = {0};
        size_t strLen = 0;
        NAPI_CALL(env, napi_get_value_string_utf8(env, hashCode, str, STR_MAX_SIZE - 1, &strLen));
        hashCodes.emplace_back(str);
    }

    return NapiGetNull(env);
}

napi_value Common::GetNotificationKey(const napi_env &env, const napi_value &value, NotificationKey &key)
{
    LBSLOGD(NAPI_UTILS, "enter");

    bool hasProperty {false};
    napi_valuetype valuetype = napi_undefined;
    napi_value result = nullptr;

    // id: number
    NAPI_CALL(env, napi_has_named_property(env, value, "id", &hasProperty));
    if (!hasProperty) {
        LBSLOGE(NAPI_UTILS, "Property id expected.");
        return nullptr;
    }
    napi_get_named_property(env, value, "id", &result);
    NAPI_CALL(env, napi_typeof(env, result, &valuetype));
    if (valuetype != napi_number) {
        LBSLOGE(NAPI_UTILS, "Wrong argument type. Number expected.");
        return nullptr;
    }
    napi_get_value_int32(env, result, &key.id);

    // label?: string
    NAPI_CALL(env, napi_has_named_property(env, value, "label", &hasProperty));
    if (hasProperty) {
        char str[STR_MAX_SIZE] = {0};
        size_t strLen = 0;
        napi_get_named_property(env, value, "label", &result);
        NAPI_CALL(env, napi_typeof(env, result, &valuetype));
        if (valuetype != napi_string) {
            LBSLOGE(NAPI_UTILS, "Wrong argument type. String expected.");
            return nullptr;
        }
        NAPI_CALL(env, napi_get_value_string_utf8(env, result, str, STR_MAX_SIZE - 1, &strLen));
        key.label = str;
    }

    return NapiGetNull(env);
}

bool Common::IsValidRemoveReason(int32_t reasonType)
{
    if (reasonType == NotificationConstant::CLICK_REASON_DELETE ||
        reasonType == NotificationConstant::CANCEL_REASON_DELETE) {
        return true;
    }
    LBSLOGE(NAPI_UTILS, "Reason %{public}d is an invalid value", reasonType);
    return false;
}

napi_value Common::GetNotificationTemplate(const napi_env &env, const napi_value &value, NotificationRequest &request)
{
    LBSLOGD(NAPI_UTILS, "enter");

    napi_valuetype valuetype = napi_undefined;
    napi_value result = nullptr;
    bool hasProperty = false;

    NAPI_CALL(env, napi_has_named_property(env, value, "template", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, value, "template", &result);
        NAPI_CALL(env, napi_typeof(env, result, &valuetype));
        if (valuetype != napi_object) {
            LBSLOGE(NAPI_UTILS, "Wrong argument type. Object expected.");
            return nullptr;
        }

        std::shared_ptr<NotificationTemplate> templ = std::make_shared<NotificationTemplate>();
        if (templ == nullptr) {
            LBSLOGE(NAPI_UTILS, "template is null");
            return nullptr;
        }
        if (GetNotificationTemplateInfo(env, result, templ) == nullptr) {
            return nullptr;
        }

        request.SetTemplate(templ);
    }

    return NapiGetNull(env);
}

napi_value Common::GetNotificationBundleOption(
    const napi_env &env, const napi_value &value, NotificationRequest &request)
{
    LBSLOGD(NAPI_UTILS, "Called.");

    napi_valuetype valuetype = napi_undefined;
    napi_value result = nullptr;
    bool hasProperty = false;

    NAPI_CALL(env, napi_has_named_property(env, value, "representativeBundle", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, value, "representativeBundle", &result);
        NAPI_CALL(env, napi_typeof(env, result, &valuetype));
        if (valuetype != napi_object) {
            LBSLOGE(NAPI_UTILS, "Wrong argument type. Object expected.");
            return nullptr;
        }

        std::shared_ptr<NotificationBundleOption> bundleOption = std::make_shared<NotificationBundleOption>();
        if (bundleOption == nullptr) {
            LBSLOGE(NAPI_UTILS, "The bundleOption is null.");
            return nullptr;
        }
        if (GetBundleOption(env, result, *bundleOption) == nullptr) {
            return nullptr;
        }

        request.SetBundleOption(bundleOption);
    }

    return NapiGetNull(env);
}

napi_value Common::GetNotificationTemplateInfo(const napi_env &env, const napi_value &value,
    std::shared_ptr<NotificationTemplate> &templ)
{
    LBSLOGD(NAPI_UTILS, "enter");

    napi_valuetype valuetype = napi_undefined;
    napi_value result = nullptr;
    bool hasProperty = false;
    char str[STR_MAX_SIZE] = {0};
    size_t strLen = 0;

    // name: string
    NAPI_CALL(env, napi_has_named_property(env, value, "name", &hasProperty));
    if (!hasProperty) {
        LBSLOGE(NAPI_UTILS, "Property name expected.");
        return nullptr;
    }
    napi_get_named_property(env, value, "name", &result);
    NAPI_CALL(env, napi_typeof(env, result, &valuetype));
    if (valuetype != napi_string) {
        LBSLOGE(NAPI_UTILS, "Wrong argument type. String expected.");
        return nullptr;
    }
    NAPI_CALL(env, napi_get_value_string_utf8(env, result, str, STR_MAX_SIZE - 1, &strLen));
    std::string strInput = str;
    templ->SetTemplateName(strInput);

    // data?: {[key: string]: object}
    NAPI_CALL(env, napi_has_named_property(env, value, "data", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, value, "data", &result);
        NAPI_CALL(env, napi_typeof(env, result, &valuetype));
        if (valuetype != napi_object) {
            LBSLOGE(NAPI_UTILS, "Wrong argument type. Object expected.");
            return nullptr;
        }
        AAFwk::WantParams wantParams;
        if (!OHOS::AppExecFwk::UnwrapWantParams(env, result, wantParams)) {
            return nullptr;
        }

        std::shared_ptr<AAFwk::WantParams> data = std::make_shared<AAFwk::WantParams>(wantParams);
        templ->SetTemplateData(data);
    }

    return NapiGetNull(env);
}
}  // namespace Location
}  // namespace OHOS
