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

#include "location_dumper.h"

#include <functional>
#include <numeric>
#include <string>
#include <vector>

#include "location_log.h"

namespace OHOS {
namespace Location {
const std::string ARGS_HELP = "-h";

void LocationDumper::PrintArgs(const std::vector<std::string>& vecArgs)
{
    std::string strArgs;
    strArgs = std::accumulate(vecArgs.begin(), vecArgs.end(), strArgs,
    [vecArgs](std::string strArgs, const std::string &each) {
        return strArgs + each + "|";
    });
    LBSLOGI(COMMON_UTILS, "Dumper[%{public}zu] args: %{public}s", vecArgs.size(), strArgs.c_str());
}

bool LocationDumper::GeocodeDump(std::function<void(std::string&)> saBasicDumpFunc,
    const std::vector<std::string>& vecArgs, std::string& result)
{
    PrintArgs(vecArgs);
    result.clear();
    if (!vecArgs.empty() && vecArgs[0] == ARGS_HELP) {
        result.append("Geocode dump options:\n")
            .append("  [-h]\n")
            .append("  description of the cmd option:\n")
            .append("    -h: show help.\n");
        return true;
    }

    saBasicDumpFunc(result);
    return true;
}

bool LocationDumper::GnssDump(std::function<void(std::string&)> saBasicDumpFunc,
    const std::vector<std::string>& vecArgs, std::string& result)
{
    PrintArgs(vecArgs);
    result.clear();
    if (!vecArgs.empty() && vecArgs[0] == ARGS_HELP) {
        result.append("Gnss dump options:\n")
            .append("  [-h]\n")
            .append("  description of the cmd option:\n")
            .append("    -h: show help.\n");
        return true;
    }

    saBasicDumpFunc(result);
    return true;
}

bool LocationDumper::LocatorDump(std::function<void(std::string&)> saBasicDumpFunc,
    const std::vector<std::string>& vecArgs, std::string& result)
{
    PrintArgs(vecArgs);
    result.clear();
    if (!vecArgs.empty() && vecArgs[0] == ARGS_HELP) {
        result.append("Locator dump options:\n")
            .append("  [-h]\n")
            .append("  description of the cmd option:\n")
            .append("    -h: show help.\n");
        return true;
    }

    saBasicDumpFunc(result);
    return true;
}

bool LocationDumper::NetWorkDump(std::function<void(std::string&)> saBasicDumpFunc,
    const std::vector<std::string>& vecArgs, std::string& result)
{
    PrintArgs(vecArgs);
    result.clear();
    if (!vecArgs.empty() && vecArgs[0] == ARGS_HELP) {
        result.append("Network dump options:\n")
            .append("  [-h]\n")
            .append("  description of the cmd option:\n")
            .append("    -h: show help.\n");
        return true;
    }

    saBasicDumpFunc(result);
    return true;
}

bool LocationDumper::PassiveDump(std::function<void(std::string&)> saBasicDumpFunc,
    const std::vector<std::string>& vecArgs, std::string& result)
{
    PrintArgs(vecArgs);
    result.clear();
    if (!vecArgs.empty() && vecArgs[0] == ARGS_HELP) {
        result.append("Passive dump options:\n")
            .append("  [-h]\n")
            .append("  description of the cmd option:\n")
            .append("    -h: show help.\n");
        return true;
    }

    saBasicDumpFunc(result);
    return true;
}
}  // namespace Location
}  // namespace OHOS
