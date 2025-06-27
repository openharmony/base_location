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

import ShareExtensionAbility from '@ohos.app.ability.ShareExtensionAbility';
import UIExtensionContentSession from '@ohos.app.ability.UIExtensionContentSession';
import Want from '@ohos.app.ability.Want';
import fs from '@ohos.file.fs';

interface UIExtensionStorage {
    session: UIExtensionContentSession,
}

const TAG: string = 'BluetoothServiceAbility: ';

export default class BluetoothServiceAbility extends ShareExtensionAbility {
    private uris: Array<string> = [];
    onCreate(): void {
        console.log(`${TAG} onCreate is called`);
    }

    onDestroy(): void {
        console.log(`${TAG} onDestroy is called`);
    }

    public async onSessionCreate(want: Want, session: UIExtensionContentSession): Promise<void> {
        console.log(`${TAG} onSessionCreate is called`);
        let systemShareModule = loadNativeModule('@hms:collaboration.systemShare') as ESObject;
        try {
            let data = await systemShareModule.getSharedData(want);
            let records = data.getRecords();
            for (let i = 0; i < records.length; i++) {
                let filePath = '';
                if (records[i].uri == undefined && records[i].content != undefined) {
                    console.log(`${TAG} share txt content`);
                    filePath = this.context.filesDir + '/' + (i + 1) + '.txt';
                    let file = fs.openSync(filePath, fs.OpenMode.READ_WRITE | fs.OpenMode.CREATE);
                    let writeLen = fs.writeSync(file.fd, records[i].content);
                    console.log(`${TAG} write data to file success and size is:` + writeLen);
                    fs.closeSync(file);
                } else if (records[i].uri == undefined) { 
                    console.log(`${TAG} records[i].uri is undefined`);
                } else {
                    filePath = records[i].uri;
                }
                this.uris.push(filePath);
            }
            AppStorage.setOrCreate('sendFileUris', this.uris);
            AppStorage.setOrCreate('sendFileUrisLength', this.uris.length);
        } catch (err) {
            console.log(`${TAG} records fail`);
        }
        let localStorage: LocalStorage = new LocalStorage({
            session: session
        } as UIExtensionStorage);
        try {
            session.loadContent('pages/BluetoothShare', localStorage);
            session.setWindowBackgroundColor('#00000000');
        } catch (err) {
            console.log(`${TAG} loadContent fail`);
        }
    }

    onSessionDestroy(): void {
        console.log(`${TAG} onSessionDestroy is called`);
    }

    onForeground(): void {
        console.log(`${TAG} onForeground is called`);
    }

    onBackground(): void {
        console.log(`${TAG} onBackground is called`);
    }
}