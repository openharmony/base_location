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

#ifndef LBS_DYNAMIC_LOADER_H
#define LBS_DYNAMIC_LOADER_H

#include <dlfcn.h>

#include "location_log.h"

namespace OHOS {
namespace Location {
template<typename FuncType>
class DynamicLibraryLoader {
    typedef FuncType* (*funcEntry)();
public:
    explicit DynamicLibraryLoader(const std::string& libPath)
    {
        libPath_ = libPath;
        handle_ = dlopen(libPath.c_str(), RTLD_LAZY);
        if (!handle_) {
            LBSLOGE(COMMON_UTILS, "%{public}s open so [%{public}s] failed, error: %{public}s",
                __func__, libPath.c_str(), dlerror());
        }
        LBSLOGI(COMMON_UTILS, "%{public}s open so [%{public}s] success.", __func__, libPath.c_str());
    }

    ~DynamicLibraryLoader()
    {
        if (handle_) {
            LBSLOGI(COMMON_UTILS, "%{public}s close so [%{public}s]", __func__, libPath_.c_str());
#ifndef TEST_CASES_ENABLED
            dlclose(handle_);
            handle_ = nullptr;
#endif
        }
    }

    funcEntry GetFunction(const std::string& funcName)
    {
        if (!handle_) {
            return nullptr;
        }
        auto funcPtr = dlsym(handle_, funcName.c_str());
        char* error = dlerror();
        if (error != nullptr || funcPtr == nullptr) {
            LBSLOGE(COMMON_UTILS, "%{public}s, [%{public}s] dlsym error, error: %{public}s",
                __func__, libPath_.c_str(), error);
            return nullptr;
        }
        return (funcEntry)(funcPtr);
    }

private:
    std::string libPath_;
    void* handle_ = nullptr;
};
} // namespace Location
} // namespace OHOS
#endif // LBS_DYNAMIC_LOADER_H