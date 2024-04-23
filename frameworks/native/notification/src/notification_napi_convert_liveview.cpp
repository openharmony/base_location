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
napi_value NotificationNapi::GetNotificationLocalLiveViewContent(
    const napi_env &env, const napi_value &result, NotificationRequest &request)
{
    LBSLOGD(NAPI_UTILS, "enter");

    napi_valuetype valuetype = napi_undefined;
    napi_value contentResult = nullptr;
    bool hasProperty = false;
    NAPI_CALL(env, napi_has_named_property(env, result, "systemLiveView", &hasProperty));
    if (!hasProperty) {
        LBSLOGE(NAPI_UTILS, "Property localLiveView expected.");
        return nullptr;
    }
    napi_get_named_property(env, result, "systemLiveView", &contentResult);
    NAPI_CALL(env, napi_typeof(env, contentResult, &valuetype));
    if (valuetype != napi_object) {
        LBSLOGE(NAPI_UTILS, "Wrong argument type. Object expected.");
        return nullptr;
    }

    std::shared_ptr<OHOS::Notification::NotificationLocalLiveViewContent> localLiveViewContent =
        std::make_shared<OHOS::Notification::NotificationLocalLiveViewContent>();
    if (localLiveViewContent == nullptr) {
        LBSLOGE(NAPI_UTILS, "localLiveViewContent is null");
        return nullptr;
    }

    if (GetNotificationLocalLiveViewContentDetailed(env, contentResult, localLiveViewContent) == nullptr) {
        return nullptr;
    }

    request.SetContent(std::make_shared<NotificationContent>(localLiveViewContent));

    // set isOnGoing of live view true
    request.SetInProgress(true);

    return NapiGetNull(env);
}

napi_value NotificationNapi::GetNotificationLocalLiveViewCapsule(
    const napi_env &env, const napi_value &contentResult,
    std::shared_ptr<OHOS::Notification::NotificationLocalLiveViewContent> content)
{
    napi_value capsuleResult = nullptr;
    napi_valuetype valuetype = napi_undefined;
    bool hasProperty = false;
    size_t strLen = 0;
    char str[STR_MAX_SIZE] = {0};
    std::shared_ptr<Media::PixelMap> pixelMap = nullptr;
    napi_value result = nullptr;

    LBSLOGD(NAPI_UTILS, "enter");

    NAPI_CALL(env, napi_has_named_property(env, contentResult, "capsule", &hasProperty));

    napi_get_named_property(env, contentResult, "capsule", &capsuleResult);
    NAPI_CALL(env, napi_typeof(env, capsuleResult, &valuetype));
    if (valuetype != napi_object) {
        LBSLOGE(NAPI_UTILS, "Wrong argument type. Object expected.");
        return nullptr;
    }

    NotificationCapsule capsule;

    NAPI_CALL(env, napi_has_named_property(env, capsuleResult, "title", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, capsuleResult, "title", &result);
        NAPI_CALL(env, napi_typeof(env, result, &valuetype));
        if (valuetype != napi_string) {
            LBSLOGE(NAPI_UTILS, "Wrong argument type. String expected.");
            return nullptr;
        }

        NAPI_CALL(env, napi_get_value_string_utf8(env, result, str, STR_MAX_SIZE - 1, &strLen));
        capsule.SetTitle(str);
        LBSLOGD(NAPI_UTILS, "capsule title = %{public}s", str);
    }

    NAPI_CALL(env, napi_has_named_property(env, capsuleResult, "backgroundColor", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, capsuleResult, "backgroundColor", &result);
        NAPI_CALL(env, napi_typeof(env, result, &valuetype));
        if (valuetype != napi_string) {
            LBSLOGE(NAPI_UTILS, "Wrong argument type. String expected.");
            return nullptr;
        }
        NAPI_CALL(env, napi_get_value_string_utf8(env, result, str, STR_MAX_SIZE - 1, &strLen));
        capsule.SetBackgroundColor(str);
        LBSLOGD(NAPI_UTILS, "capsule backgroundColor = %{public}s", str);
    }
    NAPI_CALL(env, napi_has_named_property(env, capsuleResult, "icon", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, capsuleResult, "icon", &result);
        NAPI_CALL(env, napi_typeof(env, result, &valuetype));
        if (valuetype != napi_object) {
            LBSLOGE(NAPI_UTILS, "Wrong argument type. Object expected.");
            return nullptr;
        }
        pixelMap = Media::PixelMapNapi::GetPixelMap(env, result);
        if (pixelMap == nullptr) {
            LBSLOGE(NAPI_UTILS, "Invalid object pixelMap");
            return nullptr;
        }
        capsule.SetIcon(pixelMap);
        LBSLOGD(NAPI_UTILS, "capsule icon = %{public}d", pixelMap->GetWidth());
    }

    content->SetCapsule(capsule);
    content->addFlag(NotificationLocalLiveViewContent::LiveViewContentInner::CAPSULE);

    return NapiGetNull(env);
}

