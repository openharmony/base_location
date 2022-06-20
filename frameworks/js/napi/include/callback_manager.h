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

namespace OHOS {
namespace Location {
template <typename T>
class CallbackManager {
public:
    CallbackManager();
    virtual ~CallbackManager() = default;
    bool IsCallbackInMap(napi_env& env, napi_value& handler);
    void AddCallback(napi_env& env, napi_ref& handlerRef, sptr<T>& callback);
    void DeleteCallback(napi_env& env, napi_value& handler);
    sptr<T> GetCallbackPtr(napi_env& env, napi_value& handler);
private:
    std::map<napi_env, std::map<napi_ref, sptr<T>>> callbackMap_;
};

template <typename T>
inline bool CallbackManager::IsCallbackInMap(napi_env& env, napi_value& handler)
{
    auto iter = callbackMap_.find(env);
    if (iter == callbackMap_.end()) {
        return false;
    }
    for (auto innerIter = iter->second.begin(); innerIter != iter->second.end(); innerIter++) {
        if (IsCallbackEquals(env, handler, innerIter->first)) {
            return true;
        }
    }
    return false;
}

template <typename T>
inline void CallbackManager::AddCallback(napi_env& env, napi_ref& handlerRef, sptr<T>& callback)
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
inline void CallbackManager::DeleteCallback(napi_env& env, napi_value& handler)
{
    auto iter = callbackMap_.find(env);
    if (iter == callbackMap_.end()) {
        return;
    }
    for (auto innerIter = iter->second.begin(); innerIter != iter->second.end();) {
        if (IsCallbackEquals(env, handler, innerIter->first)) {
            innerIter = iter->second.erase(innerIter);
            if (iter->second.size() == 0) {
                callbackMap_.erase(iter);
            }
            break;
        } else {
            innerIter++;
        }
    }
}

template <typename T>
inline sptr<T> CallbackManager::GetCallbackPtr(napi_env& env, napi_value& handler)
{
    auto iter = callbackMap_.find(env);
    if (iter == callbackMap_.end()) {
        return nullptr;
    }
    for (auto innerIter = iter->second.begin(); innerIter != iter->second.end(); innerIter++) {
        if (IsCallbackEquals(env, handler, innerIter->first)) {
            return innerIter->second;
        }
    }
    return nullptr;
}
} // namespace Location
} // namespace OHOS
#endif // CALLBACK_MANAGER_H
