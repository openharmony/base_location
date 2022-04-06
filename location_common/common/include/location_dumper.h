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

#ifndef OHOS_LOCATION_DUMPER_H
#define OHOS_LOCATION_DUMPER_H

#include <vector>

namespace OHOS {
namespace Location {
class LocationDumper {
public:
    bool GeocodeDump(std::function<void(std::string&)> saBasicDumpFunc,
        const std::vector<std::string> &vecArgs, std::string &result);

    bool GnssDump(std::function<void(std::string&)> saBasicDumpFunc,
        const std::vector<std::string> &vecArgs, std::string &result);

    bool LocatorDump(std::function<void(std::string&)> saBasicDumpFunc,
        const std::vector<std::string> &vecArgs, std::string &result);

    bool NetWorkDump(std::function<void(std::string&)> saBasicDumpFunc,
        const std::vector<std::string> &vecArgs, std::string &result);

    bool PassiveDump(std::function<void(std::string&)> saBasicDumpFunc,
        const std::vector<std::string> &vecArgs, std::string &result);
private:
    void PrintArgs(const std::vector<std::string>& vecArgs);
};
}  // namespace Location
}  // namespace OHOS
#endif