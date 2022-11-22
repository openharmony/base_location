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

#ifndef I_LOCATOR_H
#define I_LOCATOR_H

#include "iremote_broker.h"

namespace OHOS {
namespace Location {
class ILocator : public IRemoteBroker {
public:
    enum {
        GET_SWITCH_STATE = 1,
        REG_SWITCH_CALLBACK = 2,
        START_LOCATING = 3,
        STOP_LOCATING = 4,
        GET_CACHE_LOCATION = 5,
        ENABLE_ABILITY = 9,
        UPDATE_SA_ABILITY = 10,
        GEO_IS_AVAILABLE = 11,
        GET_FROM_COORDINATE = 12,
        GET_FROM_LOCATION_NAME = 13,
        UNREG_SWITCH_CALLBACK = 15,
        REG_GNSS_STATUS_CALLBACK = 16,
        UNREG_GNSS_STATUS_CALLBACK = 17,
        REG_NMEA_CALLBACK = 18,
        UNREG_NMEA_CALLBACK = 19,
        IS_PRIVACY_COMFIRMED = 20,
        SET_PRIVACY_COMFIRM_STATUS = 21,
        REG_CACHED_CALLBACK = 22,
        UNREG_CACHED_CALLBACK = 23,
        GET_CACHED_LOCATION_SIZE = 24,
        FLUSH_CACHED_LOCATIONS = 25,
        SEND_COMMAND = 26,
        ADD_FENCE = 27,
        REMOVE_FENCE = 28,
        GET_ISO_COUNTRY_CODE = 29,
        ENABLE_LOCATION_MOCK = 30,
        DISABLE_LOCATION_MOCK = 31,
        SET_MOCKED_LOCATIONS = 32,
        ENABLE_REVERSE_GEOCODE_MOCK = 33,
        DISABLE_REVERSE_GEOCODE_MOCK = 34,
        SET_REVERSE_GEOCODE_MOCKINFO = 35,
        REG_COUNTRY_CODE_CALLBACK = 36,
        UNREG_COUNTRY_CODE_CALLBACK = 37,
        PROXY_UID_FOR_FREEZE = 38,
        RESET_ALL_PROXY = 39,
        REG_NMEA_CALLBACK_v9 = 40,
        UNREG_NMEA_CALLBACK_v9 = 41,
    };
    DECLARE_INTERFACE_DESCRIPTOR(u"location.ILocator");
};
} // namespace Location
} // namespace OHOS
#endif // I_LOCATOR_H
