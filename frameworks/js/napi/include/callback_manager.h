/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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

#ifndef CALLBACK_MANAGER_H
#define CALLBACK_MANAGER_H

#include <unistd.h>
#include "napi_util.h"
#include "location_napi_event.h"

namespace OHOS {
namespace Location {
template <typename T>
class CallbackManager {
public:
    CallbackManager() = default;
    virtual ~CallbackManager() = default;
    bool IsCallbackInMap(napi_env& env, napi_value& handler);
    void AddCallback(napi_env& env, napi_ref& handlerRef, sptr<T>& callback);
    void DeleteCallback(napi_env& env, napi_value& handler);
    sptr<T> GetCallbackPtr(napi_env& env, napi_value& handler);
    bool UnsubscribeAllCallback(napi_env& env, std::string event);
private:
    std::map<napi_env, std::map<napi_ref, sptr<T>>> callbackMap_;
    bool UnsubscribeCallBackByEvent(napi_env& env, std::string event, sptr<T> callbackHost);
};

template <typename T>
bool CallbackManager<T>::IsCallbackInMap(napi_env& env, napi_value& handler)
{
    auto iter = callbackMap_.find(env);
    if (iter == callbackMap_.end()) {
        return false;
    }
    for (auto innerIter = iter->second.begin(); innerIter != iter->second.end(); innerIter++) {
        auto ref = innerIter->first;
        if (IsCallbackEquals(env, handler, ref)) {
            return true;
        }
    }
    return false;
}

template <typename T>
void CallbackManager<T>::AddCallback(napi_env& env, napi_ref& handlerRef, sptr<T>& callback)
{
    auto iter = callbackMap_.find(env);
    if (iter == callbackMap_.end()) {
        std::map<napi_ref, sptr<T>> innerMap;
        innerMap.insert(std::make_pair(handlerRef, callback));
        callbackMap_.insert(std::make_pair(env, innerMap));
        return;
    }
    iter->second.insert(std::make_pair(handlerRef, callback));
}

template <typename T>
void CallbackManager<T>::DeleteCallback(napi_env& env, napi_value& handler)
{
    auto iter = callbackMap_.find(env);
    if (iter == callbackMap_.end()) {
        return;
    }
    for (auto innerIter = iter->second.begin(); innerIter != iter->second.end(); innerIter++) {
        auto ref = innerIter->first;
        if (IsCallbackEquals(env, handler, ref)) {
            innerIter = iter->second.erase(innerIter);
            if (iter->second.size() == 0) {
                callbackMap_.erase(iter);
            }
            break;
        }
    }
}

template <typename T>
sptr<T> CallbackManager<T>::GetCallbackPtr(napi_env& env, napi_value& handler)
{
    auto iter = callbackMap_.find(env);
    if (iter == callbackMap_.end()) {
        return nullptr;
    }
    for (auto innerIter = iter->second.begin(); innerIter != iter->second.end(); innerIter++) {
        auto ref = innerIter->first;
        if (IsCallbackEquals(env, handler, ref)) {
            return innerIter->second;
        }
    }
    return nullptr;
}

template <typename T>
bool CallbackManager<T>::UnsubscribeCallBackByEvent(napi_env& env, std::string event, sptr<T> callbackHost) {
    if (callbackHost == nullptr) {
        return false;
    }
    if (event == "locationServiceState") {
        UnSubscribeLocationServiceState(callbackHost);
        callbackHost->DeleteHandler();
        callbackHost = nullptr;
    } else if (event == "locationChange") {
        UnSubscribeLocationChange(callbackHost);
        callbackHost->DeleteAllCallbacks();
        callbackHost = nullptr;
    } else if (event == "gnssStatusChange") {
        UnSubscribeGnssStatus(callbackHost);
        callbackHost->DeleteHandler();
        callbackHost = nullptr;
    } else if (event == "nmeaMessageChange") {
        UnSubscribeNmeaMessage(callbackHost);
        callbackHost->DeleteHandler();
        callbackHost = nullptr;
    } else if (event == "cachedGnssLocationsReporting") {
        auto cachedCallback = sptr<ICachedLocationsCallback>(callbackHost);
        UnSubscribeCacheLocationChange(cachedCallback);
        cachedCallbackHost = nullptr;
    } else if (event == "fenceStatusChange") {
        UnSubscribeFenceStatusChange(env, null, null);
    } else if (event == "countryCodeChange") {
        UnsubscribeCountryCodeChange(callbackHost);
        callbackHost->DeleteHandler();
        callbackHost = nullptr;
    }
    return true;
}

template <typename T>
bool CallbackManager<T>::UnsubscribeAllCallback(napi_env& env, std::string event)
{
    auto iter = callbackMap_.find(env);
    if (iter == callbackMap_.end()) {
        return false;
    }
    for (auto innerIter = iter->second.begin(); innerIter != iter->second.end(); innerIter++) {
        auto callbackHost = innerIter->second;
        UnsubscribeCallBackByEvent(env, event, callbackHost);
    }
    callbackMap_.clear();
    return false;
}
} // namespace Location
} // namespace OHOS
#endif // CALLBACK_MANAGER_H
