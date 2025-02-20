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

import UIExtensionContentSession from '@ohos.app.ability.UIExtensionContentSession';
import opp from '@ohos.bluetooth.opp';
import common from '@ohos.app.ability.common';
import fs from '@ohos.file.fs';

export const CLOSE = 1;

export const BACK = 0;

const TAG: string = 'BluetoothShare: ';
const BUNDLE_NAME: string = 'com.huawei.hmos.settings';
const ABILITY_NAME: string = 'BluetoothOppServiceUIExtensionAbility';

@Entry
@Component
export struct BtServicesComponent {
  uiExtensionProxy?: UIExtensionProxy;
  private serverMac: string = '';
  private fileHolders: Array<opp.FileHolder> = [];

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

  aboutToAppear(): void {
    console.log(TAG, 'aboutToAppear.');
  }

  aboutToDisappear(): void {
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

  private async sendData(): Promise<void> {
    console.log(`${TAG} sendData is called.`);
    let timeOut: number = 20;
    let fileUris = AppStorage.Get('sendFileUris');
    let length = AppStorage.Get('sendFileUrisLength');
    try {
      let oppProfile = opp.createOppServerProfile();
      for (let i = 0; i < length; i++) {
        let filePath = fileUris[i];
        console.log(`${TAG} deal uri:` + filePath);
        let fileHolder: opp.FileHolder = {
          filePath: filePath,
          fileSize: 0,
          fileFd: 0
        };
        this.fileHolders.push(fileHolder);
        console.log(`${TAG} fileHolder info: ` + JSON.stringify(fileHolder));
      }
      oppProfile.sendFile(this.serverMac, this.fileHolders);
      setTimeout(() => {
        const session: UIExtensionContentSession | undefined =
          LocalStorage.getShared()?.get<UIExtensionContentSession>('session');
        if (session === undefined) {
          console.log(TAG, 'cannot get session.');
          return;
        }
        this.closeSheetPage(session, CLOSE);
      }, timeOut);
    } catch (err) {
      console.error(`${TAG} sendData err`);
    }
  }
}