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

#ifndef COUNTRY_CODE_MANAGER_H
#define COUNTRY_CODE_MANAGER_H

#include <list>
#include <map>
#include <mutex>
#include <singleton.h>
#include <string>
#include "iremote_stub.h"
#include "nocopyable.h"
#include "gnss_ability_proxy.h"
#include "network_ability_proxy.h"
#include "passive_ability_proxy.h"
#include "request.h"
#include "work_record.h"

namespace OHOS {
namespace Location {
class CountryCodeManager : public DelayedSingleton<CountryCodeManager> {
public:
    CountryCodeManager();
    ~CountryCodeManager();
    int GetIsoCountryCode(std::string& code);
};
} // namespace Location
} // namespace OHOS
#endif // COUNTRY_CODE_MANAGER_H
