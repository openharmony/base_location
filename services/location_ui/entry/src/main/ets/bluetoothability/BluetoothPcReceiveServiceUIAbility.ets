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
import PermissionUtils from 'common/PermissionUtils';

const TAG: string = '[BT_PC_RECEIVE_UI]==>'

export default class BluetoothReceiveServiceUIAbility extends UIExtensionAbility {
    onSessionCreate(want: Want, session: UIExtensionContentSession) {
        if (!PermissionUtils.checkBluetoothShareCallerSceneboardPermission(want)) {
            console.info(TAG, `caller not has permission`);
            return;
        }
        if (want.parameters == undefined) {
            return;
        }

        let fileUri:string = want.parameters?.['fileUri'] as string;
        if (fileUri != undefined && fileUri != '') {
            this.startAbility(fileUri, want);
            return;
        }

        let fileName:string = want.parameters?.['fileName'] as string;
        if (fileName != undefined && fileName != '') {
            AppStorage.setOrCreate('fileName', fileName);
        }

        let param: Record<string, UIExtensionContentSession> = {
            'session': session
        }
        let storage: LocalStorage = new LocalStorage(param);
        session.loadContent('pages/BluetoothPcReceive', storage);
        session.setWindowBackgroundColor('#00000000');
        AppStorage.setOrCreate('ConfirmSession', session);
    }

    onSessionDestroy(session: UIExtensionContentSession) {
        console.info(TAG, `BluetoothReceiveServiceUIAbility onSessionDestroy`);
    }

    startAbility(fileUri: string, want: Want) {
        console.info(TAG, `startAbility begin.`);
        let bundleName : string = want.parameters?.['bundleName'] as string;
        let abilityName : string = want.parameters?.['abilityName'] as string;
        if (bundleName == undefined || abilityName == undefined) {
            console.error(TAG, `undefined params`);
            return;
        }
        this.startFileManagerAbility(fileUri, bundleName, abilityName);
        return;
    }

    startFileManagerAbility(fileUri: string, bundleName: string, abilityName: string) {
        console.info(TAG, `startFileManagerAbility bundleName ` + bundleName + ` abilityName ` + abilityName);
        let want: Want = {
            bundleName: bundleName,
            abilityName: abilityName,
            parameters: {
                'fileUri': fileUri
            }
        };
        this.context.startAbility(want).then(() => {
            console.info(TAG, `startFileManagerAbility successfully and terminateSelf`);
            this.context.terminateSelf();
        }).catch((err: BusinessError) => {
            console.error(TAG, `startFileManagerAbility failed, code is ${err.code}, message is ${err.message}`);
            this.context.terminateSelf();
        });
    }
}