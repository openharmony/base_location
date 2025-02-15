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

import UIExtensionAbility from '@ohos.app.ability.UIExtensionAbility';
import UIExtensionContentSession from '@ohos.app.ability.UIExtensionContentSession';
import Want from '@ohos.app.ability.Want';
import type { BusinessError } from '@ohos.base';

const TAG: string = '[BT_RECEIVE_UI]==>'

export default class BluetoothReceiveServiceUIAbility extends UIExtensionAbility {
    onSessionCreate(want: Want, session: UIExtensionContentSession) {
        if (want.parameters == undefined) {
            return;
        }

        let fileUri = want.parameters?.['fileUri'];
        if (fileUri != undefined && fileUri != '') {
            console.info(TAG, 'pullUpFileManager invoked.');
            let want: Want = {
                bundleName: 'com.huawei.hmos.files',
                abilityName: 'EntryAbility',
                parameters: {
                    'fileUri': fileUri
                }
            };
            this.context.startAbility(want)
            .then(() => {
                console.info(TAG, 'pullUpFileManager successfully and terminateSelf');
                this.context.terminateSelf();
            })
            .catch((err: BusinessError) => {
                console.warn(TAG, 'pullUpFileManager failed');
                want.bundleName = 'com.huawei.hmos.filemanager';
                want.abilityName = 'MainAbility';
                this.context.startAbility(want);
            });
            console.info(TAG, 'pullUpFileManager end.');
            return;
        }

        let param: Record<string, UIExtensionContentSession> = {
            'session': session
        }
        let storage: LocalStorage = new LocalStorage(param);
        session.loadContent('pages/BluetoothReceive', storage);
        session.setWindowBackgroundColor('#00000000');
        AppStorage.setOrCreate('ConfirmSession', session);
    }

    onSessionDestroy(session: UIExtensionContentSession) {
        console.info(TAG, 'BluetoothReceiveServiceUIAbility onSessionDestroy');
    }
}