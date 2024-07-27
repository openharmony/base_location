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
#ifndef BASE_LOCATION_DISTRIBUTED_NOTIFICATION_SERVICE_FRAMEWORKS_JS_NAPI_INCLUDE_COMMON_H
#define BASE_LOCATION_DISTRIBUTED_NOTIFICATION_SERVICE_FRAMEWORKS_JS_NAPI_INCLUDE_COMMON_H

#include "location_log.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "notification_button_option.h"
#include "notification_helper.h"
#include "notification_local_live_view_button.h"
#include "notification_progress.h"
#include "notification_time.h"
#include "ans_convert_enum.h"

namespace OHOS {
namespace Location {
using namespace OHOS::Notification;
using namespace OHOS::NotificationNapi;

constexpr int32_t STR_MAX_SIZE = 200;
constexpr int32_t LONG_STR_MAX_SIZE = 1024;
constexpr uint8_t OPERATION_MAX_TYPE = 3;
constexpr int8_t NO_ERROR = 0;
constexpr int8_t ERROR = -1;

enum class SemanticActionButton {
    NONE_ACTION_BUTTON,
    REPLY_ACTION_BUTTON,
    READ_ACTION_BUTTON,
    UNREAD_ACTION_BUTTON,
    DELETE_ACTION_BUTTON,
    ARCHIVE_ACTION_BUTTON,
    MUTE_ACTION_BUTTON,
    UNMUTE_ACTION_BUTTON,
    THUMBS_UP_ACTION_BUTTON,
    THUMBS_DOWN_ACTION_BUTTON,
    CALL_ACTION_BUTTON
};

enum class InputsSource {
    FREE_FORM_INPUT,
    OPTION
};

enum class DisturbMode {
    ALLOW_UNKNOWN,
    ALLOW_ALL,
    ALLOW_PRIORITY,
    ALLOW_NONE,
    ALLOW_ALARMS
};

enum class InputEditType {
    EDIT_AUTO,
    EDIT_DISABLED,
    EDIT_ENABLED
};


enum class NotificationFlagStatus {
    TYPE_NONE,
    TYPE_OPEN,
    TYPE_CLOSE
};

struct NotificationSubscribeInfo {
    std::vector<std::string> bundleNames;
    int32_t userId = 0;
    bool hasSubscribeInfo = false;
};

struct CallbackPromiseInfo {
    napi_ref callback = nullptr;
    napi_deferred deferred = nullptr;
    bool isCallback = false;
    int32_t errorCode = 0;
};

class NotificationNapi {
    NotificationNapi();

    ~NotificationNapi();

public:
    /**
     * @brief Gets a napi value that is used to represent specified bool value
     *
     * @param env Indicates the environment that the API is invoked under
     * @param isValue Indicates a bool value
     * @return Returns a napi value that is used to represent specified bool value
     */
    static napi_value NapiGetBoolean(napi_env env, const bool &isValue);

    /**
     * @brief Gets the napi value that is used to represent the null object
     *
     * @param env Indicates the environment that the API is invoked under
     * @return Returns the napi value that is used to represent the null object
     */
    static napi_value NapiGetNull(napi_env env);

    /**
     * @brief Gets the napi value that is used to represent the undefined object
     *
     * @param env Indicates the environment that the API is invoked under
     * @return Returns the napi value that is used to represent the undefined object
     */
    static napi_value NapiGetUndefined(napi_env env);

    /**
     * @brief Gets a napi value with specified error code for callback
     *
     * @param env Indicates the environment that the API is invoked under
     * @param errCode Indicates specified err code
     * @return Returns a napi value with specified error code for callback
     */
    static napi_value GetCallbackErrorValue(napi_env env, int32_t errCode);

    /**
     * @brief Pads the CallbackPromiseInfo struct
     *
     * @param env Indicates the environment that the API is invoked under
     * @param callback Indicates a napi_ref for callback
     * @param info Indicates the CallbackPromiseInfo struct to be padded
     * @param promise Indicates the promise to be created when the callback is null
     */
    static void PaddingCallbackPromiseInfo(
        const napi_env &env, const napi_ref &callback, CallbackPromiseInfo &info, napi_value &promise);

    /**
     * @brief Gets the returned result by the CallbackPromiseInfo struct
     *
     * @param env Indicates the environment that the API is invoked under
     * @param info Indicates the CallbackPromiseInfo struct
     * @param result Indicates the returned result
     */
    static void ReturnCallbackPromise(const napi_env &env, const CallbackPromiseInfo &info, const napi_value &result);

    /**
     * @brief Calls the callback with the result and error code
     *
     * @param env Indicates the environment that the API is invoked under
     * @param callbackIn Indicates the callback to be called
     * @param errCode Indicates the error code returned by the callback
     * @param result Indicates the result returned by the callback
     */
    static void SetCallback(const napi_env &env,
        const napi_ref &callbackIn, const int32_t &errorCode, const napi_value &result, bool newType);

    /**
     * @brief Calls the callback with the result
     *
     * @param env Indicates the environment that the API is invoked under
     * @param callbackIn Indicates the callback to be called
     * @param result Indicates the result returned by the callback
     */
    static void SetCallback(
        const napi_env &env, const napi_ref &callbackIn, const napi_value &result);

    /**
     * @brief Calls the callback with the result
     *
     * @param env Indicates the environment that the API is invoked under
     * @param callbackIn Indicates the callback to be called
     * @param result Indicates the result returned by the callback
     */
    static void SetCallbackArg2(
        const napi_env &env, const napi_ref &callbackIn, const napi_value &result0, const napi_value &result1);

    /**
     * @brief Processes the promise with the result and error code
     *
     * @param env Indicates the environment that the API is invoked under
     * @param deferred Indicates the deferred object whose associated promise to resolve
     * @param errorCode Indicates the error code returned by the callback
     * @param result Indicates the result returned by the callback
     */
    static void SetPromise(const napi_env &env,
        const napi_deferred &deferred, const int32_t &errorCode, const napi_value &result, bool newType);

    /**
     * @brief Gets the returned result by the callback when an error occurs
     *
     * @param env Indicates the environment that the API is invoked under
     * @param callback Indicates a napi_ref for callback
     * @return Returns the null object
     */
    static napi_value JSParaError(const napi_env &env, const napi_ref &callback);