napi_value NotificationNapi::GetNotificationLocalLiveViewButton(
    const napi_env &env, const napi_value &contentResult,
    std::shared_ptr<OHOS::Notification::NotificationLocalLiveViewContent> content)
{
    napi_value result = nullptr;
    napi_valuetype valuetype = napi_undefined;
    bool isArray = false;
    uint32_t length = 0;
    napi_value buttonResult = nullptr;
    bool hasProperty = false;
    char str[STR_MAX_SIZE] = {0};
    size_t strLen = 0;

    LBSLOGD(NAPI_UTILS, "enter");

    napi_get_named_property(env, contentResult, "button", &buttonResult);
    NAPI_CALL(env, napi_typeof(env, buttonResult, &valuetype));
    if (valuetype != napi_object) {
        LBSLOGE(NAPI_UTILS, "Wrong argument type. Object expected.");
        return nullptr;
    }

    NotificationLocalLiveViewButton button;

    NAPI_CALL(env, napi_has_named_property(env, buttonResult, "names", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, buttonResult, "names", &result);
        napi_is_array(env, result, &isArray);
        if (!isArray) {
            LBSLOGE(NAPI_UTILS, "Property names is expected to be an array.");
            return nullptr;
        }
        napi_get_array_length(env, result, &length);
        for (size_t i = 0; i < length; i++) {
            napi_value buttonName = nullptr;
            napi_get_element(env, result, i, &buttonName);
            NAPI_CALL(env, napi_typeof(env, buttonName, &valuetype));
            if (valuetype != napi_string) {
                LBSLOGE(NAPI_UTILS, "Wrong argument type. String expected.");
                return nullptr;
            }
            NAPI_CALL(env, napi_get_value_string_utf8(env, buttonName, str, STR_MAX_SIZE - 1, &strLen));
            button.addSingleButtonName(str);
            LBSLOGD(NAPI_UTILS, "button buttonName = %{public}s.", str);
        }
    }

    NAPI_CALL(env, napi_has_named_property(env, buttonResult, "icons", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, buttonResult, "icons", &result);
        napi_is_array(env, result, &isArray);
        if (!isArray) {
            LBSLOGE(NAPI_UTILS, "Property icons is expected to be an array.");
            return nullptr;
        }
        napi_get_array_length(env, result, &length);
        for (size_t i = 0; i < length; i++) {
            napi_value buttonIcon = nullptr;
            std::shared_ptr<Media::PixelMap> pixelMap = nullptr;
            napi_get_element(env, result, i, &buttonIcon);
            NAPI_CALL(env, napi_typeof(env, buttonIcon, &valuetype));
            if (valuetype != napi_object) {
                LBSLOGE(NAPI_UTILS, "Wrong argument type. Object expected.");
                return nullptr;
            }
            pixelMap = Media::PixelMapNapi::GetPixelMap(env, buttonIcon);
            if (pixelMap != nullptr && static_cast<uint32_t>(pixelMap->GetByteCount()) <= MAX_ICON_SIZE) {
                button.addSingleButtonIcon(pixelMap);
            } else {
                LBSLOGE(NAPI_UTILS, "Invalid pixelMap object or pixelMap is over size.");
                return nullptr;
            }
        }
    }
    LBSLOGD(NAPI_UTILS, "button buttonIcon = %{public}s", str);
    content->SetButton(button);
    content->addFlag(NotificationLocalLiveViewContent::LiveViewContentInner::BUTTON);

    return NapiGetNull(env);
}

