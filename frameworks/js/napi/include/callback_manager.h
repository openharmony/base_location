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

#include <mutex>

namespace OHOS {
namespace Location {
template <typename T>
class CallbackManager {
public:
    CallbackManager() = default;
    virtual ~CallbackManager() = default;
    bool IsCallbackInMap(const napi_env& env, const napi_value& handler);
    void AddCallback(const napi_env& env, const napi_ref& handlerRef, const sptr<T>& callback);
    void DeleteCallback(const napi_env& env, const napi_value& handler);
    sptr<T> GetCallbackPtr(const napi_env& env, const napi_value& handler);
    void DeleteCallbackByEnv(const napi_env& env);
    std::map<napi_env, std::map<napi_ref, sptr<T>>> GetCallbackMap();
private:
    std::map<napi_env, std::map<napi_ref, sptr<T>>> callbackMap_;
    std::mutex mMutex;
};

template <typename T>
std::map<napi_env, std::map<napi_ref, sptr<T>>> CallbackManager<T>::GetCallbackMap()
{
    std::unique_lock<std::mutex> lock(mMutex);
    return callbackMap_;
}

template <typename T>
void CallbackManager<T>::DeleteCallbackByEnv(const napi_env& env)
{
    std::unique_lock<std::mutex> lock(mMutex);
    auto iter = callbackMap_.find(env);
    if (iter == callbackMap_.end()) {
        return;
    }
    iter->second.clear();
    callbackMap_.erase(iter);
}

template <typename T>
bool CallbackManager<T>::IsCallbackInMap(const napi_env& env, const napi_value& handler)
{
    std::unique_lock<std::mutex> lock(mMutex);
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
void CallbackManager<T>::AddCallback(const napi_env& env, const napi_ref& handlerRef, const sptr<T>& callback)
{
    std::unique_lock<std::mutex> lock(mMutex);
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
void CallbackManager<T>::DeleteCallback(const napi_env& env, const napi_value& handler)
{
    std::unique_lock<std::mutex> lock(mMutex);
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
sptr<T> CallbackManager<T>::GetCallbackPtr(const napi_env& env, const napi_value& handler)
{
    std::unique_lock<std::mutex> lock(mMutex);
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
} // namespace Location
} // namespace OHOS
#endif // CALLBACK_MANAGER_H