    /**
     * @brief Parses a single parameter for callback
     *
     * @param env Indicates the environment that the API is invoked under
     * @param info Indicates the callback info passed into the callback function
     * @param callback Indicates the napi_ref for the callback parameter
     * @return Returns the null object if success, returns the null value otherwise
     */
    static napi_value ParseParaOnlyCallback(const napi_env &env, const napi_callback_info &info, napi_ref &callback);

    /**
     * @brief Gets a NotificationSubscribeInfo object from specified js object
     *
     * @param env Indicates the environment that the API is invoked under
     * @param value Indicates a js object to be converted
     * @param result Indicates a NotificationSubscribeInfo object from specified js object
     * @return Returns the null object if success, returns the null value otherwise
     */
    static napi_value GetNotificationSubscriberInfo(
        const napi_env &env, const napi_value &value, NotificationSubscribeInfo &result);

    /**
     * @brief Gets a NotificationRequest object from specified js object
     *
     * @param env Indicates the environment that the API is invoked under
     * @param value Indicates a js object to be converted
     * @param result Indicates a NotificationRequest object from specified js object
     * @return Returns the null object if success, returns the null value otherwise
     */
    static napi_value GetNotificationRequest(
        const napi_env &env, const napi_value &value, NotificationRequest &request);

    /**
     * @brief Gets a NotificationRequest object by number type from specified js object
     *
     * @param env Indicates the environment that the API is invoked under
     * @param value Indicates a js object to be converted
     * @param request Indicates a NotificationRequest object from specified js object
     * @return Returns the null object if success, returns the null value otherwise
     */
    static napi_value GetNotificationRequestByNumber(
        const napi_env &env, const napi_value &value, NotificationRequest &request);

    /**
     * @brief Gets a NotificationRequest object by string type from specified js object
     *
     * @param env Indicates the environment that the API is invoked under
     * @param value Indicates a js object to be converted
     * @param request Indicates a NotificationRequest object from specified js object
     * @return Returns the null object if success, returns the null value otherwise
     */
    static napi_value GetNotificationRequestByString(
        const napi_env &env, const napi_value &value, NotificationRequest &request);

    /**
     * @brief Gets a NotificationRequest object by bool type from specified js object
     *
     * @param env Indicates the environment that the API is invoked under
     * @param value Indicates a js object to be converted
     * @param request Indicates a NotificationRequest object from specified js object
     * @return Returns the null object if success, returns the null value otherwise
     */
    static napi_value GetNotificationRequestByBool(
        const napi_env &env, const napi_value &value, NotificationRequest &request);

    /**
     * @brief Gets a NotificationRequest object by custom type from specified js object
     *
     * @param env Indicates the environment that the API is invoked under
     * @param value Indicates a js object to be converted
     * @param request Indicates a NotificationRequest object from specified js object
     * @return Returns the null object if success, returns the null value otherwise
     */
    static napi_value GetNotificationRequestByCustom(
        const napi_env &env, const napi_value &value, NotificationRequest &request);

    /**
     * @brief Gets the id of NotificationRequest object from specified js object
     *
     * @param env Indicates the environment that the API is invoked under
     * @param value Indicates a js object to be converted
     * @param request Indicates a NotificationRequest object from specified js object
     * @return Returns the null object if success, returns the null value otherwise
     */
    static napi_value GetNotificationId(const napi_env &env, const napi_value &value, NotificationRequest &request);

    /**
     * @brief Gets the slot type of NotificationRequest object from specified js object
     *
     * @param env Indicates the environment that the API is invoked under
     * @param value Indicates a js object to be converted
     * @param request Indicates a NotificationRequest object from specified js object
     * @return Returns the null object if success, returns the null value otherwise
     */
    static napi_value GetNotificationSlotType(
        const napi_env &env, const napi_value &value, NotificationRequest &request);

    /**
     * @brief Gets the isOngoing flag of NotificationRequest object from specified js object
     *
     * @param env Indicates the environment that the API is invoked under
     * @param value Indicates a js object to be converted
     * @param request Indicates a NotificationRequest object from specified js object
     * @return Returns the null object if success, returns the null value otherwise
     */
    static napi_value GetNotificationIsOngoing(
        const napi_env &env, const napi_value &value, NotificationRequest &request);

    /**
     * @brief Gets the isUnremovable flag of NotificationRequest object from specified js object
     *
     * @param env Indicates the environment that the API is invoked under
     * @param value Indicates a js object to be converted
     * @param request Indicates a NotificationRequest object from specified js object
     * @return Returns the null object if success, returns the null value otherwise
     */
    static napi_value GetNotificationIsUnremovable(
        const napi_env &env, const napi_value &value, NotificationRequest &request);

    /**
     * @brief Gets the delivery time of NotificationRequest object from specified js object
     *
     * @param env Indicates the environment that the API is invoked under
     * @param value Indicates a js object to be converted
     * @param request Indicates a NotificationRequest object from specified js object
     * @return Returns the null object if success, returns the null value otherwise
     */
    static napi_value GetNotificationDeliveryTime(
        const napi_env &env, const napi_value &value, NotificationRequest &request);

    /**
     * @brief Gets the tapDismissed flag of NotificationRequest object from specified js object
     *
     * @param env Indicates the environment that the API is invoked under
     * @param value Indicates a js object to be converted
     * @param request Indicates a NotificationRequest object from specified js object
     * @return Returns the null object if success, returns the null value otherwise
     */
    static napi_value GetNotificationtapDismissed(
        const napi_env &env, const napi_value &value, NotificationRequest &request);

    /**
     * @brief Gets the extra information of NotificationRequest object from specified js object
     *
     * @param env Indicates the environment that the API is invoked under
     * @param value Indicates a js object to be converted
     * @param request Indicates a NotificationRequest object from specified js object
     * @return Returns the null object if success, returns the null value otherwise
     */
    static napi_value GetNotificationExtraInfo(
        const napi_env &env, const napi_value &value, NotificationRequest &request);

    /**
     * @brief Gets the group name of NotificationRequest object from specified js object
     *
     * @param env Indicates the environment that the API is invoked under
     * @param value Indicates a js object to be converted
     * @param request Indicates a NotificationRequest object from specified js object
     * @return Returns the null object if success, returns the null value otherwise
     */
    static napi_value GetNotificationGroupName(
        const napi_env &env, const napi_value &value, NotificationRequest &request);

