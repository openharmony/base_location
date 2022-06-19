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

namespace OHOS {
namespace Location {
template <typename T>
class CallbackManager {
public:
    CallbackManager();
    virtual ~CallbackManager() = default;
    bool IsCallbackInMap();
private:
    std::map<napi_env, std::map<napi_ref, sptr<T>>> callbackMap_;
};

template <typename T>
inline bool CallbackManager::IsCallbackInMap(napi_env env, napi_value& callback)
{
    for (auto iter = callbackMap_.begin(); iter != callbackMap_.end(); iter++) {
        if (iter->first != env) {
            continue;
        }
        for (auto innerIter = iter->second.begin(); innerIter != iter->second.end(); innerIter++) {
            if (IsCallbackEquals(env, callback, innerIter->first)) {
                LBSLOGE(LOCATION_NAPI, "this request is already started, just return.");
                return true;
            }
        }
    }
    return false;
}

template <typename T>
inline void CallbackManager::AddCallback(napi_env env, napi_ref& handlerRef, sptr<T>& callback)
{
    std::map<napi_ref, sptr<T>> innerMap;
    innerMap.insert(std::make_pair(handlerRef, callback));
    callbackMap_.insert(std::make_pair(env, innerMap));
}

template <typename T>
inline void CallbackManager::DeleteCallback(napi_env env, napi_value& callback)//这里还需要传入一个函数指针，用于进行注销动作
{
    for (auto iter = callbackMap_.begin(); iter != callbackMap_.end(); iter++) {
        if (iter->first != env) {
            continue;
        }
        for (auto innerIter = iter->second.begin(); innerIter != iter->second.end();) {
            if (IsCallbackEquals(env, callback, innerIter->first)) {
                LBSLOGE(LOCATION_NAPI, "this request is already started, just return.");
                sptr<T> switchCallbackHost = innerIter->second;
                UnSubscribeLocationServiceState(switchCallbackHost);
                switchCallbackHost->DeleteHandler();
                innerIter = switchMap.erase(innerIter);
            } else {
                innerIter++;
            }
        }
    }
}
} // namespace Location
} // namespace OHOS
#endif // CALLBACK_MANAGER_H