napi_value NotificationNapi::GetNotificationLocalLiveViewProgress(const napi_env &env, const napi_value &contentResult,
    std::shared_ptr<OHOS::Notification::NotificationLocalLiveViewContent> content)
{
    napi_value result = nullptr;
    napi_valuetype valuetype = napi_undefined;
    bool hasProperty = false;
    int32_t intValue = -1;
    bool boolValue = false;
    napi_value progressResult = nullptr;

    LBSLOGD(NAPI_UTILS, "enter");

    napi_get_named_property(env, contentResult, "progress", &progressResult);
    NAPI_CALL(env, napi_typeof(env, progressResult, &valuetype));
    if (valuetype != napi_object) {
        LBSLOGE(NAPI_UTILS, "Wrong argument type. Object expected.");
        return nullptr;
    }

    NotificationProgress progress;

    NAPI_CALL(env, napi_has_named_property(env, progressResult, "maxValue", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, progressResult, "maxValue", &result);
        NAPI_CALL(env, napi_typeof(env, result, &valuetype));
        if (valuetype != napi_number) {
            LBSLOGE(NAPI_UTILS, "Wrong argument type. Number expected.");
            return nullptr;
        }
        napi_get_value_int32(env, result, &intValue);
        progress.SetMaxValue(intValue);
        LBSLOGD(NAPI_UTILS, "progress intValue = %{public}d", intValue);
    }

    NAPI_CALL(env, napi_has_named_property(env, progressResult, "currentValue", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, progressResult, "currentValue", &result);
        NAPI_CALL(env, napi_typeof(env, result, &valuetype));
        if (valuetype != napi_number) {
            LBSLOGE(NAPI_UTILS, "Wrong argument type. Number expected.");
            return nullptr;
        }
        napi_get_value_int32(env, result, &intValue);
        progress.SetCurrentValue(intValue);
        LBSLOGD(NAPI_UTILS, "progress currentValue = %{public}d", intValue);
    }

    NAPI_CALL(env, napi_has_named_property(env, progressResult, "isPercentage", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, progressResult, "isPercentage", &result);
        NAPI_CALL(env, napi_typeof(env, result, &valuetype));
        if (valuetype != napi_boolean) {
            LBSLOGE(NAPI_UTILS, "Wrong argument type. bool expected.");
            return nullptr;
        }
        napi_get_value_bool(env, result, &boolValue);
        progress.SetIsPercentage(boolValue);
        LBSLOGD(NAPI_UTILS, "progress isPercentage = %{public}d", boolValue);
    }

    content->SetProgress(progress);
    content->addFlag(NotificationLocalLiveViewContent::LiveViewContentInner::PROGRESS);

    return NapiGetNull(env);
}