    /**
     * @brief Gets the removal WantAgent object of NotificationRequest object from specified js object
     *
     * @param env Indicates the environment that the API is invoked under
     * @param value Indicates a js object to be converted
     * @param request Indicates a NotificationRequest object from specified js object
     * @return Returns the null object if success, returns the null value otherwise
     */
    static napi_value GetNotificationRemovalWantAgent(
        const napi_env &env, const napi_value &value, NotificationRequest &request);

    /**
     * @brief Gets the max screen WantAgent object of NotificationRequest object from specified js object
     *
     * @param env Indicates the environment that the API is invoked under
     * @param value Indicates a js object to be converted
     * @param request Indicates a NotificationRequest object from specified js object
     * @return Returns the null object if success, returns the null value otherwise
     */
    static napi_value GetNotificationMaxScreenWantAgent(
        const napi_env &env, const napi_value &value, NotificationRequest &request);

    /**
     * @brief Gets the auto deleted time of NotificationRequest object from specified js object
     *
     * @param env Indicates the environment that the API is invoked under
     * @param value Indicates a js object to be converted
     * @param request Indicates a NotificationRequest object from specified js object
     * @return Returns the null object if success, returns the null value otherwise
     */
    static napi_value GetNotificationAutoDeletedTime(
        const napi_env &env, const napi_value &value, NotificationRequest &request);

    /**
     * @brief Gets the classification of NotificationRequest object from specified js object
     *
     * @param env Indicates the environment that the API is invoked under
     * @param value Indicates a js object to be converted
     * @param request Indicates a NotificationRequest object from specified js object
     * @return Returns the null object if success, returns the null value otherwise
     */
    static napi_value GetNotificationClassification(
        const napi_env &env, const napi_value &value, NotificationRequest &request);

    /**
     * @brief Gets the appMessageId of NotificationRequest object from specified js object
     *
     * @param env Indicates the environment that the API is invoked under
     * @param value Indicates a js object to be converted
     * @param request Indicates a NotificationRequest object from specified js object
     * @return Returns the null object if success, returns the null value otherwise
     */
    static napi_value GetNotificationAppMessageId(
        const napi_env &env, const napi_value &value, NotificationRequest &request);

    /**
     * @brief Gets the color of NotificationRequest object from specified js object
     *
     * @param env Indicates the environment that the API is invoked under
     * @param value Indicates a js object to be converted
     * @param request Indicates a NotificationRequest object from specified js object
     * @return Returns the null object if success, returns the null value otherwise
     */
    static napi_value GetNotificationColor(const napi_env &env, const napi_value &value, NotificationRequest &request);

    /**
     * @brief Gets the colorEnabled flag of NotificationRequest object from specified js object
     *
     * @param env Indicates the environment that the API is invoked under
     * @param value Indicates a js object to be converted
     * @param request Indicates a NotificationRequest object from specified js object
     * @return Returns the null object if success, returns the null value otherwise
     */
    static napi_value GetNotificationColorEnabled(
        const napi_env &env, const napi_value &value, NotificationRequest &request);

    /**
     * @brief Gets the isAlertOnce flag of NotificationRequest object from specified js object
     *
     * @param env Indicates the environment that the API is invoked under
     * @param value Indicates a js object to be converted
     * @param request Indicates a NotificationRequest object from specified js object
     * @return Returns the null object if success, returns the null value otherwise
     */
    static napi_value GetNotificationIsAlertOnce(
        const napi_env &env, const napi_value &value, NotificationRequest &request);

    /**
     * @brief Gets the isStopwatch flag of NotificationRequest object from specified js object
     *
     * @param env Indicates the environment that the API is invoked under
     * @param value Indicates a js object to be converted
     * @param request Indicates a NotificationRequest object from specified js object
     * @return Returns the null object if success, returns the null value otherwise
     */
    static napi_value GetNotificationIsStopwatch(
        const napi_env &env, const napi_value &value, NotificationRequest &request);

    /**
     * @brief Gets the isCountDown flag of NotificationRequest object from specified js object
     *
     * @param env Indicates the environment that the API is invoked under
     * @param value Indicates a js object to be converted
     * @param request Indicates a NotificationRequest object from specified js object
     * @return Returns the null object if success, returns the null value otherwise
     */
    static napi_value GetNotificationIsCountDown(
        const napi_env &env, const napi_value &value, NotificationRequest &request);

    /**
     * @brief Gets the status bar text of NotificationRequest object from specified js object
     *
     * @param env Indicates the environment that the API is invoked under
     * @param value Indicates a js object to be converted
     * @param request Indicates a NotificationRequest object from specified js object
     * @return Returns the null object if success, returns the null value otherwise
     */
    static napi_value GetNotificationStatusBarText(
        const napi_env &env, const napi_value &value, NotificationRequest &request);

    /**
     * @brief Gets the label of NotificationRequest object from specified js object
     *
     * @param env Indicates the environment that the API is invoked under
     * @param value Indicates a js object to be converted
     * @param request Indicates a NotificationRequest object from specified js object
     * @return Returns the null object if success, returns the null value otherwise
     */
    static napi_value GetNotificationLabel(const napi_env &env, const napi_value &value, NotificationRequest &request);

    /**
     * @brief Gets the badge icon style of NotificationRequest object from specified js object
     *
     * @param env Indicates the environment that the API is invoked under
     * @param value Indicates a js object to be converted
     * @param request Indicates a NotificationRequest object from specified js object
     * @return Returns the null object if success, returns the null value otherwise
     */
    static napi_value GetNotificationBadgeIconStyle(
        const napi_env &env, const napi_value &value, NotificationRequest &request);

    /**
     * @brief Gets the showDeliveryTime flag of NotificationRequest object from specified js object
     *
     * @param env Indicates the environment that the API is invoked under
     * @param value Indicates a js object to be converted
     * @param request Indicates a NotificationRequest object from specified js object
     * @return Returns the null object if success, returns the null value otherwise
     */
    static napi_value GetNotificationShowDeliveryTime(
        const napi_env &env, const napi_value &value, NotificationRequest &request);

    static napi_value GetNotificationIsRemoveAllowed(
        const napi_env &env, const napi_value &value, NotificationRequest &request);

    /**
     * @brief Gets the content of NotificationRequest object from specified js object
     *
     * @param env Indicates the environment that the API is invoked under
     * @param value Indicates a js object to be converted
     * @param request Indicates a NotificationRequest object from specified js object
     * @return Returns the null object if success, returns the null value otherwise
     */
    static napi_value GetNotificationContent(
        const napi_env &env, const napi_value &value, NotificationRequest &request);

