/*
 * Copyright (c) 2024-2024 Huawei Device Co., Ltd.
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

import UIExtensionAbility from '@ohos.app.ability.UIExtensionAbility'
import UIExtensionContentSession from '@ohos.app.ability.UIExtensionContentSession'
import Want from '@ohos.app.ability.Want';

const TAG: string = '[WIFI-NI]==>'

export default class WifiUIExtAbility extends UIExtensionAbility {
    onSessionCreate(want: Want, session: UIExtensionContentSession) {
        console.info(TAG, `WifiUIExtAbility onSessionCreate, want: ${JSON.stringify(want)}`)
        if (want.parameters == undefined) {
            return;
        }

        console.info(TAG, `wifiDialogType: ` + want.parameters?.['wifiDialogType']);
        AppStorage.setOrCreate('wifiDialogType', want.parameters?.['wifiDialogType']);
        AppStorage.setOrCreate('wifi5gSsid', want.parameters?.['wifi5gSsid']);
        AppStorage.setOrCreate('p2pDeviceName', want.parameters?.['p2pDeviceName']);

        let param: Record<string, UIExtensionContentSession> = {
            'session': session
        }
        let storage: LocalStorage = new LocalStorage(param);
        let type : number = AppStorage.get('wifiDialogType') as number
        if (type == 4) {
            session.loadContent('pages/P2pWscPbcDialog', storage);
            session.setWindowBackgroundColor('#cc000000');
        } else {
            session.loadContent('pages/WifiDialog', storage);
            session.setWindowBackgroundColor('#00000000');
        }
        AppStorage.setOrCreate('ConfirmSession', session);
    }

    onSessionDestroy(session: UIExtensionContentSession) {
        console.info(TAG, `WifiUIExtAbility onSessionDestroy`)
    }
}