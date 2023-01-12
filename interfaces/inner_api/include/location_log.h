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

#ifndef LOCATION_LOG_H
#define LOCATION_LOG_H

#include "hilog/log.h"

namespace OHOS {
namespace Location {
enum {
    LOCATOR_LOG_ID = 0xD002300,
};

static constexpr OHOS::HiviewDFX::HiLogLabel GEO_CONVERT = {LOG_CORE, LOCATOR_LOG_ID, "GeoConvert"};
static constexpr OHOS::HiviewDFX::HiLogLabel LOCATOR = {LOG_CORE, LOCATOR_LOG_ID, "Locator"};
static constexpr OHOS::HiviewDFX::HiLogLabel LOCATOR_STANDARD = {LOG_CORE, LOCATOR_LOG_ID, "Locator_standard"};
static constexpr OHOS::HiviewDFX::HiLogLabel SWITCH_CALLBACK = {LOG_CORE, LOCATOR_LOG_ID, "SwitchCallback"};
static constexpr OHOS::HiviewDFX::HiLogLabel GNSS_STATUS_CALLBACK = {LOG_CORE, LOCATOR_LOG_ID, "GnssStatusCallback"};
static constexpr OHOS::HiviewDFX::HiLogLabel NMEA_MESSAGE_CALLBACK = {LOG_CORE, LOCATOR_LOG_ID, "NmeaMessageCallback"};
static constexpr OHOS::HiviewDFX::HiLogLabel CACHED_LOCATIONS_CALLBACK = {
    LOG_CORE, LOCATOR_LOG_ID, "CachedLocationsCallback"
};
static constexpr OHOS::HiviewDFX::HiLogLabel COUNTRY_CODE_CALLBACK = {
    LOG_CORE, LOCATOR_LOG_ID, "CountryCodeCallback"
};
static constexpr OHOS::HiviewDFX::HiLogLabel LOCATOR_CALLBACK = {LOG_CORE, LOCATOR_LOG_ID, "LocatorCallback"};
static constexpr OHOS::HiviewDFX::HiLogLabel GNSS = {LOG_CORE, LOCATOR_LOG_ID, "GnssAbility"};
static constexpr OHOS::HiviewDFX::HiLogLabel GNSS_TEST = {LOG_CORE, LOCATOR_LOG_ID, "GnssAbilityTest"};
static constexpr OHOS::HiviewDFX::HiLogLabel NETWORK = {LOG_CORE, LOCATOR_LOG_ID, "NetworkAbility"};
static constexpr OHOS::HiviewDFX::HiLogLabel NETWORK_TEST = {LOG_CORE, LOCATOR_LOG_ID, "NetworkAbilityTest"};
static constexpr OHOS::HiviewDFX::HiLogLabel PASSIVE = {LOG_CORE, LOCATOR_LOG_ID, "PassiveAbility"};
static constexpr OHOS::HiviewDFX::HiLogLabel PASSIVE_TEST = {LOG_CORE, LOCATOR_LOG_ID, "PassiveAbilityTest"};
static constexpr OHOS::HiviewDFX::HiLogLabel REQUEST_MANAGER = {LOG_CORE, LOCATOR_LOG_ID, "RequestManager"};
static constexpr OHOS::HiviewDFX::HiLogLabel REPORT_MANAGER = {LOG_CORE, LOCATOR_LOG_ID, "ReportManager"};
static constexpr OHOS::HiviewDFX::HiLogLabel COMMON_UTILS = {LOG_CORE, LOCATOR_LOG_ID, "CommonUtils"};
static constexpr OHOS::HiviewDFX::HiLogLabel LOCATOR_EVENT = {LOG_CORE, LOCATOR_LOG_ID, "LocatorEvent"};
static constexpr OHOS::HiviewDFX::HiLogLabel COUNTRY_CODE = {LOG_CORE, LOCATOR_LOG_ID, "CountryCode"};
static constexpr OHOS::HiviewDFX::HiLogLabel VISIBILITY_CONTROLLER = {
    LOG_CORE, LOCATOR_LOG_ID, "VisibilityController"
};
static constexpr OHOS::HiviewDFX::HiLogLabel FUSION_CONTROLLER = {LOG_CORE, LOCATOR_LOG_ID, "FusionController"};
static constexpr OHOS::HiviewDFX::HiLogLabel DISTRIBUTION_CONTROLLER = {
    LOG_CORE, LOCATOR_LOG_ID, "Location_Distribution"
};
static constexpr OHOS::HiviewDFX::HiLogLabel HIVIEW_EVENT = {LOG_CORE, LOCATOR_LOG_ID, "ChrEvent"};
static constexpr OHOS::HiviewDFX::HiLogLabel LOCATOR_BACKGROUND_PROXY = {
    LOG_CORE, LOCATOR_LOG_ID, "LocatorBackgroundProxy"
};
static constexpr OHOS::HiviewDFX::HiLogLabel LOCATION_NAPI = {LOG_CORE, LOCATOR_LOG_ID, "LocationNapi"};

#ifndef LBSLOGD
#define LBSLOGD(label, ...) (void)OHOS::HiviewDFX::HiLog::Debug(label, __VA_ARGS__)
#endif

#ifndef LBSLOGE
#define LBSLOGE(label, ...) (void)OHOS::HiviewDFX::HiLog::Error(label, __VA_ARGS__)
#endif

#ifndef LBSLOGF
#define LBSLOGF(label, ...) (void)OHOS::HiviewDFX::HiLog::Fatal(label, __VA_ARGS__)
#endif

#ifndef LBSLOGI
#define LBSLOGI(label, ...) (void)OHOS::HiviewDFX::HiLog::Info(label, __VA_ARGS__)
#endif

#ifndef LBSLOGW
#define LBSLOGW(label, ...) (void)OHOS::HiviewDFX::HiLog::Warn(label, __VA_ARGS__)
#endif
} // namespace Location
} // namespace OHOS
#endif // LOCATION_LOG_H
