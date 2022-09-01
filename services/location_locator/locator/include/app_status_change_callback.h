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

#ifdef APP_STATUS_CHANGE_CALLBACK_H
#define APP_STATUS_CHANGE_CALLBACK_H

#include "application_status_observer_stub.h"
#include "app_mgr_proxy.h"

namespace OHOS {
namespace Location {
class AppStatusChangeCallback :: public AppExecFwk::ApplicationStateObserverStub {
public:
    AppStatusChangeCallback();
    virtual ~AppStatusChangeCallback();

    void OnForegroundApplicationChanged(const AppExecFwk::AppStateData& appStateData) override;
}
} // namespace Location
} // namespace OHOS
#endif // APP_STATUS_CHANGE_CALLBACK_H