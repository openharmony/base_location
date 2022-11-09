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

#ifndef REQUEST_H
#define REQUEST_H

#include <list>
#include <string>

#include "i_locator_callback.h"
#include "request_config.h"
#include "location_mock_config.h"

namespace OHOS {
namespace Location {
class Request {
public:
    Request();
    ~Request();
    pid_t GetUid();
    pid_t GetPid();
    std::string GetPackageName();
    sptr<RequestConfig> GetRequestConfig();
    sptr<ILocatorCallback> GetLocatorCallBack();
    std::string GetUUid();
    void SetUid(pid_t uid);
    void SetPid(pid_t pid);
    void SetPackageName(std::string packageName);
    void SetRequestConfig(RequestConfig& requestConfig);
    void SetLocationMockConfig(const LocationMockConfig& locationMockConfig);
    void SetLocatorCallBack(const sptr<ILocatorCallback>& callback);
    void SetUUid(std::string uuid);
    std::string ToString() const;
    void GetProxyName(std::shared_ptr<std::list<std::string>> proxys);
    bool GetIsRequesting();
    void SetRequesting(bool state);
    sptr<Location> GetLastLocation();
    void SetLastLocation(const std::unique_ptr<Location>& location);
    uint32_t GetTokenId();
    uint32_t GetFirstTokenId();
    void SetTokenId(uint32_t tokenId);
    void SetFirstTokenId(uint32_t firstTokenId);
    bool GetLocationPermState();
    bool GetBackgroundPermState();
    bool GetApproximatelyPermState();
    void SetLocationPermState(bool state);
    void SetBackgroundPermState(bool state);
    void SetApproximatelyPermState(bool state);
private:
    void GetProxyNameByPriority(std::shared_ptr<std::list<std::string>> proxys);

    pid_t uid_;
    pid_t pid_;
    uint32_t tokenId_;
    uint32_t firstTokenId_;
    sptr<Location> lastLocation_;
    std::string packageName_;
    std::string uuid_;
    sptr<RequestConfig> requestConfig_;
    sptr<ILocatorCallback> callBack_;
    bool isRequesting_;
    bool isUsingLocationPerm_;
    bool isUsingBackgroundPerm_;
    bool isUsingApproximatelyPerm_;
};
} // namespace Location
} // namespace OHOS
#endif // REQUEST_H
