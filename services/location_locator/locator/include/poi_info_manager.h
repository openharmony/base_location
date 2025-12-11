/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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

#ifndef POI_INFO_MANAGER
#define POI_INFO_MANAGER

#include <mutex>
#include <singleton.h>
#include <string>
#include <time.h>
#include <cJSON.h>

#include "ability_connect_callback_interface.h"
#include "event_handler.h"
#include "event_runner.h"
#include "ffrt.h"
#include "system_ability.h"
#include "common_utils.h"
#include "constant_definition.h"
#include "subability_common.h"
#include "iremote_stub.h"
#include "i_poi_info_callback.h"
#include "location.h"

namespace OHOS {
namespace Location {

typedef struct StrPoiInfoStruct {
    uint64_t poiInfosTime = 0;
    std::string latestPoiInfos = "";
} StrPoiInfoStruct;

class PoiInfoHandler : public AppExecFwk::EventHandler {
public:
    using PoiInfoEventHandler = std::function<void(const AppExecFwk::InnerEvent::Pointer &)>;
    using PoiInfoEventHandleMap = std::map<int, PoiInfoEventHandler>;
    explicit PoiInfoHandler(const std::shared_ptr<AppExecFwk::EventRunner>& runner);
    ~PoiInfoHandler() override;

private:
    void ProcessEvent(const AppExecFwk::InnerEvent::Pointer& event) override;
    void InitPoiInfoEventProcessMap();
    void HandleRequestPoiInfo(const AppExecFwk::InnerEvent::Pointer& event);
    void HandleResetServiceProxy(const AppExecFwk::InnerEvent::Pointer& event);
    PoiInfoEventHandleMap poiEventProcessMap_;
};

class PoiServiceDeathRecipient : public IRemoteObject::DeathRecipient {
public:
    void OnRemoteDied(const wptr<IRemoteObject> &remote) override;
    PoiServiceDeathRecipient();
    ~PoiServiceDeathRecipient() override;
};

class PoiInfoCallback : public IRemoteStub<IPoiInfoCallback> {
public:
    PoiInfoCallback();
    ~PoiInfoCallback();
    int32_t OnRemoteRequest(uint32_t code,
        MessageParcel &data, MessageParcel &reply, MessageOption &option) override;
    void OnPoiInfoChange(std::shared_ptr<PoiInfo> &results) override;
    void OnErrorReport(const std::string errorCode) override;
    bool ReportPoiPermissionCheck(AppIdentity identity);
    sptr<IRemoteObject> cb_;
    AppIdentity identity_;
};

typedef struct PoiInfoRequest {
    sptr<IRemoteObject> callback;
    AppIdentity identity;
}PoiInfoRequest;

class PoiInfoManager {
public:
    PoiInfoManager();
    ~PoiInfoManager();
    static PoiInfoManager* GetInstance();
    void UpdateCachedPoiInfo(const std::unique_ptr<Location>& location);
    void ClearPoiInfos(const std::unique_ptr<Location>& finalLocation);
    void UpdateLocationPoiInfo(const std::unique_ptr<Location>& finalLocation);

    void PreRequestPoiInfo(const sptr<IRemoteObject>& cb, AppIdentity identity);
    void RequestPoiInfo(sptr<IRemoteObject>& cb, AppIdentity identity);
    bool ConnectPoiService();
    void PreDisconnectAbilityConnect();
    void DisconnectAbilityConnect();
    void RegisterPoiServiceDeathRecipient();
    void UnregisterPoiServiceDeathRecipient();
    bool PreResetServiceProxy();
    void ResetServiceProxy();
    bool IsConnect();
    void NotifyConnected(const sptr<IRemoteObject>& remoteObject);
    void NotifyDisConnected();

    std::string GetLatestPoiInfo();
    void SetLatestPoiInfo(std::string poiInfo);
    uint64_t GetLatestPoiInfoTime();
    void SetLatestPoiInfoTime(uint64_t poiInfoTime);

    uint64_t GetPoiInfoTime(const std::string& poiInfos);
    bool IsPoiInfoValid(std::string poiInfos, uint64_t poiInfoTime);
    void AddCachedPoiInfoToLocation(const std::unique_ptr<Location>& finalLocation);
    Poi ParsePoiInfo(cJSON* poiJson);
    PoiInfo ParsePoiInfoFromStr(const std::string& jsonString);

private:
    std::mutex latestPoiInfoMutex_;
    StrPoiInfoStruct latestPoiInfoStruct_;
    std::shared_ptr<PoiInfoHandler> poiInfoHandler_;
    ffrt::mutex poiServiceMutex_;
    ffrt::mutex connMutex_;
    sptr<IRemoteObject> poiServiceProxy_;
    ffrt::condition_variable connectCondition_;
    sptr<AAFwk::IAbilityConnection> conn_;
    sptr<IRemoteObject::DeathRecipient> poiServiceRecipient_ = sptr<PoiServiceDeathRecipient>(new
        PoiServiceDeathRecipient());
};
} // namespace OHOS
} // namespace Location
#endif // POI_INFO_MANAGER