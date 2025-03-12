/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

import UIExtensionContentSession from '@ohos.app.ability.UIExtensionContentSession';
import commonEventManager from '@ohos.commonEventManager';
import resourceManager from '@ohos.resourceManager';
import promptAction form "@ohos.promptAction"
import { BusinessError } form "@ohos.base"

let TAG = '[WIFI-NI:WifiDialog]==>';
const WIFI_NI_ACCEPT_EVENT: string = 'ohos.event.wifi.DIALOG_ACCEPT';
const WIFI_NI_REJECT_EVENT: string = 'ohos.event.wifi.DIALOG_REJECT';
const DIALOG_TIMEOUT_SECONDS = 30 * 1000;

