/*
 * Copyright (c) 2025-2025 Huawei Device Co., Ltd.
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

import systemParameter from '@ohos.systemParameterEnhance';
import deviceInfo from '@ohos.deviceInfo';
import UIExtensionContentSession from '@ohos.app.ability.UIExtensionContentSession';
import opp from '@ohos.bluetooth.opp';
import common from '@ohos.app.ability.common';
import fs from '@ohos.file.fs';
import fileUri from '@ohos.file.fileuri';
import { BusinessError } from '@ohos.base';

export const CLOSE = 1;

export const BACK = 0;

const TAG: string = 'BluetoothShare: ';
const BUNDLE_NAME: string = 'com.huawei.hmos.settings';
const ABILITY_NAME: string = 'BluetoothOppServiceUIExtensionAbility';
const CONTROL_TYPE_ALLOW_SEND_RECEIVE: string = '1';
const CONTROL_TYPE_DISALLOW_SEND_ALLOW_RECEIVE: string = '2';
const MAX_FILE_NUM: number = 300;
const TRANSMIT_CONTROL_PROP_KEY: string = 'persist.distributed_scene.datafiles_trans_ctrl';

@Entry
@Component
export struct BtServicesComponent {
  uiExtensionProxy?: UIExtensionProxy;
  private serverMac: string = '';
  private fileHolders: Array<opp.FileHolder> = [];
  /**
   * 设备类型：PC
   */
  private static readonly DEVICE_TYPE_PC: string = 'pc';

  /**
   * 设备类型: 2in1
   */
  private static readonly DEVICE_TYPE_PC_NEW: string = '2in1';

  build() {
    Column() {
      UIExtensionComponent({
        bundleName: BUNDLE_NAME,
        abilityName: ABILITY_NAME,
        parameters: {
          'ability.want.params.uiExtensionType': 'sys/commonUI',
        }
      })
        .backgroundColor('#01111111')
        .onRemoteReady((proxy) => {
          console.log(TAG, 'onRemoteReady.');
          this.uiExtensionProxy = proxy;
        })
        .onError((error) => {
          console.log(TAG, `onError code: ${error?.code} message: ${error?.message}`);
        })
        .onTerminated(() => {
        })
        .onReceive((data: Record<string, string | Object>) => {
          console.log(TAG, 'onReceive: ' + data?.action);
          if (!data) {
            console.error(TAG, 'onReceive error');
            return;
          }
          this.dealReceivedData(data);
        })
        .size({ width: '100%', height: '100%'})
        .expandSafeArea([SafeAreaType.SYSTEM], [SafeAreaEdge.BOTTOM, SafeAreaEdge.TOP])
    }
  }

  private needDisableShare() : boolean {
    try {
      console.log(TAG, 'systemParameter get start');
      const info: string = systemParameter.getSync(TRANSMIT_CONTROL_PROP_KEY, CONTROL_TYPE_ALLOW_SEND_RECEIVE);
      return info === CONTROL_TYPE_DISALLOW_SEND_ALLOW_RECEIVE;
    } catch (err) {
      console.error(TAG, 'systemParameter get failed, msg: ${err.message}');
      return false;
    }
  }

  /**
   * 获取设备类型
   * @return string
   */
  private static getDeviceType() : string {
    return deviceInfo.deviceType;
  }

  private static isPC() : boolean {
    return BtServicesComponent.getDeviceType() === BtServicesComponent.DEVICE_TYPE_PC ||
      BtServicesComponent.getDeviceType() === BtServicesComponent.DEVICE_TYPE_PC_NEW;
  }

  /**
   * 关闭、退出蓝牙分享页面
   */
  private closeSheetPage(session: UIExtensionContentSession, code: number, message?: string) {
    if (!session) {
      console.log(TAG, `invalid session`);
      return;
    }
    console.log(TAG, `closeSheetPage start`);
    let result: common.AbilityResult = {
      resultCode: code
    };
    if (message) {
      result.want = {
        parameters: {
          message: message
        }
      }
    }
    try {
      session?.terminateSelfWithResult(result).then(() => {
        console.log(TAG, `terminateSelfWithResult success`);
      }).catch((error: Error) => {
        console.log(TAG, `terminateSelfWithResult failed, err name: ${error?.name}`);
      });
    } catch (err) {
      console.log(TAG, `closeSheetPage error, msg: ${err.message}`);
    }
  }

  aboutToAppear() : void {
    console.log(TAG, 'aboutToAppear.');
    if (BtServicesComponent.isPC() && this.needDisableShare()) {
      console.log(TAG, 'is PC IT device');
      const session: UIExtensionContentSession | undefined =
        LocalStorage.getShared()?.get<UIExtensionContentSession>('session');
      if (session === undefined) {
        console.log(TAG, 'cannot get session');
        return;
      }
      console.log(TAG, 'closeSheetPage');
      this.closeSheetPage(session, CLOSE);
    }
  }

  aboutToDisappear() : void {
    console.log(TAG, 'aboutToDisappear.');
  }

  private async dealReceivedData(data: Record<string, Object>): Promise<void> {
    const session: UIExtensionContentSession | undefined = LocalStorage.getShared()?.get<UIExtensionContentSession>('session');
    if (session === undefined) {
      console.log(TAG, 'cannot get session.');
      return;
    }
    if (data['action'] === 'sendDeviceInfo') {
      console.log(TAG, 'sendDeviceInfo message.');
      let serverMac: string = data['deviceInfo'] as string;
      this.serverMac = serverMac;
      this.sendData();
    }
    if (data['action'] === 'closeSheet') {
      console.log(TAG, 'closeSheet message.');
      this.closeSheetPage(session, CLOSE);
    }
    if (data['action'] === 'backSheet') {
      console.log(TAG, 'backSheet message.');
      this.closeSheetPage(session, BACK);
    }
  }

  private dealSysFileUris(sysShareFileUris: string[], sysShareFilelength: number) : string[] {
    let result: string[] = [];
    for (let i = 0; i < sysShareFilelength; i++) {
      let sandboxUri: string = new fileUri.FileUri(sysShareFileUris[i]).path;
      let isDirectory = fs.statSync(sandboxUri).isDirectory();
      if (isDirectory) {
        console.log(TAG, 'share file directory.');
        let directoryFiles: string[] = fs.listFileSync(sandboxUri);
        console.log(TAG, 'directoryFiles length is ' + directoryFiles.length);
        let uris: string[] = [];
        for (let j = 0; j < directoryFiles.length; j++) {
          uris.push(sandboxUri + '/' + directoryFiles[j]);
        }
        result.push(...this.dealSysFileUris(uris, directoryFiles.length));
      } else {
        result.push(sysShareFileUris[i]);
      }
    }
    return result;
  }

  private async sendData() : Promise<void> {
    console.log(`${TAG} sendData is called.`);
    let sysShareFileUris: string[] = AppStorage.Get('sendFileUris');
    let sysShareFilelength: number = AppStorage.Get('sendFileUrisLength');
    if ((sysShareFileUris === undefined) || (sysShareFileUris === null) ||
        (sysShareFilelength === undefined) || (sysShareFilelength === null)) {
      console.log(TAG, 'sysShareFileUris is undefined');
      return;
    }
    try {
      let oppProfile = opp.createOppServerProfile();
      let fileUris: string[] = this.dealSysFileUris(sysShareFileUris, sysShareFilelength);
      let fileUrisLength: number = (fileUris.length > MAX_FILE_NUM) ? MAX_FILE_NUM : fileUris.length;
      for (let i = 0; i < fileUrisLength; i++) {
        let file = fs.openSync(fileUris[i], fs.OpenMode.READ_ONLY);
        let filePath = decodeURIComponent(fileUris[i]);
        console.log(`${TAG} deal file:` + file.fd);
        let stat: fs.Stat = fs.statSync(file.fd);
        let fileHolder: opp.FileHolder = {
          filePath: filePath,
          fileSize: stat.size,
          fileFd: file.fd
        };
        this.fileHolders.push(fileHolder);
      }
      if (this.fileHolders.length <= 0) {
        console.log(TAG, 'fileHolders is undefined');
        return;
      }
      await oppProfile.sendFile(this.serverMac, this.fileHolders);
      const session: UIExtensionContentSession | undefined =
        LocalStorage.getShared()?.get<UIExtensionContentSession>('session');
      for (let i = 0; i < this.fileHolders.length; i++) {
        console.log(TAG, 'close fileHolders fd.');
        if (this.fileHolders[i].fileFd != -1) {
          fs.close(this.fileHolders[i].fileFd);
          this.fileHolders[i].fileFd = -1;
        }
      }
    } catch (err) {
      console.error(`${TAG} sendData err, code is ${err.code}, message is ${err.message}`);
    }
    const session: UIExtensionContentSession | undefined =
      LocalStorage.getShared()?.get<UIExtensionContentSession>('session');
    if (session == undefined) {
      console.log(TAG, 'cannot get session.');
      return;
    }
    this.closeSheetPage(session, CLOSE);
  }
}