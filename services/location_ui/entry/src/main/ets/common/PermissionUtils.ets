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

import Want from '@ohos.app.ability.Want';
import bundleManager from '@ohos.bundle.bundleManager';
import type { BusinessError } from '@ohos.base';

const TAG: string = 'PermissionUtils: ';

export class PermissionUtils {
    public readonly WANT_PARA_KEY_CALLER_BUNDLE_NAME: string = 'ohos.aafwk.param.callerBundleName';
    public readonly WANT_PARA_KEY_CALLER_TOKEN: string = 'ohos.aafwk.param.callerToken';
    public readonly WANT_PARA_KEY_CALLER_UID: string = 'ohos.aafwk.param.callerUid';
    public readonly BLUETOOTH_SERVICE_UID: number = 1002;
    public readonly SCENEBOARD_BUNDLE_NAME: string = 'com.ohos.sceneboard';

    /**
    * 校验蓝牙分享普通ability调用方权限：蓝牙服务允许作为调用方拉起
    */
    checkBluetoothShareCallerBluetoothPermission(want: Want): boolean {
        const callerBundleName = want?.parameters?.[this.WANT_PARA_KEY_CALLER_BUNDLE_NAME] as string;
        const callerUid = want?.parameters?.[this.WANT_PARA_KEY_CALLER_UID] as number;
        console.info(TAG, `checkBluetoothPermission callerUid ` + callerUid + ` BundleName ` + callerBundleName);

        let ret: boolean = false;
        if (!callerBundleName) {
            // 蓝牙服务为系统服务无bundle name, 校验uid是否为蓝牙.
            return callerUid == this.BLUETOOTH_SERVICE_UID;
        }
        return ret;
    }

    /**
    * 校验蓝牙分享UI ability调用方权限：大桌面应用允许作为调用方拉起
    */
    checkBluetoothShareCallerSceneboardPermission(want: Want): boolean {
        const callerBundleName = want?.parameters?.[this.WANT_PARA_KEY_CALLER_BUNDLE_NAME] as string;
        const callerUid = want?.parameters?.[this.WANT_PARA_KEY_CALLER_UID] as number;
        console.info(TAG, `checkSceneboardPermission callerUid ` + callerUid + ` BundleName ` + callerBundleName);

        let ret: boolean = false;
        if (!callerBundleName) {
            console.info(TAG, `without bundleName sceneboard app check fail.`);
            return ret;
        }

        let systemCallerAppInfo: bundleManager.ApplicationInfo;
        try {
            systemCallerAppInfo = bundleManager.getApplicationInfoSync(callerBundleName,
                bundleManager.ApplicationFlag.GET_APPLICATION_INFO_DEFAULT);
        } catch (error) {
            console.info(TAG, `Get app info error by ${callerBundleName}, cause: ${(error as BusinessError).message}`);
            return ret;
        }

        if (!systemCallerAppInfo) {
            console.info(TAG, `Get app info error by ${callerBundleName}, cause: systemCallerAppInfo is undefined`);
            return ret;
        }
        const callerAccessTokenId = want?.parameters?.[this.WANT_PARA_KEY_CALLER_TOKEN] as number;

        //系统应用sceneboard可以调用.
        ret = callerBundleName == this.SCENEBOARD_BUNDLE_NAME && systemCallerAppInfo.systemApp &&
            systemCallerAppInfo.accessTokenId == callerAccessTokenId && systemCallerAppInfo.uid == callerUid;
        if (!ret) {
            console.info(TAG, `sceneboard app check fail.`);
        }
        return ret;
    }
}

let mPermissionUtils = new PermissionUtils();

export default mPermissionUtils as PermissionUtils;
