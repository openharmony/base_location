/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "locatorhandler_fuzzer.h"

#include "event_handler.h"
#include "accesstoken_kit.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "message_option.h"
#include "message_parcel.h"
#include "nativetoken_kit.h"
#include "system_ability_definition.h"
#include "token_setproc.h"
#define private public
#include "locator_ability.h"
#undef private

#include "i_locator_callback.h"

namespace OHOS {
using namespace OHOS::Location;
const int32_t MAX_MEM_SIZE = 4 * 1024 * 1024;
const int32_t CODE_MAX = 50;
const int32_t CODE_MIN = 1;
const int32_t MIN_SIZE_NUM = 4;

uint32_t GetU32Data(const char* ptr)
{
    return (ptr[0] << 24) | (ptr[1] << 16) | (ptr[2] << 8) | (ptr[3]);
}

char* ParseData(const uint8_t* data, size_t size)
{
    if (data == nullptr) {
        return nullptr;
    }

    if (size > MAX_MEM_SIZE) {
        return nullptr;
    }

    char* ch = static_cast<char*>(malloc(size + 1));
    if (ch == nullptr) {
        return nullptr;
    }

    (void)memset_s(ch, size + 1, 0x00, size + 1);
    if (memcpy_s(ch, size, data, size) != EOK) {
        free(ch);
        ch = nullptr;
        return nullptr;
    }
    return ch;
}

bool LocatorHandlerFuzzTest(const char* data, size_t size)
{
    uint32_t code = GetU32Data(data) % (CODE_MAX - CODE_MIN + 1) + CODE_MIN;
    std::shared_ptr<AppExecFwk::EventRunner> runner;
    auto locatorHandler = new LocatorHandler(runner);
    AppExecFwk::InnerEvent::Pointer event = AppExecFwk::InnerEvent::Get(code, code);
    locatorHandler->ProcessEvent(event);
    locatorHandler->UpdateSaEvent(event);
    locatorHandler->InitRequestManagerEvent(event);
    locatorHandler->ApplyRequirementsEvent(event);
    locatorHandler->RetryRegisterActionEvent(event);
    locatorHandler->ReportLocationMessageEvent(event);
    locatorHandler->SendSwitchStateToHifenceEvent(event);
    locatorHandler->UnloadSaEvent(event);
    locatorHandler->StartLocatingEvent(event);
    locatorHandler->StopLocatingEvent(event);
    locatorHandler->GetCachedLocationSuccess(event);
    locatorHandler->GetCachedLocationFailed(event);
    locatorHandler->StartScanBluetoothDeviceEvent(event);
    locatorHandler->StopScanBluetoothDeviceEvent(event);
    locatorHandler->RegLocationErrorEvent(event);
    locatorHandler->UnRegLocationErrorEvent(event);
    locatorHandler->ReportNetworkLocatingErrorEvent(event);
    locatorHandler->RequestCheckEvent(event);
    locatorHandler->SyncStillMovementState(event);
    locatorHandler->SyncIdleState(event);
    locatorHandler->SendGeoRequestEvent(event);
    locatorHandler->SyncSwitchStatus(event);
    locatorHandler->InitMonitorManagerEvent(event);
    locatorHandler->IsStandByEvent(event);
    locatorHandler->SetLocationWorkingStateEvent(event);
    locatorHandler->SetSwitchStateToDbEvent(event);
    locatorHandler->SetSwitchStateToDbForUserEvent(event);
    locatorHandler->WatchSwitchParameter(event);
    return true;
}
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (size < OHOS::MIN_SIZE_NUM) {
        return 0;
    }
    char* ch = OHOS::ParseData(data, size);
    if (ch != nullptr) {
        OHOS::LocatorHandlerFuzzTest(ch, size);
        free(ch);
        ch = nullptr;
    }
    return 0;
}