    /**
     * @brief Gets the WantAgent object of NotificationRequest object from specified js object
     *
     * @param env Indicates the environment that the API is invoked under
     * @param value Indicates a js object to be converted
     * @param request Indicates a NotificationRequest object from specified js object
     * @return Returns the null object if success, returns the null value otherwise
     */
    static napi_value GetNotificationWantAgent(
        const napi_env &env, const napi_value &value, NotificationRequest &request);

    /**
     * @brief Gets a NotificationSlot object from specified js object
     *
     * @param env Indicates the environment that the API is invoked under
     * @param value Indicates a js object to be converted
     * @param slot Indicates a NotificationSlot object from specified js object
     * @return Returns the null object if success, returns the null value otherwise
     */
    static napi_value GetNotificationSlot(
        const napi_env &env, const napi_value &value, NotificationSlot &slot);

    /**
     * @brief Gets the string objects of NotificationSlot object from specified js object
     *
     * @param env Indicates the environment that the API is invoked under
     * @param value Indicates a js object to be converted
     * @param slot Indicates a NotificationSlot object from specified js object
     * @return Returns the null object if success, returns the null value otherwise
     */
    static napi_value GetNotificationSlotByString(
        const napi_env &env, const napi_value &value, NotificationSlot &slot);

    /**
     * @brief Gets the bool objects of NotificationSlot object from specified js object
     *
     * @param env Indicates the environment that the API is invoked under
     * @param value Indicates a js object to be converted
     * @param slot Indicates a NotificationSlot object from specified js object
     * @return Returns the null object if success, returns the null value otherwise
     */
    static napi_value GetNotificationSlotByBool(
        const napi_env &env, const napi_value &value, NotificationSlot &slot);

    /**
     * @brief Gets the number objects of NotificationSlot object from specified js object
     *
     * @param env Indicates the environment that the API is invoked under
     * @param value Indicates a js object to be converted
     * @param slot Indicates a NotificationSlot object from specified js object
     * @return Returns the null object if success, returns the null value otherwise
     */
    static napi_value GetNotificationSlotByNumber(
        const napi_env &env, const napi_value &value, NotificationSlot &slot);

    /**
     * @brief Gets the vibration of NotificationSlot object from specified js object
     *
     * @param env Indicates the environment that the API is invoked under
     * @param value Indicates a js object to be converted
     * @param slot Indicates a NotificationSlot object from specified js object
     * @return Returns the null object if success, returns the null value otherwise
     */
    static napi_value GetNotificationSlotByVibration(
        const napi_env &env, const napi_value &value, NotificationSlot &slot);

    /**
     * @brief Gets the action buttons of NotificationRequest object from specified js object
     *
     * @param env Indicates the environment that the API is invoked under
     * @param value Indicates a js object to be converted
     * @param request Indicates a NotificationRequest object from specified js object
     * @return Returns the null object if success, returns the null value otherwise
     */
    static napi_value GetNotificationActionButtons(
        const napi_env &env, const napi_value &value, NotificationRequest &request);

    /**
     * @brief Gets a NotificationActionButton object from specified js object
     *
     * @param env Indicates the environment that the API is invoked under
     * @param actionButton Indicates a js object to be converted
     * @param pActionButton Indicates a NotificationActionButton object from specified js object
     * @return Returns the null object if success, returns the null value otherwise
     */
    static napi_value GetNotificationActionButtonsDetailed(
        const napi_env &env, const napi_value &actionButton, std::shared_ptr<NotificationActionButton> &pActionButton);

    /**
     * @brief Gets the basic information of NotificationActionButton object from specified js object
     *
     * @param env Indicates the environment that the API is invoked under
     * @param actionButton Indicates a js object to be converted
     * @param pActionButton Indicates a NotificationActionButton object from specified js object
     * @return Returns the null object if success, returns the null value otherwise
     */
    static napi_value GetNotificationActionButtonsDetailedBasicInfo(
        const napi_env &env, const napi_value &actionButton, std::shared_ptr<NotificationActionButton> &pActionButton);

    /**
     * @brief Gets the extras of NotificationActionButton object from specified js object
     *
     * @param env Indicates the environment that the API is invoked under
     * @param actionButton Indicates a js object to be converted
     * @param pActionButton Indicates a NotificationActionButton object from specified js object
     * @return Returns the null object if success, returns the null value otherwise
     */
    static napi_value GetNotificationActionButtonsDetailedByExtras(
        const napi_env &env, const napi_value &actionButton, std::shared_ptr<NotificationActionButton> &pActionButton);

    /**
     * @brief Gets the user input of NotificationActionButton object from specified js object
     *
     * @param env Indicates the environment that the API is invoked under
     * @param actionButton Indicates a js object to be converted
     * @param pActionButton Indicates a NotificationActionButton object from specified js object
     * @return Returns the null object if success, returns the null value otherwise
     */
    static napi_value GetNotificationUserInput(
        const napi_env &env, const napi_value &actionButton, std::shared_ptr<NotificationActionButton> &pActionButton);

    /**
     * @brief Gets the input key of NotificationUserInput object from specified js object
     *
     * @param env Indicates the environment that the API is invoked under
     * @param userInputResult Indicates a js object to be converted
     * @param userInput Indicates a NotificationUserInput object from specified js object
     * @return Returns the null object if success, returns the null value otherwise
     */
    static napi_value GetNotificationUserInputByInputKey(
        const napi_env &env, const napi_value &userInputResult, std::shared_ptr<NotificationUserInput> &userInput);

    /**
     * @brief Gets the tag of NotificationUserInput object from specified js object
     *
     * @param env Indicates the environment that the API is invoked under
     * @param userInputResult Indicates a js object to be converted
     * @param userInput Indicates a NotificationUserInput object from specified js object
     * @return Returns the null object if success, returns the null value otherwise
     */
    static napi_value GetNotificationUserInputByTag(
        const napi_env &env, const napi_value &userInputResult, std::shared_ptr<NotificationUserInput> &userInput);

