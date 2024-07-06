/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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

#ifndef LBS_RES_LOADER_H
#define LBS_RES_LOADER_H

#include "location_log.h"

#include "i_resources.h"
#include "dynamic_library_loader.h"

namespace OHOS {
namespace Location {
class LbsResLoader {
public:
    const std::string RES_DLL_NAME = "liblbsresources.z.so";

    LbsResLoader();
    ~LbsResLoader();

    std::string GetSystemRegion();

private:
    DynamicLibraryLoader<IResources> loader_;
    IResources *(*entry_)();
};
} // namespace Location
} // namespace OHOS
#endif // LBS_RES_LOADER_H