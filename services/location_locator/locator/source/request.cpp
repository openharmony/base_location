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

#include "request.h"
#include "common_utils.h"
#include "constant_definition.h"

namespace OHOS {
namespace Location {
Request::Request()
{
    this->pid_ = -1;
    this->uid_ = -1;
    this->tokenId_ = 0;
    this->firstTokenId_ = 0;
    this->packageName_ = "";
    this->isRequesting_ = false;
    requestConfig_ = new (std::nothrow) RequestConfig();
    lastLocation_ = new (std::nothrow) Location();
    isUsingLocationPerm_ = false;
    isUsingBackgroundPerm_ = false;
    isUsingApproximatelyPerm_ = false;
}

Request::~Request() {}

void Request::SetRequestConfig(RequestConfig& requestConfig)
{
    if (this->requestConfig_ == nullptr) {
        return;
    }
    this->requestConfig_->Set(requestConfig);
}

void Request::SetLocatorCallBack(const sptr<ILocatorCallback>& callback)
{
    this->callBack_ = callback;
}

sptr<RequestConfig> Request::GetRequestConfig()
{
    return requestConfig_;
}

sptr<ILocatorCallback> Request::GetLocatorCallBack()
{
    return callBack_;
}

void Request::SetUid(pid_t uid)
{
    this->uid_ = uid;
}

pid_t Request::GetUid()
{
    return uid_;
}

void Request::SetPid(pid_t pid)
{
    this->pid_ = pid;
}

pid_t Request::GetPid()
{
    return pid_;
}

void Request::SetTokenId(uint32_t tokenId)
{
    this->tokenId_ = tokenId;
}

uint32_t Request::GetTokenId()
{
    return tokenId_;
}

void Request::SetFirstTokenId(uint32_t firstTokenId)
{
    this->firstTokenId_ = firstTokenId;
}

uint32_t Request::GetFirstTokenId()
{
    return firstTokenId_;
}

void Request::SetPackageName(std::string packageName)
{
    this->packageName_ = packageName;
}

std::string Request::GetPackageName()
{
    return packageName_;
}

bool Request::GetIsRequesting()
{
    return isRequesting_;
}

void Request::SetRequesting(bool state)
{
    this->isRequesting_ = state;
}

sptr<Location> Request::GetLastLocation()
{
    return lastLocation_;
}

std::string Request::GetUuid()
{
    return uuid_;
}

void Request::SetUuid(std::string uuid)
{
    this->uuid_ = uuid;
}

void Request::SetLastLocation(const std::unique_ptr<Location>& location)
{
    if (this->lastLocation_ == nullptr) {
        return;
    }
    this->lastLocation_->SetLatitude(location->GetLatitude());
    this->lastLocation_->SetLongitude(location->GetLongitude());
    this->lastLocation_->SetAltitude(location->GetAltitude());
    this->lastLocation_->SetAccuracy(location->GetAccuracy());
    this->lastLocation_->SetSpeed(location->GetSpeed());
    this->lastLocation_->SetDirection(location->GetDirection());
    this->lastLocation_->SetTimeStamp(location->GetTimeStamp());
    this->lastLocation_->SetTimeSinceBoot(location->GetTimeSinceBoot());
}

void Request::GetProxyName(std::shared_ptr<std::list<std::string>> proxys)
{
    if (requestConfig_ == nullptr || proxys == nullptr) {
        return;
    }
    switch (requestConfig_->GetScenario()) {
        case SCENE_NAVIGATION:
        case SCENE_TRAJECTORY_TRACKING:
        case SCENE_CAR_HAILING: {
            proxys->push_back(GNSS_ABILITY);
            break;
        }
        case SCENE_DAILY_LIFE_SERVICE: {
            proxys->push_back(NETWORK_ABILITY);
            break;
        }
        case SCENE_NO_POWER: {
            proxys->push_back(PASSIVE_ABILITY);
            break;
        }
        case SCENE_UNSET: {
            GetProxyNameByPriority(proxys);
            break;
        }
        default:
            break;
    }
}

void Request::GetProxyNameByPriority(std::shared_ptr<std::list<std::string>> proxys)
{
    if (requestConfig_ == nullptr || proxys == nullptr) {
        return;
    }
    switch (requestConfig_->GetPriority()) {
        case PRIORITY_ACCURACY:
            proxys->push_back(GNSS_ABILITY);
            break;
        case PRIORITY_LOW_POWER:
            proxys->push_back(NETWORK_ABILITY);
            break;
        case PRIORITY_FAST_FIRST_FIX:
            proxys->push_back(GNSS_ABILITY);
            proxys->push_back(NETWORK_ABILITY);
            break;
        default:
            break;
    }
}

bool Request::GetLocationPermState()
{
    return isUsingLocationPerm_;
}

bool Request::GetBackgroundPermState()
{
    return isUsingBackgroundPerm_;
}

bool Request::GetApproximatelyPermState()
{
    return isUsingApproximatelyPerm_;
}

void Request::SetLocationPermState(bool state)
{
    isUsingLocationPerm_ = state;
}

void Request::SetBackgroundPermState(bool state)
{
    isUsingBackgroundPerm_ = state;
}

void Request::SetApproximatelyPermState(bool state)
{
    isUsingApproximatelyPerm_ = state;
}

std::string Request::ToString() const
{
    if (requestConfig_ == nullptr) {
        return "";
    }
    std::string str = "[request config: " + requestConfig_->ToString() +
        "] from pid:" + std::to_string(pid_) +
        ", uid:" + std::to_string(uid_) +
        ", tokenId:" + std::to_string(tokenId_) +
        ", firstTokenId:" + std::to_string(firstTokenId_) +
        ", uuid:" + uuid_ + ", packageName:" + packageName_;
    return str;
}
} // namespace Location
} // namespace OHOS