    /**
     * @brief Gets the options of NotificationUserInput object from specified js object
     *
     * @param env Indicates the environment that the API is invoked under
     * @param userInputResult Indicates a js object to be converted
     * @param userInput Indicates a NotificationUserInput object from specified js object
     * @return Returns the null object if success, returns the null value otherwise
     */
    static napi_value GetNotificationUserInputByOptions(
        const napi_env &env, const napi_value &userInputResult, std::shared_ptr<NotificationUserInput> &userInput);

    /**
     * @brief Gets the permit mime types of NotificationUserInput object from specified js object
     *
     * @param env Indicates the environment that the API is invoked under
     * @param userInputResult Indicates a js object to be converted
     * @param userInput Indicates a NotificationUserInput object from specified js object
     * @return Returns the null object if success, returns the null value otherwise
     */
    static napi_value GetNotificationUserInputByPermitMimeTypes(
        const napi_env &env, const napi_value &userInputResult, std::shared_ptr<NotificationUserInput> &userInput);

    /**
     * @brief Gets the permit free from input of NotificationUserInput object from specified js object
     *
     * @param env Indicates the environment that the API is invoked under
     * @param userInputResult Indicates a js object to be converted
     * @param userInput Indicates a NotificationUserInput object from specified js object
     * @return Returns the null object if success, returns the null value otherwise
     */
    static napi_value GetNotificationUserInputByPermitFreeFormInput(
        const napi_env &env, const napi_value &userInputResult, std::shared_ptr<NotificationUserInput> &userInput);

    /**
     * @brief Gets the edit type of NotificationUserInput object from specified js object
     *
     * @param env Indicates the environment that the API is invoked under
     * @param userInputResult Indicates a js object to be converted
     * @param userInput Indicates a NotificationUserInput object from specified js object
     * @return Returns the null object if success, returns the null value otherwise
     */
    static napi_value GetNotificationUserInputByEditType(
        const napi_env &env, const napi_value &userInputResult, std::shared_ptr<NotificationUserInput> &userInput);

    /**
     * @brief Gets the additional data of NotificationUserInput object from specified js object
     *
     * @param env Indicates the environment that the API is invoked under
     * @param userInputResult Indicates a js object to be converted
     * @param userInput Indicates a NotificationUserInput object from specified js object
     * @return Returns the null object if success, returns the null value otherwise
     */
    static napi_value GetNotificationUserInputByAdditionalData(
        const napi_env &env, const napi_value &userInputResult, std::shared_ptr<NotificationUserInput> &userInput);

    /**
     * @brief Gets the small icon of NotificationRequest object from specified js object
     *
     * @param env Indicates the environment that the API is invoked under
     * @param value Indicates a js object to be converted
     * @param request Indicates a NotificationRequest object from specified js object
     * @return Returns the null object if success, returns the null value otherwise
     */
    static napi_value GetNotificationSmallIcon(
        const napi_env &env, const napi_value &value, NotificationRequest &request);

    /**
     * @brief Gets the large icon of NotificationRequest object from specified js object
     *
     * @param env Indicates the environment that the API is invoked under
     * @param value Indicates a js object to be converted
     * @param request Indicates a NotificationRequest object from specified js object
     * @return Returns the null object if success, returns the null value otherwise
     */
    static napi_value GetNotificationLargeIcon(
        const napi_env &env, const napi_value &value, NotificationRequest &request);

    /**
     * @brief Gets the overlay icon of NotificationRequest object from specified js object
     *
     * @param env Indicates the environment that the API is invoked under
     * @param value Indicates a js object to be converted
     * @param request Indicates a NotificationRequest object from specified js object
     * @return Returns the null object if success, returns the null value otherwise
     */
    static napi_value GetNotificationOverlayIcon(
        const napi_env &env, const napi_value &value, NotificationRequest &request);

    /**
     * @brief Gets the distributed options of NotificationRequest object from specified js object
     *
     * @param env Indicates the environment that the API is invoked under
     * @param value Indicates a js object to be converted
     * @param request Indicates a NotificationRequest object from specified js object
     * @return Returns the null object if success, returns the null value otherwise
     */
    static napi_value GetNotificationRequestDistributedOptions(
        const napi_env &env, const napi_value &value, NotificationRequest &request);

    /**
     * @brief Gets the isDistributed flag of NotificationRequest object from specified js object
     *
     * @param env Indicates the environment that the API is invoked under
     * @param value Indicates a js object to be converted
     * @param request Indicates a NotificationRequest object from specified js object
     * @return Returns the null object if success, returns the null value otherwise
     */
    static napi_value GetNotificationIsDistributed(
        const napi_env &env, const napi_value &value, NotificationRequest &request);

    /**
     * @brief Gets the devices that support display of NotificationRequest object from specified js object
     *
     * @param env Indicates the environment that the API is invoked under
     * @param value Indicates a js object to be converted
     * @param request Indicates a NotificationRequest object from specified js object
     * @return Returns the null object if success, returns the null value otherwise
     */
    static napi_value GetNotificationSupportDisplayDevices(
        const napi_env &env, const napi_value &value, NotificationRequest &request);

    /**
     * @brief Gets the devices that support operation of NotificationRequest object from specified js object
     *
     * @param env Indicates the environment that the API is invoked under
     * @param value Indicates a js object to be converted
     * @param request Indicates a NotificationRequest object from specified js object
     * @return Returns the null object if success, returns the null value otherwise
     */
    static napi_value GetNotificationSupportOperateDevices(
        const napi_env &env, const napi_value &value, NotificationRequest &request);

    /**
     * @brief Gets a content type of notification from specified js object
     *
     * @param env Indicates the environment that the API is invoked under
     * @param value Indicates a js object to be converted
     * @param type Indicates a the content type of notification from specified js object
     * @return Returns the null object if success, returns the null value otherwise
     */
    static napi_value GetNotificationContentType(const napi_env &env, const napi_value &result, int32_t &type);

    /**
     * @brief Gets a basic content of NotificationRequest object from specified js object
     *
     * @param env Indicates the environment that the API is invoked under
     * @param value Indicates a js object to be converted
     * @param request Indicates a NotificationRequest object from specified js object
     * @return Returns the null object if success, returns the null value otherwise
     */
    static napi_value GetNotificationBasicContent(
        const napi_env &env, const napi_value &result, NotificationRequest &request);