napi_value NotificationNapi::GetNotificationLocalLiveViewTime(const napi_env &env, const napi_value &contentResult,
    std::shared_ptr<OHOS::Notification::NotificationLocalLiveViewContent> content)
{
    napi_value result = nullptr;
    napi_valuetype valuetype = napi_undefined;
    bool hasProperty = false;
    int32_t intValue = -1;
    bool boolValue = false;
    napi_value timeResult = nullptr;

    LBSLOGD(NAPI_UTILS, "enter");

    napi_get_named_property(env, contentResult, "time", &timeResult);
    NAPI_CALL(env, napi_typeof(env, timeResult, &valuetype));
    if (valuetype != napi_object) {
        LBSLOGE(NAPI_UTILS, "Wrong argument type. Object expected.");
        return nullptr;
    }

    NotificationTime time;

    NAPI_CALL(env, napi_has_named_property(env, timeResult, "initialTime", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, timeResult, "initialTime", &result);
        NAPI_CALL(env, napi_typeof(env, result, &valuetype));
        if (valuetype != napi_number) {
            LBSLOGE(NAPI_UTILS, "Wrong argument type. Number expected.");
            return nullptr;
        }
        napi_get_value_int32(env, result, &intValue);
        time.SetInitialTime(intValue);
        content->addFlag(NotificationLocalLiveViewContent::LiveViewContentInner::INITIAL_TIME);
        LBSLOGD(NAPI_UTILS, "time initialTime = %{public}d", intValue);
    }

    NAPI_CALL(env, napi_has_named_property(env, timeResult, "isCountDown", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, timeResult, "isCountDown", &result);
        NAPI_CALL(env, napi_typeof(env, result, &valuetype));
        if (valuetype != napi_boolean) {
            LBSLOGE(NAPI_UTILS, "Wrong argument type. bool expected.");
            return nullptr;
        }
        napi_get_value_bool(env, result, &boolValue);
        time.SetIsCountDown(boolValue);
        LBSLOGD(NAPI_UTILS, "time isCountDown = %{public}d", boolValue);
    }

    NAPI_CALL(env, napi_has_named_property(env, timeResult, "isPaused", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, timeResult, "isPaused", &result);
        NAPI_CALL(env, napi_typeof(env, result, &valuetype));
        if (valuetype != napi_boolean) {
            LBSLOGE(NAPI_UTILS, "Wrong argument type. bool expected.");
            return nullptr;
        }
        napi_get_value_bool(env, result, &boolValue);
        time.SetIsPaused(boolValue);
        LBSLOGD(NAPI_UTILS, "time isPaused = %{public}d", boolValue);
    }

    NAPI_CALL(env, napi_has_named_property(env, timeResult, "isInTitle", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, timeResult, "isInTitle", &result);
        NAPI_CALL(env, napi_typeof(env, result, &valuetype));
        if (valuetype != napi_boolean) {
            LBSLOGE(NAPI_UTILS, "Wrong argument type. bool expected.");
            return nullptr;
        }
        napi_get_value_bool(env, result, &boolValue);
        time.SetIsInTitle(boolValue);
        LBSLOGD(NAPI_UTILS, "time isInTitle = %{public}d", boolValue);
    }

    content->SetTime(time);
    content->addFlag(NotificationLocalLiveViewContent::LiveViewContentInner::TIME);

    return NapiGetNull(env);
}

napi_value NotificationNapi::GetNotificationLocalLiveViewContentDetailed(
    const napi_env &env, const napi_value &contentResult,
    std::shared_ptr<OHOS::Notification::NotificationLocalLiveViewContent> content)
{
    bool hasProperty = false;
    int32_t type = -1;
    napi_value result = nullptr;
    napi_valuetype valuetype = napi_undefined;

    LBSLOGD(NAPI_UTILS, "enter");

    //title, text
    if (GetNotificationBasicContentDetailed(env, contentResult, content) == nullptr) {
        LBSLOGE(NAPI_UTILS, "Basic content get fail.");
        return nullptr;
    }

    // typeCode
    NAPI_CALL(env, napi_has_named_property(env, contentResult, "typeCode", &hasProperty));
    if (!hasProperty) {
        LBSLOGE(NAPI_UTILS, "Property typeCode expected.");
        return nullptr;
    }
    napi_get_named_property(env, contentResult, "typeCode", &result);
    NAPI_CALL(env, napi_typeof(env, result, &valuetype));
    if (valuetype != napi_number) {
        LBSLOGE(NAPI_UTILS, "Wrong argument typeCode. Number expected.");
        return nullptr;
    }
    napi_get_value_int32(env, result, &type);
    content->SetType(type);
    LBSLOGD(NAPI_UTILS, "localLiveView type = %{public}d", type);

    //capsule?
    NAPI_CALL(env, napi_has_named_property(env, contentResult, "capsule", &hasProperty));
    if (hasProperty && GetNotificationLocalLiveViewCapsule(env, contentResult, content) == nullptr) {
        return nullptr;
    }

    //button?
    NAPI_CALL(env, napi_has_named_property(env, contentResult, "button", &hasProperty));
    if (hasProperty && GetNotificationLocalLiveViewButton(env, contentResult, content) == nullptr) {
        return nullptr;
    }

    //progress?
    NAPI_CALL(env, napi_has_named_property(env, contentResult, "progress", &hasProperty));
    if (hasProperty && GetNotificationLocalLiveViewProgress(env, contentResult, content) == nullptr) {
        return nullptr;
    }

    //time?
    NAPI_CALL(env, napi_has_named_property(env, contentResult, "time", &hasProperty));
    if (hasProperty && GetNotificationLocalLiveViewTime(env, contentResult, content) == nullptr) {
        return nullptr;
    }

    return NapiGetNull(env);
}

