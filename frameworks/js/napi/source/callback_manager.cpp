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

#include "callback_manager.h"

namespace OHOS {
namespace Location {
bool CallbackManager::IsCallbackInMap(napi_env& env, napi_value& handler)
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
void CallbackManager::AddCallback(napi_env& env, napi_ref& handlerRef, sptr<T>& callback)
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

void CallbackManager::DeleteCallback(napi_env& env, napi_value& handler)
{
    auto iter = callbackMap_.find(env);
    if (iter == callbackMap_.end()) {
        return;
    }
    for (auto innerIter = iter->second.begin(); innerIter != iter->second.end(); innerIter++) {
        if (IsCallbackEquals(env, handler, innerIter->first)) {
            innerIter = iter->second.erase(innerIter);
            if (iter->second.size() == 0) {
                callbackMap_.erase(iter);
            }
            break;
        }
    }
}

template <typename T>
sptr<T> CallbackManager::GetCallbackPtr(napi_env& env, napi_value& handler)
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