    /**
     * @brief Gets a NotificationBasicContent object from specified js object
     *
     * @param env Indicates the environment that the API is invoked under
     * @param contentResult Indicates a js object to be converted
     * @param basicContent Indicates a NotificationBasicContent object from specified js object
     * @return Returns the null object if success, returns the null value otherwise
     */
    static napi_value GetNotificationBasicContentDetailed(
        const napi_env &env, const napi_value &contentResult, std::shared_ptr<NotificationBasicContent> basicContent);

    /**
     * @brief Gets a long-text content of NotificationRequest object from specified js object
     *
     * @param env Indicates the environment that the API is invoked under
     * @param value Indicates a js object to be converted
     * @param request Indicates a NotificationRequest object from specified js object
     * @return Returns the null object if success, returns the null value otherwise
     */
    static napi_value GetNotificationLongTextContent(
        const napi_env &env, const napi_value &result, NotificationRequest &request);

    /**
     * @brief Gets a NotificationLongTextContent object from specified js object
     *
     * @param env Indicates the environment that the API is invoked under
     * @param contentResult Indicates a js object to be converted
     * @param longContent Indicates a NotificationLongTextContent object from specified js object
     * @return Returns the null object if success, returns the null value otherwise
     */
    static napi_value GetNotificationLongTextContentDetailed(
        const napi_env &env, const napi_value &contentResult,
        std::shared_ptr<OHOS::Notification::NotificationLongTextContent> &longContent);

    /**
     * @brief Gets a picture content of NotificationRequest object from specified js object
     *
     * @param env Indicates the environment that the API is invoked under
     * @param result Indicates a js object to be converted
     * @param request Indicates a NotificationRequest object from specified js object
     * @return Returns the null object if success, returns the null value otherwise
     */
    static napi_value GetNotificationPictureContent(
        const napi_env &env, const napi_value &result, NotificationRequest &request);

    /**
     * @brief Gets a NotificationPictureContent object from specified js object
     *
     * @param env Indicates the environment that the API is invoked under
     * @param contentResult Indicates a js object to be converted
     * @param pictureContent Indicates a NotificationPictureContent object from specified js object
     * @return Returns the null object if success, returns the null value otherwise
     */
    static napi_value GetNotificationPictureContentDetailed(
        const napi_env &env, const napi_value &contentResult,
        std::shared_ptr<OHOS::Notification::NotificationPictureContent> &pictureContent);

    /**
     * @brief Gets a NotificationLocalLiveViewContent object from specified js object
     *
     * @param env Indicates the environment that the API is invoked under
     * @param result Indicates a js object to be converted
     * @param request Indicates a NotificationLocalLiveViewContent object from specified js object
     * @return Returns the null object if success, returns the null value otherwise
     */
    static napi_value GetNotificationLocalLiveViewContent(
        const napi_env &env, const napi_value &result, NotificationRequest &request);

    /**
     * @brief Gets a capsule of NotificationLocalLiveViewContent object from specified js object
     *
     * @param env Indicates the environment that the API is invoked under
     * @param contentResult Indicates a js object to be converted
     * @param content Indicates a capsule object from specified js object
     * @return Returns the null object if success, returns the null value otherwise
     */
    static napi_value GetNotificationLocalLiveViewCapsule(
        const napi_env &env, const napi_value &contentResult,
        std::shared_ptr<OHOS::Notification::NotificationLocalLiveViewContent> content);

    /**
     * @brief Gets a button of NotificationLocalLiveViewContent object from specified js object
     *
     * @param env Indicates the environment that the API is invoked under
     * @param contentResult Indicates a js object to be converted
     * @param content Indicates a button object from specified js object
     * @return Returns the null object if success, returns the null value otherwise
     */
    static napi_value GetNotificationLocalLiveViewButton(
        const napi_env &env, const napi_value &contentResult,
        std::shared_ptr<OHOS::Notification::NotificationLocalLiveViewContent> content);

    /**
     * @brief Gets a time of NotificationLocalLiveViewContent object from specified js object
     *
     * @param env Indicates the environment that the API is invoked under
     * @param contentResult Indicates a js object to be converted
     * @param content Indicates a time object from specified js object
     * @return Returns the null object if success, returns the null value otherwise
     */
    static napi_value GetNotificationLocalLiveViewTime(
        const napi_env &env, const napi_value &contentResult,
        std::shared_ptr<OHOS::Notification::NotificationLocalLiveViewContent> content);

    /**
     * @brief Gets a progress of NotificationLocalLiveViewContent object from specified js object
     *
     * @param env Indicates the environment that the API is invoked under
     * @param contentResult Indicates a js object to be converted
     * @param content Indicates a progress object from specified js object
     * @return Returns the null object if success, returns the null value otherwise
     */
    static napi_value GetNotificationLocalLiveViewProgress(
        const napi_env &env, const napi_value &contentResult,
        std::shared_ptr<OHOS::Notification::NotificationLocalLiveViewContent> content);

    /**
     * @brief Gets a NotificationLocalLiveViewContent object from specified js object
     *
     * @param env Indicates the environment that the API is invoked under
     * @param contentResult Indicates a js object to be converted
     * @param content Indicates a NotificationLocalLiveViewContent object from specified js object
     * @return Returns the null object if success, returns the null value otherwise
     */
    static napi_value GetNotificationLocalLiveViewContentDetailed(
        const napi_env &env, const napi_value &contentResult,
        std::shared_ptr<OHOS::Notification::NotificationLocalLiveViewContent> content);

    /**
     * @brief Gets a conversational content of NotificationRequest object from specified js object
     *
     * @param env Indicates the environment that the API is invoked under
     * @param result Indicates a js object to be converted
     * @param request Indicates a NotificationRequest object from specified js object
     * @return Returns the null object if success, returns the null value otherwise
     */
    static napi_value GetNotificationConversationalContent(
        const napi_env &env, const napi_value &result, NotificationRequest &request);

    /**
     * @brief Gets the user of NotificationConversationalContent object from specified js object
     *
     * @param env Indicates the environment that the API is invoked under
     * @param contentResult Indicates a js object to be converted
     * @param user Indicates a MessageUser object from specified js object
     * @return Returns the null object if success, returns the null value otherwise
     */
    static napi_value GetNotificationConversationalContentByUser(
        const napi_env &env, const napi_value &contentResult, MessageUser &user);