napi_value NotificationNapi::GetNotificationLiveViewContent(
    const napi_env &env, const napi_value &result, NotificationRequest &request)
{
    LBSLOGD(NAPI_UTILS, "enter");

    napi_value contentResult = AppExecFwk::GetPropertyValueByPropertyName(env, result, "liveView", napi_object);
    if (contentResult == nullptr) {
        LBSLOGE(NAPI_UTILS, "Property liveView expected.");
        return nullptr;
    }

    std::shared_ptr<NotificationLiveViewContent> liveViewContent = std::make_shared<NotificationLiveViewContent>();
    if (liveViewContent == nullptr) {
        LBSLOGE(NAPI_UTILS, "LiveViewContent is null");
        return nullptr;
    }

    if (GetNotificationLiveViewContentDetailed(env, contentResult, liveViewContent) == nullptr) {
        return nullptr;
    }

    request.SetContent(std::make_shared<NotificationContent>(liveViewContent));

    return NapiGetNull(env);
}

napi_value NotificationNapi::GetNotificationLiveViewContentDetailed(
    const napi_env &env, const napi_value &contentResult,
    std::shared_ptr<NotificationLiveViewContent> &liveViewContent)
{
    LBSLOGD(NAPI_UTILS, "enter");

    // lockScreenPicture?: pixelMap
    if (GetLockScreenPicture(env, contentResult, liveViewContent) == nullptr) {
        LBSLOGE(NAPI_UTILS, "Failed to get lockScreenPicture from liveView content.");
        return nullptr;
    }

    // status: NotificationLiveViewContent::LiveViewStatus
    int32_t status = 0;
    if (!AppExecFwk::UnwrapInt32ByPropertyName(env, contentResult, "status", status)) {
        LBSLOGE(NAPI_UTILS, "Failed to get status from liveView content.");
        return nullptr;
    }
    NotificationLiveViewContent::LiveViewStatus outType = NotificationLiveViewContent::LiveViewStatus::LIVE_VIEW_BUTT;
    if (!AnsEnumUtil::LiveViewStatusJSToC(LiveViewStatus(status), outType)) {
        LBSLOGE(NAPI_UTILS, "The liveview status is not valid.");
        return nullptr;
    }
    liveViewContent->SetLiveViewStatus(outType);

    // version?: uint32_t
    napi_value jsValue = AppExecFwk::GetPropertyValueByPropertyName(env, contentResult,
        "version", napi_number);
    if (jsValue != nullptr) {
        int32_t version = NotificationLiveViewContent::MAX_VERSION;
        NAPI_CALL(env, napi_get_value_int32(env, jsValue, &version));
        liveViewContent->SetVersion(version);
    }

    // extraInfo?: {[key:string] : any}
    jsValue = AppExecFwk::GetPropertyValueByPropertyName(env, contentResult, "extraInfo", napi_object);
    if (jsValue != nullptr) {
        std::shared_ptr<AAFwk::WantParams> extras = std::make_shared<AAFwk::WantParams>();
        if (!OHOS::AppExecFwk::UnwrapWantParams(env, jsValue, *extras)) {
            return nullptr;
        }
        liveViewContent->SetExtraInfo(extras);
    }

    //isOnlyLocalUpdate_?: boolean
    bool isLocalUpdateOnly = false;
    if (AppExecFwk::UnwrapBooleanByPropertyName(env, contentResult, "isLocalUpdateOnly", isLocalUpdateOnly)) {
        liveViewContent->SetIsOnlyLocalUpdate(isLocalUpdateOnly);
    }

    // pictureInfo?: {[key, string]: Array<image.pixelMap>}
    jsValue = AppExecFwk::GetPropertyValueByPropertyName(env, contentResult, "pictureInfo", napi_object);
    if (jsValue == nullptr) {
        LBSLOGI(NAPI_UTILS, "No picture maps.");
        return NapiGetNull(env);
    }

    std::map<std::string, std::vector<std::shared_ptr<Media::PixelMap>>> pictureMap;
    if (GetLiveViewPictureInfo(env, jsValue, pictureMap) == nullptr) {
        LBSLOGE(NAPI_UTILS, "Failed to get picture map from liveView content.");
        return nullptr;
    }
    liveViewContent->SetPicture(pictureMap);

    return NapiGetNull(env);
}

