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

#include <mutex>
#include <singleton.h>
#include "geo_convert_skeleton.h"
#include "if_system_ability_manager.h"
#include "system_ability.h"
#include "common_utils.h"

namespace OHOS {
namespace Location {
class GeoConvertService : public SystemAbility, public GeoConvertServiceStub, DelayedSingleton<GeoConvertService> {
DECLEAR_SYSTEM_ABILITY(GeoConvertService);

public:
    DISALLOW_COPY_AND_MOVE(GeoConvertService);
    GeoConvertService();
    ~GeoConvertService();
    void OnStart() override;
    void OnStop() override;
    ServiceRunningState QueryServiceState() const
    {
        return state_;
    }
    int IsGeoConvertAvailable(MessageParcel &data, MessageParcel &rep) override;
    int GetAddressByCoordinate(MessageParcel &data, MessageParcel &rep) override;
    int GetAddressByLocationName(MessageParcel &data, MessageParcel &rep) override;
    int32_t Dump(int32_t fd, const std::vector<std::u16string>& args) override;
private:
    static void SaDumpInfo(std::string& result);
    int RemoteToService(uint32_t code, MessageParcel &data, MessageParcel &rep);
    bool Init();

    bool registerToService_ = false;
    ServiceRunningState state_ = ServiceRunningState::STATE_NOT_START;
    sptr<IRemoteObject> locationService_;
};
} // namespace OHOS
} // namespace Location
#endif // GEO_CONVERT_SERVICE_H