    /**
     * @brief Gets the title of NotificationConversationalContent object from specified js object
     *
     * @param env Indicates the environment that the API is invoked under
     * @param contentResult Indicates a js object to be converted
     * @param conversationalContent Indicates a NotificationConversationalContent object from specified js object
     * @return Returns the null object if success, returns the null value otherwise
     */
    static napi_value GetNotificationConversationalContentTitle(
        const napi_env &env, const napi_value &contentResult,
        std::shared_ptr<OHOS::Notification::NotificationConversationalContent> &conversationalContent);

    /**
     * @brief Gets the group of NotificationConversationalContent object from specified js object
     *
     * @param env Indicates the environment that the API is invoked under
     * @param contentResult Indicates a js object to be converted
     * @param conversationalContent Indicates a NotificationConversationalContent object from specified js object
     * @return Returns the null object if success, returns the null value otherwise
     */
    static napi_value GetNotificationConversationalContentGroup(
        const napi_env &env, const napi_value &contentResult,
        std::shared_ptr<OHOS::Notification::NotificationConversationalContent> &conversationalContent);

    /**
     * @brief Gets the messages of NotificationConversationalContent object from specified js object
     *
     * @param env Indicates the environment that the API is invoked under
     * @param contentResult Indicates a js object to be converted
     * @param conversationalContent Indicates a NotificationConversationalContent object from specified js object
     * @return Returns the null object if success, returns the null value otherwise
     */
    static napi_value GetNotificationConversationalContentMessages(
        const napi_env &env, const napi_value &contentResult,
        std::shared_ptr<OHOS::Notification::NotificationConversationalContent> &conversationalContent);

    /**
     * @brief Gets a NotificationConversationalMessage object from specified js object
     *
     * @param env Indicates the environment that the API is invoked under
     * @param conversationalMessage Indicates a js object to be converted
     * @param message Indicates a NotificationConversationalMessage object from specified js object
     * @return Returns the null object if success, returns the null value otherwise
     */
    static napi_value GetConversationalMessage(
        const napi_env &env, const napi_value &conversationalMessage,
        std::shared_ptr<NotificationConversationalMessage> &message);

    /**
     * @brief Gets the basic information of NotificationConversationalMessage object from specified js object
     *
     * @param env Indicates the environment that the API is invoked under
     * @param conversationalMessage Indicates a js object to be converted
     * @param message Indicates a NotificationConversationalMessage object from specified js object
     * @return Returns the null object if success, returns the null value otherwise
     */
    static napi_value GetConversationalMessageBasicInfo(
        const napi_env &env, const napi_value &conversationalMessage,
        std::shared_ptr<NotificationConversationalMessage> &message);

    /**
     * @brief Gets the other information of NotificationConversationalMessage object from specified js object
     *
     * @param env Indicates the environment that the API is invoked under
     * @param conversationalMessage Indicates a js object to be converted
     * @param message Indicates a NotificationConversationalMessage object from specified js object
     * @return Returns the null object if success, returns the null value otherwise
     */
    static napi_value GetConversationalMessageOtherInfo(
        const napi_env &env, const napi_value &conversationalMessage,
        std::shared_ptr<NotificationConversationalMessage> &message);

    /**
     * @brief Gets a MessageUser object from specified js object
     *
     * @param env Indicates the environment that the API is invoked under
     * @param result Indicates a js object to be converted
     * @param messageUser Indicates a MessageUser object from specified js object
     * @return Returns the null object if success, returns the null value otherwise
     */
    static napi_value GetMessageUser(const napi_env &env, const napi_value &result, MessageUser &messageUser);

    /**
     * @brief Gets a MessageUser object from specified js object
     *
     * @param env Indicates the environment that the API is invoked under
     * @param result Indicates a js object to be converted
     * @param messageUser Indicates a MessageUser object from specified js object
     * @return Returns the null object if success, returns the null value otherwise
     */
    static napi_value GetMessageUserByString(const napi_env &env, const napi_value &result, MessageUser &messageUser);

    /**
     * @brief Gets the bool objects of MessageUser object from specified js object
     *
     * @param env Indicates the environment that the API is invoked under
     * @param result Indicates a js object to be converted
     * @param messageUser Indicates a MessageUser object from specified js object
     * @return Returns the null object if success, returns the null value otherwise
     */
    static napi_value GetMessageUserByBool(const napi_env &env, const napi_value &result, MessageUser &messageUser);

    /**
     * @brief Gets the custom objects of MessageUser object from specified js object
     *
     * @param env Indicates the environment that the API is invoked under
     * @param result Indicates a js object to be converted
     * @param messageUser Indicates a MessageUser object from specified js object
     * @return Returns the null object if success, returns the null value otherwise
     */
    static napi_value GetMessageUserByCustom(const napi_env &env, const napi_value &result, MessageUser &messageUser);

    /**
     * @brief Gets the multi-line content of NotificationRequest object from specified js object
     *
     * @param env Indicates the environment that the API is invoked under
     * @param result Indicates a js object to be converted
     * @param request Indicates a NotificationRequest object from specified js object
     * @return Returns the null object if success, returns the null value otherwise
     */
    static napi_value GetNotificationMultiLineContent(
        const napi_env &env, const napi_value &result, NotificationRequest &request);

    /**
     * @brief Gets the lines of NotificationMultiLineContent object from specified js object
     *
     * @param env Indicates the environment that the API is invoked under
     * @param result Indicates a js object to be converted
     * @param multiLineContent Indicates a NotificationMultiLineContent object from specified js object
     * @return Returns the null object if success, returns the null value otherwise
     */
    static napi_value GetNotificationMultiLineContentLines(const napi_env &env, const napi_value &result,
        std::shared_ptr<OHOS::Notification::NotificationMultiLineContent> &multiLineContent);

    /**
     * @brief Gets the liveView content of NotificationRequest object from specified js object
     *
     * @param env Indicates the environment that the API is invoked under
     * @param result Indicates a js object to be converted
     * @param request Indicates a NotificationRequest object from specified js object
     * @return Returns the null object if success, returns the null value otherwise
     */
    static napi_value GetNotificationLiveViewContent(
        const napi_env &env, const napi_value &result, NotificationRequest &request);

    /**
     * @brief Gets a NotificationLiveViewContent object from specified js object
     *
     * @param env Indicates the environment that the API is invoked under
     * @param contentResult Indicates a js object to be converted
     * @param liveViewContent Indicates a NotificationMultiLineContent object from specified js object
     * @return Returns the null object if success, returns the null value otherwise
     */
    static napi_value GetNotificationLiveViewContentDetailed(const napi_env &env, const napi_value &contentResult,
        std::shared_ptr<NotificationLiveViewContent> &liveViewContent);

