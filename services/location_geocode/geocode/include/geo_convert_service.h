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

#ifndef GEO_CONVERT_SERVICE_H
#define GEO_CONVERT_SERVICE_H
#ifdef FEATURE_GEOCODE_SUPPORT

#include <mutex>
#include <singleton.h>
#include <string>
#include <vector>

#include "if_system_ability_manager.h"
#include "iremote_object.h"
#include "message_parcel.h"
#include "message_option.h"
#include "system_ability.h"

#include "common_utils.h"
#include "constant_definition.h"
#include "geo_coding_mock_info.h"
#include "geo_convert_callback_host.h"
#include "geo_convert_skeleton.h"

namespace OHOS {
namespace Location {
class GeoConvertService : public SystemAbility, public GeoConvertServiceStub {
DECLEAR_SYSTEM_ABILITY(GeoConvertService);

public:
    DISALLOW_COPY_AND_MOVE(GeoConvertService);
    static GeoConvertService* GetInstance();

    GeoConvertService();
    ~GeoConvertService() override;
    void OnStart() override;
    void OnStop() override;
    ServiceRunningState QueryServiceState() const
    {
        return state_;
    }
    int IsGeoConvertAvailable(MessageParcel &reply) override;
    int GetAddressByCoordinate(MessageParcel &data, MessageParcel &reply) override;
    int GetAddressByLocationName(MessageParcel &data, MessageParcel &reply) override;
    bool EnableReverseGeocodingMock() override;
    bool DisableReverseGeocodingMock() override;
    LocationErrCode SetReverseGeocodingMockInfo(std::vector<std::shared_ptr<GeocodingMockInfo>>& mockInfo) override;
    int32_t Dump(int32_t fd, const std::vector<std::u16string>& args) override;
    void UnloadGeoConvertSystemAbility() override;

    bool ConnectService();
    bool ReConnectService();
    void NotifyConnected(const sptr<IRemoteObject>& remoteObject);
    void NotifyDisConnected();
    bool ResetServiceProxy();
private:
    bool Init();
    static void SaDumpInfo(std::string& result);
    int RemoteToService(uint32_t code, MessageParcel &data, MessageParcel &rep);
    void ReportAddressMock(MessageParcel &data, MessageParcel &reply);
    bool CheckIfGeoConvertConnecting();
    bool WriteInfoToParcel(MessageParcel &data, MessageParcel &dataParcel, bool flag);
    bool WriteResultToParcel(const std::list<std::shared_ptr<GeoAddress>> result, MessageParcel &reply, bool flag);
    bool GetService();
    bool IsConnect();
    void RegisterGeoServiceDeathRecipient();
    bool SendGeocodeRequest(int code, MessageParcel& dataParcel, MessageParcel& replyParcel, MessageOption& option);

    bool mockEnabled_ = false;
    bool registerToService_ = false;
    ServiceRunningState state_ = ServiceRunningState::STATE_NOT_START;
    std::vector<std::shared_ptr<GeocodingMockInfo>> mockInfo_;
    std::mutex mockInfoMutex_;

    std::mutex mutex_;
    sptr<IRemoteObject> serviceProxy_ = nullptr;
    std::condition_variable connectCondition_;
};

class GeoServiceDeathRecipient : public IRemoteObject::DeathRecipient {
public:
    void OnRemoteDied(const wptr<IRemoteObject> &remote) override;
    GeoServiceDeathRecipient();
    ~GeoServiceDeathRecipient() override;
};
} // namespace OHOS
} // namespace Location
#endif // FEATURE_GEOCODE_SUPPORT
#endif // GEO_CONVERT_SERVICE_H
