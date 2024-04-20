/*
 * Copyright (c) 2023-2023 Huawei Device Co., Ltd.
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
#include "ans_log_wrapper.h"
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
const char *Common::GetPropertyNameByContentType(ContentType type)
{
    switch (type) {
        case ContentType::NOTIFICATION_CONTENT_BASIC_TEXT: // normal?: NotificationBasicContent
            return "normal";
        case ContentType::NOTIFICATION_CONTENT_LONG_TEXT: // longText?: NotificationLongTextContent
            return "longText";
        case ContentType::NOTIFICATION_CONTENT_PICTURE: // picture?: NotificationPictureContent
            return "picture";
        case ContentType::NOTIFICATION_CONTENT_CONVERSATION: // conversation?: NotificationConversationalContent
            return "conversation";
        case ContentType::NOTIFICATION_CONTENT_MULTILINE: // multiLine?: NotificationMultiLineContent
            return "multiLine";
        case ContentType::NOTIFICATION_CONTENT_LOCAL_LIVE_VIEW: // systemLiveView?: NotificationLocalLiveViewContent
            return "systemLiveView";
        case ContentType::NOTIFICATION_CONTENT_LIVE_VIEW: // liveView?: NotificationLiveViewContent
            return "liveView";
        default:
            ANS_LOGE("ContentType is does not exist");
            return "null";
    }
}

napi_value Common::GetNotificationContent(const napi_env &env, const napi_value &value, NotificationRequest &request)
{
    ANS_LOGD("enter");

    napi_value result = AppExecFwk::GetPropertyValueByPropertyName(env, value, "content", napi_object);
    if (result == nullptr) {
        ANS_LOGE("No content.");
        return nullptr;
    }

    int32_t type = 0;
    if (GetNotificationContentType(env, result, type) == nullptr) {
        return nullptr;
    }
    NotificationContent::Type outType = NotificationContent::Type::NONE;
    if (!AnsEnumUtil::ContentTypeJSToC(ContentType(type), outType)) {
        return nullptr;
    }
    switch (outType) {
        case NotificationContent::Type::BASIC_TEXT:
            if (GetNotificationBasicContent(env, result, request) == nullptr) {
                return nullptr;
            }
            break;
        case NotificationContent::Type::LONG_TEXT:
            if (GetNotificationLongTextContent(env, result, request) == nullptr) {
                return nullptr;
            }
            break;
        case NotificationContent::Type::PICTURE:
            if (GetNotificationPictureContent(env, result, request) == nullptr) {
                return nullptr;
            }
            break;
        case NotificationContent::Type::CONVERSATION:
            if (GetNotificationConversationalContent(env, result, request) == nullptr) {
                return nullptr;
            }
            break;
        case NotificationContent::Type::MULTILINE:
            if (GetNotificationMultiLineContent(env, result, request) == nullptr) {
                return nullptr;
            }
            break;
        case NotificationContent::Type::LOCAL_LIVE_VIEW:
            if (GetNotificationLocalLiveViewContent(env, result, request) == nullptr) {
                return nullptr;
            }
            break;
        case NotificationContent::Type::LIVE_VIEW:
            if (GetNotificationLiveViewContent(env, result, request) == nullptr) {
                return nullptr;
            }
            break;
        default:
            return nullptr;
    }

    return NapiGetNull(env);
}

napi_value Common::GetNotificationBasicContent(
    const napi_env &env, const napi_value &result, NotificationRequest &request)
{
    ANS_LOGD("enter");

    napi_valuetype valuetype = napi_undefined;
    napi_value contentResult = nullptr;
    bool hasProperty = false;
    NAPI_CALL(env, napi_has_named_property(env, result, "normal", &hasProperty));
    if (!hasProperty) {
        ANS_LOGE("Property normal expected.");
        return nullptr;
    }
    napi_get_named_property(env, result, "normal", &contentResult);
    NAPI_CALL(env, napi_typeof(env, contentResult, &valuetype));
    if (valuetype != napi_object) {
        ANS_LOGE("Wrong argument type. Object expected.");
        return nullptr;
    }

    std::shared_ptr<NotificationNormalContent> normalContent = std::make_shared<NotificationNormalContent>();
    if (normalContent == nullptr) {
        ANS_LOGE("normalContent is null");
        return nullptr;
    }

    if (GetNotificationBasicContentDetailed(env, contentResult, normalContent) == nullptr) {
        return nullptr;
    }

    request.SetContent(std::make_shared<NotificationContent>(normalContent));

    return NapiGetNull(env);
}

napi_value Common::GetNotificationBasicContentDetailed(
    const napi_env &env, const napi_value &contentResult, std::shared_ptr<NotificationBasicContent> basicContent)
{
    ANS_LOGD("enter");

    bool hasProperty = false;
    char str[STR_MAX_SIZE] = {0};
    size_t strLen = 0;

    // title: string
    auto value = AppExecFwk::GetPropertyValueByPropertyName(env, contentResult, "title", napi_string);
    if (value == nullptr) {
        ANS_LOGE("Failed to get title from js.");
        return nullptr;
    }
    NAPI_CALL(env, napi_get_value_string_utf8(env, value, str, STR_MAX_SIZE - 1, &strLen));
    if (std::strlen(str) == 0) {
        ANS_LOGE("Property title is empty");
        return nullptr;
    }
    basicContent->SetTitle(str);
    ANS_LOGD("normal::title = %{public}s", str);

    // text: string
    value = AppExecFwk::GetPropertyValueByPropertyName(env, contentResult, "text", napi_string);
    if (value == nullptr) {
        ANS_LOGE("Failed to get text from js.");
        return nullptr;
    }
    NAPI_CALL(env, napi_get_value_string_utf8(env, value, str, STR_MAX_SIZE - 1, &strLen));
    if (std::strlen(str) == 0) {
        ANS_LOGE("Property text is empty");
        return nullptr;
    }
    basicContent->SetText(str);
    ANS_LOGD("normal::text = %{public}s", str);

    // additionalText?: string
    NAPI_CALL(env, napi_has_named_property(env, contentResult, "additionalText", &hasProperty));
    if (hasProperty) {
        value = AppExecFwk::GetPropertyValueByPropertyName(env, contentResult, "additionalText", napi_string);
        if (value == nullptr) {
            ANS_LOGE("Failed to get additionalText from js.");
            return nullptr;
        }
        NAPI_CALL(env, napi_get_value_string_utf8(env, value, str, STR_MAX_SIZE - 1, &strLen));
        basicContent->SetAdditionalText(str);
        ANS_LOGD("normal::additionalText = %{public}s", str);
    }

    // lockScreenPicture?: pixelMap
    return GetLockScreenPicture(env, contentResult, basicContent);
}

napi_value Common::GetNotificationLongTextContent(
    const napi_env &env, const napi_value &result, NotificationRequest &request)
{
    ANS_LOGD("enter");

    napi_valuetype valuetype = napi_undefined;
    napi_value contentResult = nullptr;
    bool hasProperty = false;

    NAPI_CALL(env, napi_has_named_property(env, result, "longText", &hasProperty));
    if (!hasProperty) {
        ANS_LOGE("Property longText expected.");
        return nullptr;
    }

    napi_get_named_property(env, result, "longText", &contentResult);
    NAPI_CALL(env, napi_typeof(env, contentResult, &valuetype));
    if (valuetype != napi_object) {
        ANS_LOGE("Wrong argument type. Object expected.");
        return nullptr;
    }

    std::shared_ptr<OHOS::Notification::NotificationLongTextContent> longContent =
        std::make_shared<OHOS::Notification::NotificationLongTextContent>();
    if (longContent == nullptr) {
        ANS_LOGE("longContent is null");
        return nullptr;
    }

    if (GetNotificationLongTextContentDetailed(env, contentResult, longContent) == nullptr) {
        return nullptr;
    }

    request.SetContent(std::make_shared<NotificationContent>(longContent));

    return NapiGetNull(env);
}

napi_value Common::GetNotificationLongTextContentDetailed(
    const napi_env &env, const napi_value &contentResult,
    std::shared_ptr<OHOS::Notification::NotificationLongTextContent> &longContent)
{
    ANS_LOGD("enter");

    napi_valuetype valuetype = napi_undefined;
    napi_value longContentResult = nullptr;
    bool hasProperty = false;
    char str[STR_MAX_SIZE] = {0};
    char long_str[LONG_STR_MAX_SIZE + 1] = {0};
    size_t strLen = 0;

    if (GetNotificationBasicContentDetailed(env, contentResult, longContent) == nullptr) {
        return nullptr;
    }

    // longText: string
    NAPI_CALL(env, napi_has_named_property(env, contentResult, "longText", &hasProperty));
    if (!hasProperty) {
        ANS_LOGE("Property longText expected.");
        return nullptr;
    }
    napi_get_named_property(env, contentResult, "longText", &longContentResult);
    NAPI_CALL(env, napi_typeof(env, longContentResult, &valuetype));
    if (valuetype != napi_string) {
        ANS_LOGE("Wrong argument type. String expected.");
        return nullptr;
    }
    NAPI_CALL(env, napi_get_value_string_utf8(env, longContentResult, long_str, LONG_STR_MAX_SIZE, &strLen));
    if (std::strlen(long_str) == 0) {
        ANS_LOGE("Property longText is empty");
        return nullptr;
    }
    longContent->SetLongText(long_str);
    ANS_LOGD("longText::longText = %{public}s", long_str);

    // briefText: string
    NAPI_CALL(env, napi_has_named_property(env, contentResult, "briefText", &hasProperty));
    if (!hasProperty) {
        ANS_LOGE("Property briefText expected.");
        return nullptr;
    }
    napi_get_named_property(env, contentResult, "briefText", &longContentResult);
    NAPI_CALL(env, napi_typeof(env, longContentResult, &valuetype));
    if (valuetype != napi_string) {
        ANS_LOGE("Wrong argument type. String expected.");
        return nullptr;
    }
    NAPI_CALL(env, napi_get_value_string_utf8(env, longContentResult, str, STR_MAX_SIZE - 1, &strLen));
    if (std::strlen(str) == 0) {
        ANS_LOGE("Property briefText is empty");
        return nullptr;
    }
    longContent->SetBriefText(str);
    ANS_LOGD("longText::briefText = %{public}s", str);

    // expandedTitle: string
    NAPI_CALL(env, napi_has_named_property(env, contentResult, "expandedTitle", &hasProperty));
    if (!hasProperty) {
        ANS_LOGE("Property expandedTitle expected.");
        return nullptr;
    }
    napi_get_named_property(env, contentResult, "expandedTitle", &longContentResult);
    NAPI_CALL(env, napi_typeof(env, longContentResult, &valuetype));
    if (valuetype != napi_string) {
        ANS_LOGE("Wrong argument type. String expected.");
        return nullptr;
    }
    NAPI_CALL(env, napi_get_value_string_utf8(env, longContentResult, str, STR_MAX_SIZE - 1, &strLen));
    if (std::strlen(str) == 0) {
        ANS_LOGE("Property expandedTitle is empty");
        return nullptr;
    }
    longContent->SetExpandedTitle(str);
    ANS_LOGD("longText::expandedTitle = %{public}s", str);

    return NapiGetNull(env);
}

napi_value Common::GetNotificationPictureContent(
    const napi_env &env, const napi_value &result, NotificationRequest &request)
{
    ANS_LOGD("enter");

    napi_valuetype valuetype = napi_undefined;
    napi_value contentResult = nullptr;
    bool hasProperty = false;

    NAPI_CALL(env, napi_has_named_property(env, result, "picture", &hasProperty));
    if (!hasProperty) {
        ANS_LOGE("Property picture expected.");
        return nullptr;
    }
    napi_get_named_property(env, result, "picture", &contentResult);
    NAPI_CALL(env, napi_typeof(env, contentResult, &valuetype));
    if (valuetype != napi_object) {
        ANS_LOGE("Wrong argument type. Object expected.");
        return nullptr;
    }

    std::shared_ptr<OHOS::Notification::NotificationPictureContent> pictureContent =
        std::make_shared<OHOS::Notification::NotificationPictureContent>();
    if (pictureContent == nullptr) {
        ANS_LOGE("pictureContent is null");
        return nullptr;
    }
    if (GetNotificationPictureContentDetailed(env, contentResult, pictureContent) == nullptr) {
        return nullptr;
    }

    request.SetContent(std::make_shared<NotificationContent>(pictureContent));

    return NapiGetNull(env);
}

napi_value Common::GetNotificationPictureContentDetailed(const napi_env &env,
    const napi_value &contentResult, std::shared_ptr<OHOS::Notification::NotificationPictureContent> &pictureContent)
{
    ANS_LOGD("enter");

    napi_valuetype valuetype = napi_undefined;
    napi_value pictureContentResult = nullptr;
    bool hasProperty = false;
    char str[STR_MAX_SIZE] = {0};
    size_t strLen = 0;

    if (GetNotificationBasicContentDetailed(env, contentResult, pictureContent) == nullptr) {
        return nullptr;
    }

    // briefText: string
    NAPI_CALL(env, napi_has_named_property(env, contentResult, "briefText", &hasProperty));
    if (!hasProperty) {
        ANS_LOGE("Property briefText expected.");
        return nullptr;
    }
    napi_get_named_property(env, contentResult, "briefText", &pictureContentResult);
    NAPI_CALL(env, napi_typeof(env, pictureContentResult, &valuetype));
    if (valuetype != napi_string) {
        ANS_LOGE("Wrong argument type. String expected.");
        return nullptr;
    }
    NAPI_CALL(env, napi_get_value_string_utf8(env, pictureContentResult, str, STR_MAX_SIZE - 1, &strLen));
    if (std::strlen(str) == 0) {
        ANS_LOGE("Property briefText is empty");
        return nullptr;
    }
    pictureContent->SetBriefText(str);

    // expandedTitle: string
    NAPI_CALL(env, napi_has_named_property(env, contentResult, "expandedTitle", &hasProperty));
    if (!hasProperty) {
        ANS_LOGE("Property expandedTitle expected.");
        return nullptr;
    }
    napi_get_named_property(env, contentResult, "expandedTitle", &pictureContentResult);
    NAPI_CALL(env, napi_typeof(env, pictureContentResult, &valuetype));
    if (valuetype != napi_string) {
        ANS_LOGE("Wrong argument type. String expected.");
        return nullptr;
    }
    NAPI_CALL(env, napi_get_value_string_utf8(env, pictureContentResult, str, STR_MAX_SIZE - 1, &strLen));
    if (std::strlen(str) == 0) {
        ANS_LOGE("Property expandedTitle is empty");
        return nullptr;
    }
    pictureContent->SetExpandedTitle(str);

    // picture: image.PixelMap
    NAPI_CALL(env, napi_has_named_property(env, contentResult, "picture", &hasProperty));
    if (!hasProperty) {
        ANS_LOGE("Property picture expected.");
        return nullptr;
    }
    napi_get_named_property(env, contentResult, "picture", &pictureContentResult);
    NAPI_CALL(env, napi_typeof(env, pictureContentResult, &valuetype));
    if (valuetype != napi_object) {
        ANS_LOGE("Wrong argument type. Object expected.");
        return nullptr;
    }
    std::shared_ptr<Media::PixelMap> pixelMap = nullptr;
    pixelMap = Media::PixelMapNapi::GetPixelMap(env, pictureContentResult);
    if (pixelMap == nullptr) {
        ANS_LOGE("Invalid object pixelMap");
        return nullptr;
    }
    pictureContent->SetBigPicture(pixelMap);

    return Common::NapiGetNull(env);
}

napi_value Common::GetNotificationConversationalContent(
    const napi_env &env, const napi_value &result, NotificationRequest &request)
{
    ANS_LOGD("enter");

    napi_valuetype valuetype = napi_undefined;
    napi_value contentResult = nullptr;
    bool hasProperty = false;
    MessageUser user;

    NAPI_CALL(env, napi_has_named_property(env, result, "conversation", &hasProperty));
    if (!hasProperty) {
        ANS_LOGE("Property conversation expected.");
        return nullptr;
    }
    napi_get_named_property(env, result, "conversation", &contentResult);
    NAPI_CALL(env, napi_typeof(env, contentResult, &valuetype));
    if (valuetype != napi_object) {
        ANS_LOGE("Wrong argument type. Object expected.");
        return nullptr;
    }

    if (GetNotificationConversationalContentByUser(env, contentResult, user) == nullptr) {
        return nullptr;
    }

    std::shared_ptr<OHOS::Notification::NotificationConversationalContent> conversationalContent =
        std::make_shared<OHOS::Notification::NotificationConversationalContent>(user);
    if (conversationalContent == nullptr) {
        ANS_LOGE("conversationalContent is null");
        return nullptr;
    }

    if (GetNotificationBasicContentDetailed(env, contentResult, conversationalContent) == nullptr) {
        return nullptr;
    }
    if (GetNotificationConversationalContentTitle(env, contentResult, conversationalContent) == nullptr) {
        return nullptr;
    }
    if (GetNotificationConversationalContentGroup(env, contentResult, conversationalContent) == nullptr) {
        return nullptr;
    }
    if (GetNotificationConversationalContentMessages(env, contentResult, conversationalContent) == nullptr) {
        return nullptr;
    }

    request.SetContent(std::make_shared<NotificationContent>(conversationalContent));

    return NapiGetNull(env);
}

napi_value Common::GetNotificationConversationalContentByUser(
    const napi_env &env, const napi_value &contentResult, MessageUser &user)
{
    ANS_LOGD("enter");

    napi_valuetype valuetype = napi_undefined;
    bool hasProperty = false;

    // user: MessageUser
    NAPI_CALL(env, napi_has_named_property(env, contentResult, "user", &hasProperty));
    if (!hasProperty) {
        ANS_LOGE("Property user expected.");
        return nullptr;
    }
    napi_value userResult = nullptr;
    napi_get_named_property(env, contentResult, "user", &userResult);
    NAPI_CALL(env, napi_typeof(env, userResult, &valuetype));
    if (valuetype != napi_object) {
        ANS_LOGE("Wrong argument type. Object expected.");
        return nullptr;
    }
    if (!GetMessageUser(env, userResult, user)) {
        return nullptr;
    }

    return NapiGetNull(env);
}

napi_value Common::GetMessageUser(const napi_env &env, const napi_value &result, MessageUser &messageUser)
{
    ANS_LOGD("enter");

    if (GetMessageUserByString(env, result, messageUser) == nullptr) {
        return nullptr;
    }

    if (GetMessageUserByBool(env, result, messageUser) == nullptr) {
        return nullptr;
    }

    if (GetMessageUserByCustom(env, result, messageUser) == nullptr) {
        return nullptr;
    }

    return NapiGetNull(env);
}

napi_value Common::GetMessageUserByString(const napi_env &env, const napi_value &result, MessageUser &messageUser)
{
    ANS_LOGD("enter");

    napi_valuetype valuetype = napi_undefined;
    bool hasProperty = false;
    char str[STR_MAX_SIZE] = {0};
    size_t strLen = 0;

    // name: string
    NAPI_CALL(env, napi_has_named_property(env, result, "name", &hasProperty));
    if (!hasProperty) {
        ANS_LOGE("Property name expected.");
        return nullptr;
    }
    napi_value nameResult = nullptr;
    napi_get_named_property(env, result, "name", &nameResult);
    NAPI_CALL(env, napi_typeof(env, nameResult, &valuetype));
    if (valuetype != napi_string) {
        ANS_LOGE("Wrong argument type. String expected.");
        return nullptr;
    }
    NAPI_CALL(env, napi_get_value_string_utf8(env, nameResult, str, STR_MAX_SIZE - 1, &strLen));
    messageUser.SetName(str);
    ANS_LOGI("MessageUser::name = %{public}s", str);

    // key: string
    NAPI_CALL(env, napi_has_named_property(env, result, "key", &hasProperty));
    if (!hasProperty) {
        ANS_LOGE("Property key expected.");
        return nullptr;
    }
    napi_value keyResult = nullptr;
    napi_get_named_property(env, result, "key", &keyResult);
    NAPI_CALL(env, napi_typeof(env, keyResult, &valuetype));
    if (valuetype != napi_string) {
        ANS_LOGE("Wrong argument type. String expected.");
        return nullptr;
    }
    NAPI_CALL(env, napi_get_value_string_utf8(env, keyResult, str, STR_MAX_SIZE - 1, &strLen));
    messageUser.SetKey(str);
    ANS_LOGI("MessageUser::key = %{public}s", str);

    // uri: string
    NAPI_CALL(env, napi_has_named_property(env, result, "uri", &hasProperty));
    if (!hasProperty) {
        ANS_LOGE("Property uri expected.");
        return nullptr;
    }
    napi_value uriResult = nullptr;
    napi_get_named_property(env, result, "uri", &uriResult);
    NAPI_CALL(env, napi_typeof(env, uriResult, &valuetype));
    if (valuetype != napi_string) {
        ANS_LOGE("Wrong argument type. String expected.");
        return nullptr;
    }
    NAPI_CALL(env, napi_get_value_string_utf8(env, uriResult, str, STR_MAX_SIZE - 1, &strLen));
    Uri uri(str);
    messageUser.SetUri(uri);

    return NapiGetNull(env);
}

napi_value Common::GetMessageUserByBool(const napi_env &env, const napi_value &result, MessageUser &messageUser)
{
    ANS_LOGD("enter");

    napi_valuetype valuetype = napi_undefined;
    bool hasProperty = false;

    // isMachine: boolean
    NAPI_CALL(env, napi_has_named_property(env, result, "isMachine", &hasProperty));
    if (!hasProperty) {
        ANS_LOGE("Property isMachine expected.");
        return nullptr;
    }
    napi_value machineResult = nullptr;
    napi_get_named_property(env, result, "isMachine", &machineResult);
    NAPI_CALL(env, napi_typeof(env, machineResult, &valuetype));
    if (valuetype != napi_boolean) {
        ANS_LOGE("Wrong argument type. Bool expected.");
        return nullptr;
    }
    bool machine = false;
    napi_get_value_bool(env, machineResult, &machine);
    messageUser.SetMachine(machine);
    ANS_LOGD("MessageUser::isMachine = %{public}d", machine);

    // isUserImportant: boolean
    NAPI_CALL(env, napi_has_named_property(env, result, "isUserImportant", &hasProperty));
    if (!hasProperty) {
        ANS_LOGE("Property isUserImportant expected.");
        return nullptr;
    }
    napi_value importantResult = nullptr;
    napi_get_named_property(env, result, "isUserImportant", &importantResult);
    NAPI_CALL(env, napi_typeof(env, importantResult, &valuetype));
    if (valuetype != napi_boolean) {
        ANS_LOGE("Wrong argument type. Bool expected.");
        return nullptr;
    }
    bool important = false;
    napi_get_value_bool(env, importantResult, &important);
    messageUser.SetUserAsImportant(important);
    ANS_LOGI("MessageUser::isUserImportant = %{public}d", important);

    return NapiGetNull(env);
}

napi_value Common::GetMessageUserByCustom(const napi_env &env, const napi_value &result, MessageUser &messageUser)
{
    ANS_LOGD("enter");

    napi_valuetype valuetype = napi_undefined;
    bool hasProperty = false;

    // icon?: image.PixelMap
    NAPI_CALL(env, napi_has_named_property(env, result, "icon", &hasProperty));
    if (hasProperty) {
        napi_value iconResult = nullptr;
        napi_get_named_property(env, result, "icon", &iconResult);
        NAPI_CALL(env, napi_typeof(env, iconResult, &valuetype));
        if (valuetype != napi_object) {
            ANS_LOGE("Wrong argument type. Object expected.");
            return nullptr;
        }
        std::shared_ptr<Media::PixelMap> pixelMap = nullptr;
        pixelMap = Media::PixelMapNapi::GetPixelMap(env, iconResult);
        if (pixelMap == nullptr) {
            ANS_LOGE("Invalid object pixelMap");
            return nullptr;
        }
        messageUser.SetPixelMap(pixelMap);
    }

    return NapiGetNull(env);
}

napi_value Common::GetNotificationConversationalContentTitle(
    const napi_env &env, const napi_value &contentResult,
    std::shared_ptr<OHOS::Notification::NotificationConversationalContent> &conversationalContent)
{
    ANS_LOGD("enter");
    napi_valuetype valuetype = napi_undefined;
    napi_value conversationalContentResult = nullptr;
    bool hasProperty = false;
    char str[STR_MAX_SIZE] = {0};
    size_t strLen = 0;

    // conversationTitle: string
    NAPI_CALL(env, napi_has_named_property(env, contentResult, "conversationTitle", &hasProperty));
    if (!hasProperty) {
        ANS_LOGE("Property conversationTitle expected.");
        return nullptr;
    }
    napi_get_named_property(env, contentResult, "conversationTitle", &conversationalContentResult);
    NAPI_CALL(env, napi_typeof(env, conversationalContentResult, &valuetype));
    if (valuetype != napi_string) {
        ANS_LOGE("Wrong argument type. String expected.");
        return nullptr;
    }
    NAPI_CALL(env, napi_get_value_string_utf8(env, conversationalContentResult, str, STR_MAX_SIZE - 1, &strLen));
    conversationalContent->SetConversationTitle(str);
    ANS_LOGD("conversationTitle = %{public}s", str);

    return NapiGetNull(env);
}

napi_value Common::GetNotificationConversationalContentGroup(
    const napi_env &env, const napi_value &contentResult,
    std::shared_ptr<OHOS::Notification::NotificationConversationalContent> &conversationalContent)
{
    ANS_LOGD("enter");
    napi_valuetype valuetype = napi_undefined;
    napi_value conversationalContentResult = nullptr;
    bool hasProperty = false;

    // conversationGroup: boolean
    NAPI_CALL(env, napi_has_named_property(env, contentResult, "conversationGroup", &hasProperty));
    if (!hasProperty) {
        ANS_LOGE("Property conversationGroup expected.");
        return nullptr;
    }
    napi_get_named_property(env, contentResult, "conversationGroup", &conversationalContentResult);
    NAPI_CALL(env, napi_typeof(env, conversationalContentResult, &valuetype));
    if (valuetype != napi_boolean) {
        ANS_LOGE("Wrong argument type. Bool expected.");
        return nullptr;
    }
    bool conversationGroup = false;
    napi_get_value_bool(env, conversationalContentResult, &conversationGroup);
    conversationalContent->SetConversationGroup(conversationGroup);
    ANS_LOGI("conversationalText::conversationGroup = %{public}d", conversationGroup);

    return NapiGetNull(env);
}

napi_value Common::GetNotificationConversationalContentMessages(
    const napi_env &env, const napi_value &contentResult,
    std::shared_ptr<OHOS::Notification::NotificationConversationalContent> &conversationalContent)
{
    ANS_LOGD("enter");
    napi_valuetype valuetype = napi_undefined;
    napi_value conversationalContentResult = nullptr;
    bool hasProperty = false;

    // messages: Array<ConversationalMessage>
    NAPI_CALL(env, napi_has_named_property(env, contentResult, "messages", &hasProperty));
    if (!hasProperty) {
        ANS_LOGE("Property messages expected.");
        return nullptr;
    }
    napi_get_named_property(env, contentResult, "messages", &conversationalContentResult);
    bool isArray = false;
    napi_is_array(env, conversationalContentResult, &isArray);
    if (!isArray) {
        ANS_LOGE("Property messages is expected to be an array.");
        return nullptr;
    }
    uint32_t length = 0;
    napi_get_array_length(env, conversationalContentResult, &length);
    if (length == 0) {
        ANS_LOGE("The array is empty.");
        return nullptr;
    }
    for (size_t i = 0; i < length; i++) {
        napi_value conversationalMessage = nullptr;
        napi_get_element(env, conversationalContentResult, i, &conversationalMessage);
        NAPI_CALL(env, napi_typeof(env, conversationalMessage, &valuetype));
        if (valuetype != napi_object) {
            ANS_LOGE("Wrong argument type. Object expected.");
            return nullptr;
        }
        std::shared_ptr<NotificationConversationalMessage> message = nullptr;
        if (!GetConversationalMessage(env, conversationalMessage, message)) {
            return nullptr;
        }
        conversationalContent->AddConversationalMessage(message);
    }

    return NapiGetNull(env);
}

napi_value Common::GetConversationalMessage(const napi_env &env, const napi_value &conversationalMessage,
    std::shared_ptr<NotificationConversationalMessage> &message)
{
    ANS_LOGD("enter");

    if (GetConversationalMessageBasicInfo(env, conversationalMessage, message) == nullptr) {
        return nullptr;
    }
    if (GetConversationalMessageOtherInfo(env, conversationalMessage, message) == nullptr) {
        return nullptr;
    }
    return NapiGetNull(env);
}

napi_value Common::GetConversationalMessageBasicInfo(const napi_env &env, const napi_value &conversationalMessage,
    std::shared_ptr<NotificationConversationalMessage> &message)
{
    ANS_LOGD("enter");

    napi_valuetype valuetype = napi_undefined;
    bool hasProperty = false;
    char str[STR_MAX_SIZE] = {0};
    size_t strLen = 0;
    std::string text;
    int64_t timestamp = 0;
    MessageUser sender;

    // text: string
    NAPI_CALL(env, napi_has_named_property(env, conversationalMessage, "text", &hasProperty));
    if (!hasProperty) {
        ANS_LOGE("Property text expected.");
        return nullptr;
    }
    napi_value textResult = nullptr;
    napi_get_named_property(env, conversationalMessage, "text", &textResult);
    NAPI_CALL(env, napi_typeof(env, textResult, &valuetype));
    if (valuetype != napi_string) {
        ANS_LOGE("Wrong argument type. String expected.");
        return nullptr;
    }
    NAPI_CALL(env, napi_get_value_string_utf8(env, textResult, str, STR_MAX_SIZE - 1, &strLen));
    text = str;
    ANS_LOGI("conversationalMessage::text = %{public}s", str);

    // timestamp: number
    NAPI_CALL(env, napi_has_named_property(env, conversationalMessage, "timestamp", &hasProperty));
    if (!hasProperty) {
        ANS_LOGE("Property timestamp expected.");
        return nullptr;
    }
    napi_value timestampResult = nullptr;
    napi_get_named_property(env, conversationalMessage, "timestamp", &timestampResult);
    NAPI_CALL(env, napi_typeof(env, timestampResult, &valuetype));
    if (valuetype != napi_number) {
        ANS_LOGE("Wrong argument type. Number expected.");
        return nullptr;
    }
    napi_get_value_int64(env, timestampResult, &timestamp);
    ANS_LOGI("conversationalMessage::timestamp = %{public}" PRId64, timestamp);

    // sender: MessageUser
    NAPI_CALL(env, napi_has_named_property(env, conversationalMessage, "sender", &hasProperty));
    if (!hasProperty) {
        ANS_LOGE("Property sender expected.");
        return nullptr;
    }
    napi_value senderResult = nullptr;
    napi_get_named_property(env, conversationalMessage, "sender", &senderResult);
    NAPI_CALL(env, napi_typeof(env, senderResult, &valuetype));
    if (valuetype != napi_object) {
        ANS_LOGE("Wrong argument type. Object expected.");
        return nullptr;
    }
    if (!GetMessageUser(env, senderResult, sender)) {
        return nullptr;
    }

    message = std::make_shared<NotificationConversationalMessage>(text, timestamp, sender);
    if (!message) {
        ANS_LOGE("Failed to create NotificationConversationalMessage object");
        return nullptr;
    }

    return NapiGetNull(env);
}

napi_value Common::GetConversationalMessageOtherInfo(const napi_env &env, const napi_value &conversationalMessage,
    std::shared_ptr<NotificationConversationalMessage> &message)
{
    ANS_LOGD("enter");

    napi_valuetype valuetype = napi_undefined;
    bool hasProperty = false;
    char str[STR_MAX_SIZE] = {0};
    size_t strLen = 0;
    std::string mimeType;
    std::string uri;

    // mimeType: string
    NAPI_CALL(env, napi_has_named_property(env, conversationalMessage, "mimeType", &hasProperty));
    if (!hasProperty) {
        ANS_LOGE("Property mimeType expected.");
        return nullptr;
    }
    napi_value mimeTypeResult = nullptr;
    napi_get_named_property(env, conversationalMessage, "mimeType", &mimeTypeResult);
    NAPI_CALL(env, napi_typeof(env, mimeTypeResult, &valuetype));
    if (valuetype != napi_string) {
        ANS_LOGE("Wrong argument type. String expected.");
        return nullptr;
    }
    NAPI_CALL(env, napi_get_value_string_utf8(env, mimeTypeResult, str, STR_MAX_SIZE - 1, &strLen));
    mimeType = str;
    ANS_LOGI("conversationalMessage::mimeType = %{public}s", str);

    // uri?: string
    NAPI_CALL(env, napi_has_named_property(env, conversationalMessage, "uri", &hasProperty));
    if (hasProperty) {
        napi_value uriResult = nullptr;
        napi_get_named_property(env, conversationalMessage, "uri", &uriResult);
        NAPI_CALL(env, napi_typeof(env, uriResult, &valuetype));
        if (valuetype != napi_string) {
            ANS_LOGE("Wrong argument type. String expected.");
            return nullptr;
        }
        NAPI_CALL(env, napi_get_value_string_utf8(env, uriResult, str, STR_MAX_SIZE - 1, &strLen));
        uri = str;
    }

    std::shared_ptr<Uri> uriPtr = std::make_shared<Uri>(uri);
    message->SetData(mimeType, uriPtr);

    return NapiGetNull(env);
}

napi_value Common::GetNotificationMultiLineContent(
    const napi_env &env, const napi_value &result, NotificationRequest &request)
{
    ANS_LOGD("enter");

    napi_valuetype valuetype = napi_undefined;
    napi_value contentResult = nullptr;
    napi_value multiLineContentResult = nullptr;
    bool hasProperty = false;
    char str[STR_MAX_SIZE] = {0};
    size_t strLen = 0;

    NAPI_CALL(env, napi_has_named_property(env, result, "multiLine", &hasProperty));
    if (!hasProperty) {
        ANS_LOGE("Property multiLine expected.");
        return nullptr;
    }
    napi_get_named_property(env, result, "multiLine", &contentResult);
    NAPI_CALL(env, napi_typeof(env, contentResult, &valuetype));
    if (valuetype != napi_object) {
        ANS_LOGE("Wrong argument type. Object expected.");
        return nullptr;
    }

    std::shared_ptr<OHOS::Notification::NotificationMultiLineContent> multiLineContent =
        std::make_shared<OHOS::Notification::NotificationMultiLineContent>();
    if (multiLineContent == nullptr) {
        ANS_LOGE("multiLineContent is null");
        return nullptr;
    }

    if (GetNotificationBasicContentDetailed(env, contentResult, multiLineContent) == nullptr) {
        return nullptr;
    }

    // briefText: string
    NAPI_CALL(env, napi_has_named_property(env, contentResult, "briefText", &hasProperty));
    if (!hasProperty) {
        ANS_LOGE("Property briefText expected.");
        return nullptr;
    }
    napi_get_named_property(env, contentResult, "briefText", &multiLineContentResult);
    NAPI_CALL(env, napi_typeof(env, multiLineContentResult, &valuetype));
    if (valuetype != napi_string) {
        ANS_LOGE("Wrong argument type. String expected.");
        return nullptr;
    }
    NAPI_CALL(env, napi_get_value_string_utf8(env, multiLineContentResult, str, STR_MAX_SIZE - 1, &strLen));
    if (std::strlen(str) == 0) {
        ANS_LOGE("Property briefText is empty");
        return nullptr;
    }
    multiLineContent->SetBriefText(str);
    ANS_LOGD("multiLine: briefText = %{public}s", str);

    // longTitle: string
    NAPI_CALL(env, napi_has_named_property(env, contentResult, "longTitle", &hasProperty));
    if (!hasProperty) {
        ANS_LOGE("Property longTitle expected.");
        return nullptr;
    }
    napi_get_named_property(env, contentResult, "longTitle", &multiLineContentResult);
    NAPI_CALL(env, napi_typeof(env, multiLineContentResult, &valuetype));
    if (valuetype != napi_string) {
        ANS_LOGE("Wrong argument type. String expected.");
        return nullptr;
    }
    NAPI_CALL(env, napi_get_value_string_utf8(env, multiLineContentResult, str, STR_MAX_SIZE - 1, &strLen));
    if (std::strlen(str) == 0) {
        ANS_LOGE("Property longTitle is empty");
        return nullptr;
    }
    multiLineContent->SetExpandedTitle(str);
    ANS_LOGD("multiLine: longTitle = %{public}s", str);

    // lines: Array<String>
    NAPI_CALL(env, napi_has_named_property(env, contentResult, "lines", &hasProperty));
    if (!hasProperty) {
        ANS_LOGE("Property lines expected.");
        return nullptr;
    }
    if (GetNotificationMultiLineContentLines(env, contentResult, multiLineContent) == nullptr) {
        return nullptr;
    }

    request.SetContent(std::make_shared<NotificationContent>(multiLineContent));

    ANS_LOGD("end");
    return NapiGetNull(env);
}

napi_value Common::GetNotificationMultiLineContentLines(const napi_env &env, const napi_value &result,
    std::shared_ptr<OHOS::Notification::NotificationMultiLineContent> &multiLineContent)
{
    ANS_LOGD("enter");

    bool isArray = false;
    napi_valuetype valuetype = napi_undefined;
    napi_value multilines = nullptr;
    char str[STR_MAX_SIZE] = {0};
    size_t strLen = 0;
    uint32_t length = 0;

    napi_get_named_property(env, result, "lines", &multilines);
    napi_is_array(env, multilines, &isArray);
    if (!isArray) {
        ANS_LOGE("Property lines is expected to be an array.");
        return nullptr;
    }

    napi_get_array_length(env, multilines, &length);
    if (length == 0) {
        ANS_LOGE("The array is empty.");
        return nullptr;
    }
    for (size_t i = 0; i < length; i++) {
        napi_value line = nullptr;
        napi_get_element(env, multilines, i, &line);
        NAPI_CALL(env, napi_typeof(env, line, &valuetype));
        if (valuetype != napi_string) {
            ANS_LOGE("Wrong argument type. String expected.");
            return nullptr;
        }
        NAPI_CALL(env, napi_get_value_string_utf8(env, line, str, STR_MAX_SIZE - 1, &strLen));
        multiLineContent->AddSingleLine(str);
        ANS_LOGI("multiLine: lines : addSingleLine = %{public}s", str);
    }

    return NapiGetNull(env);
}

napi_value Common::GetLockScreenPicture(
    const napi_env &env, const napi_value &contentResult, std::shared_ptr<NotificationBasicContent> basicContent)
{
    bool hasProperty = false;
    NAPI_CALL(env, napi_has_named_property(env, contentResult, "lockscreenPicture", &hasProperty));
    if (hasProperty) {
        auto value = AppExecFwk::GetPropertyValueByPropertyName(env, contentResult, "lockscreenPicture", napi_object);
        if (value == nullptr) {
            ANS_LOGE("Failed to get lockScreenPicture from js.");
            return nullptr;
        }
        auto pixelMap = Media::PixelMapNapi::GetPixelMap(env, value);
        if (pixelMap == nullptr) {
            ANS_LOGE("Invalid object pixelMap");
            return nullptr;
        }
        basicContent->SetLockScreenPicture(pixelMap);
    }

    return NapiGetNull(env);
}
}
}