    /**
     * @brief Gets a GetLiveViewPictures from specified js object
     *
     * @param env Indicates the environment that the API is invoked under
     * @param picturesObj Indicates a js object to be converted
     * @param pictures Indicates pictures object from specified js object
     * @return Returns the null object if success, returns the null value otherwise
     */
    static napi_value GetLiveViewPictures(const napi_env &env, const napi_value &picturesObj,
        std::vector<std::shared_ptr<Media::PixelMap>> &pictures);

    /**
     * @brief Gets a GetLiveViewPictures from specified js object
     *
     * @param env Indicates the environment that the API is invoked under
     * @param pictureMapObj Indicates a js object to be converted
     * @param pictureMap Indicates picturemap from specified js object
     * @return Returns the null object if success, returns the null value otherwise
     */
    static napi_value GetLiveViewPictureInfo(const napi_env &env, const napi_value &pictureMapObj,
        std::map<std::string, std::vector<std::shared_ptr<Media::PixelMap>>> &pictureMap);

    /**
     * @brief Gets a NotificationBundleOption object from specified js object
     *
     * @param env Indicates the environment that the API is invoked under
     * @param value Indicates a js object to be converted
     * @param option Indicates a NotificationBundleOption object from specified js object
     * @return Returns the null object if success, returns the null value otherwise
     */
    static napi_value GetBundleOption(const napi_env &env, const napi_value &value, NotificationBundleOption &option);

    /**
     * @brief Gets a NotificationButtonOption object from specified js object
     *
     * @param env Indicates the environment that the API is invoked under
     * @param value Indicates a js object to be converted
     * @param option Indicates a NotificationButtonOption object from specified js object
     * @return Returns the null object if success, returns the null value otherwise
     */
    static napi_value GetButtonOption(const napi_env &env, const napi_value &value, NotificationButtonOption &option);

    static napi_value GetHashCodes(const napi_env &env, const napi_value &value, std::vector<std::string> &hashCodes);

    /**
     * @brief Gets a NotificationKey object from specified js object
     *
     * @param env Indicates the environment that the API is invoked under
     * @param value Indicates a js object to be converted
     * @param key Indicates a NotificationKey object from specified js object
     * @return Returns the null object if success, returns the null value otherwise
     */
    static napi_value GetNotificationKey(const napi_env &env, const napi_value &value, NotificationKey &key);

    /**
     * @brief Gets the template of NotificationRequest object from specified js object
     *
     * @param env Indicates the environment that the API is invoked under
     * @param value Indicates a js object to be converted
     * @param request Indicates a NotificationRequest object from specified js object
     * @return Returns the null object if success, returns the null value otherwise
     */
    static napi_value GetNotificationTemplate(
        const napi_env &env, const napi_value &value, NotificationRequest &request);

    /**
     * @brief Gets a NotificationTemplate object from specified js object
     *
     * @param env Indicates the environment that the API is invoked under
     * @param value Indicates a js object to be converted
     * @param templ Indicates a NotificationTemplate object from specified js object
     * @return Returns the null object if success, returns the null value otherwise
     */
    static napi_value GetNotificationTemplateInfo(const napi_env &env, const napi_value &value,
        std::shared_ptr<NotificationTemplate> &templ);

    /**
     * @brief Gets the number of badge of NotificationRequest object from specified js object
     *
     * @param env Indicates the environment that the API is invoked under
     * @param value Indicates a js object to be converted
     * @param request Indicates a NotificationRequest object from specified js object
     * @return Returns the null object if success, returns the null value otherwise
     */
    static napi_value GetNotificationBadgeNumber(
        const napi_env &env, const napi_value &value, NotificationRequest &request);

    /**
     * @brief Gets a NotificationUnifiedGroupInfo object from specified js object
     *
     * @param env Indicates the environment that the API is invoked under
     * @param value Indicates a js object to be converted
     * @param templ Indicates a NotificationUnifiedGroupInfo object from specified js object
     * @return Returns the null object if success, returns the null value otherwise
     */
    static napi_value GetNotificationUnifiedGroupInfo(
        const napi_env &env, const napi_value &value, NotificationRequest &request);

    /**
     * @brief Gets the notification control flags of NotificationRequest object from specified js object.
     *
     * @param env Indicates the environment that the API is invoked under
     * @param value Indicates a js object to be converted
     * @param request Indicates a NotificationRequest object from specified js object
     * @return Returns the null object if success, returns the null value otherwise
     */
    static napi_value GetNotificationControlFlags(
        const napi_env &env, const napi_value &value, NotificationRequest &request);

    /**
     * @brief Create a napi value with specified error object for callback
     *
     * @param env Indicates the environment that the API is invoked under
     * @param errCode Indicates specified err code
     * @return Returns a napi value with specified error object for callback
     */
    static napi_value CreateErrorValue(napi_env env, int32_t errCode, bool newType);

    /**
     * @brief Gets the notificationBundleOption of NotificationRequest object from specified js object
     *
     * @param env Indicates the environment that the API is invoked under
     * @param value Indicates a js object to be converted
     * @param request Indicates a NotificationRequest object from specified js object
     * @return Returns the null object if success, returns the null value otherwise
     */
    static napi_value GetNotificationBundleOption(
        const napi_env &env, const napi_value &value, NotificationRequest &request);
    static bool IsValidRemoveReason(int32_t reasonType);
    static void NapiThrow(napi_env env, int32_t errCode);
    static napi_value GetLockScreenPicture(
        const napi_env &env, const napi_value &contentResult, std::shared_ptr<NotificationBasicContent> basicContent);

private:
    static const int32_t ARGS_ONE = 1;
    static const int32_t ARGS_TWO = 2;
    static const int32_t ONLY_CALLBACK_MAX_PARA = 1;
    static const int32_t ONLY_CALLBACK_MIN_PARA = 0;
    static std::set<std::shared_ptr<AbilityRuntime::WantAgent::WantAgent>> wantAgent_;
    static std::mutex mutex_;
    static const char *GetPropertyNameByContentType(ContentType type);
};
}  // namespace Location
}  // namespace OHOS

#endif  // BASE_LOCATION_DISTRIBUTED_NOTIFICATION_SERVICE_FRAMEWORKS_JS_NAPI_INCLUDE_COMMON_H