napi_value NotificationNapi::GetLiveViewPictures(
    const napi_env &env, const napi_value &picturesObj,
    std::vector<std::shared_ptr<Media::PixelMap>> &pictures)
{
    LBSLOGD(NAPI_UTILS, "enter");

    bool isArray = false;
    napi_is_array(env, picturesObj, &isArray);
    if (!isArray) {
        LBSLOGE(NAPI_UTILS, "The picture is not array.");
        return nullptr;
    }

    uint32_t length = 0;
    napi_get_array_length(env, picturesObj, &length);
    if (length == 0) {
        LBSLOGE(NAPI_UTILS, "The array is empty.");
        return nullptr;
    }

    for (uint32_t i = 0; i < length; ++i) {
        napi_value pictureObj = nullptr;
        napi_get_element(env, picturesObj, i, &pictureObj);
        if (!AppExecFwk::IsTypeForNapiValue(env, pictureObj, napi_object)) {
            LBSLOGE(NAPI_UTILS, "Wrong argument type. object expected.");
            break;
        }

        std::shared_ptr<Media::PixelMap> pixelMap = Media::PixelMapNapi::GetPixelMap(env, pictureObj);
        if (pixelMap == nullptr) {
            LBSLOGE(NAPI_UTILS, "Invalid pixelMap.");
            break;
        }

        pictures.emplace_back(pixelMap);
    }

    return NapiGetNull(env);
}

napi_value NotificationNapi::GetLiveViewPictureInfo(
    const napi_env &env, const napi_value &pictureMapObj,
    std::map<std::string, std::vector<std::shared_ptr<Media::PixelMap>>> &pictureMap)
{
    LBSLOGD(NAPI_UTILS, "enter");

    napi_value pictureNamesObj = nullptr;
    uint32_t length = 0;
    if (napi_get_property_names(env, pictureMapObj, &pictureNamesObj) != napi_ok) {
        LBSLOGE(NAPI_UTILS, "Get picture names failed.");
        return nullptr;
    }
    napi_get_array_length(env, pictureNamesObj, &length);
    if (length == 0) {
        LBSLOGE(NAPI_UTILS, "The pictures name is empty.");
        return nullptr;
    }

    napi_value pictureNameObj = nullptr;
    napi_value picturesObj = nullptr;
    for (uint32_t index = 0; index < length; index++) {
        napi_get_element(env, pictureNamesObj, index, &pictureNameObj);
        std::string pictureName = AppExecFwk::UnwrapStringFromJS(env, pictureNameObj);
        LBSLOGD(NAPI_UTILS, "%{public}s called, get pictures of %{public}s.", __func__, pictureName.c_str());
        napi_get_named_property(env, pictureMapObj, pictureName.c_str(), &picturesObj);

        std::vector<std::shared_ptr<Media::PixelMap>> pictures;
        if (!GetLiveViewPictures(env, picturesObj, pictures)) {
            LBSLOGE(NAPI_UTILS, "Get pictures of %{public}s failed.", pictureName.c_str());
            break;
        }

        pictureMap[pictureName] = pictures;
    }

    return NapiGetNull(env);
}
}
}
