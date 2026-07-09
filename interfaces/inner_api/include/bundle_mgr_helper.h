/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
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

#ifndef BUNDLE_MGR_HELPER
#define BUNDLE_MGR_HELPER

#include "location.h"
#include "app_identity.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "os_account_manager.h"
#include "os_account_info.h"
#include "bundle_mgr_client.h"
#include "bundle_mgr_interface.h"
#include "bundle_mgr_proxy.h"

namespace OHOS {
namespace Location {

class BundleMgrHelper {
public:
    static bool CheckAppInstalled(const std::string& bundleName);
    static bool GetCurrentUserId(int &userId);
    static bool GetBundleNameByUid(int32_t uid, std::string& bundleName);
    static bool GetAppInfo(const std::string& bundleName, AppExecFwk::ApplicationInfo& info);
    static bool CheckAppDebug(const std::string& bundleName);
private:
};

}
}
#endif