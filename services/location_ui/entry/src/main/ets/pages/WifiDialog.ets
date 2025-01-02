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

import UIExtensionContentSession from '@ohos.app.ability.UIExtensionContentSession';
import commonEventManager from '@ohos.commonEventManager';
import resourceManager from '@ohos.resourceManager';

let TAG = '[WIFI-NI:WifiDialog]==>';
const WIFI_NI_ACCEPT_EVENT: string = 'ohos.event.wifi.DIALOG_ACCEPT';
const WIFI_NI_REJECT_EVENT: string = 'ohos.event.wifi.DIALOG_REJECT';

@Entry
@Component
struct dialogPlusPage {
  @State title: string = '';
  @State primaryButtonValue: string = '';
  @State secondaryButtonValue: string = '';
  private WIFI_DIALOG_CDD: number = 0;
  private WIFI_DIALOG_THREE_VAP: number = 1;
  private WIFI_DIALOG_CANDIDATE_CONNECT: number = 2;
  private WIFI_DIALOG_5G_AUTO_IDENTIFY_CONN: number = 3;

  aboutToAppear() {
    console.info(TAG, 'aboutToAppear')
    if (AppStorage.get('wifiDialogType') != null) {
      let type : number = AppStorage.get('wifiDialogType') as number
      switch (type) {
        case this.WIFI_DIALOG_CDD:
            this.title = getContext().resourceManager.getStringSync($r('app.string.wifi_cdd_notify_title').id);
            this.primaryButtonValue = getContext().resourceManager.getStringSync($r('app.string.wifi_cdd_notify_no_button').id);
            this.secondaryButtonValue = getContext().resourceManager.getStringSync($r('app.string.wifi_cdd_notify_yes_button').id);
            break;
        case this.WIFI_DIALOG_THREE_VAP:
            this.title = getContext().resourceManager.getStringSync($r('app.string.wifi_three_vap_notify_title').id);
            this.primaryButtonValue = getContext().resourceManager.getStringSync($r('app.string.wifi_three_vap_notify_no_button').id);
            this.secondaryButtonValue = getContext().resourceManager.getStringSync($r('app.string.wifi_three_vap_notify_yes_button').id);
            break;
        case this.WIFI_DIALOG_CANDIDATE_CONNECT:
           this.title = getContext().resourceManager.getStringSync($r('app.string.wifi_candidate_connect_notify_title').id);
            this.primaryButtonValue = getContext().resourceManager.getStringSync($r('app.string.wifi_candidate_connect_notify_no_button').id);
            this.secondaryButtonValue = getContext().resourceManager.getStringSync($r('app.string.wifi_candidate_connect_notify_yes_button').id);
            break;
        case this.WIFI_DIALOG_5G_AUTO_IDENTIFY_CONN:
            let ssid : string = AppStorage.get('wifi5gSsid') as string;
            this.title = this.getFormatString($r('app.string.wifipro_auto_connect_dialog_context'), ssid);
            this.primaryButtonValue = getContext().resourceManager.getStringSync($r('app.string.wifipro_auto_connect_dialog_no_button').id);
            this.secondaryButtonValue = getContext().resourceManager.getStringSync($r('app.string.wifipro_auto_connect_dialog_ok_button').id);
            break;
        default:
            break;
      }
      AlertDialog.show(
        {
          title: '',
          message: this.title,
          autoCancel: false,
          primaryButton: {
            value: this.primaryButtonValue,
            action: () => {
              this.onCancel(type);
            }
          },
          secondaryButton: {
            value: this.secondaryButtonValue,
            action: () => {
              this.onAccept(type);
            }
          }
        }
      )
      console.info(TAG, 'dialog show success')
    }
  }

  getFormatString(resource: Resource, subStr: string): string {
      let result = getContext().resourceManager.getStringSync(resource.id);
      return result.replace(new RegExp('%s', 'gm'), subStr);
  }
 
  aboutToDisappear() {
    let session = AppStorage.get<UIExtensionContentSession>('ConfirmSession');
    if (session) {
      session.terminateSelf();
    }
  }

  onCancel(type :number) {
    console.info(TAG, 'Wifi dialog cancel clicked.')

    const options: commonEventManager.CommonEventPublishData = {
      code: 0,
      data: 'message',
      subscriberPermissions: [],
      isOrdered: true,
      isSticky: false,
      parameters: { 'dialogType': type }
    }

    commonEventManager.publish(WIFI_NI_REJECT_EVENT, options, (err) => {
      if (err) {
        console.info(TAG, 'Wifi dialog cancel event publish failed .' + JSON.stringify(err));
      } else {
        console.info(TAG, 'Wifi dialog cancel event publish success.');
      }
    })

    let session = AppStorage.get<UIExtensionContentSession>('ConfirmSession');
    if (session) {
      session.terminateSelf();
    }
  }

  onAccept(type : number) {
    console.info(TAG, 'Wifi dialog accept clicked.')

    const options: commonEventManager.CommonEventPublishData = {
      code: 0,
      data: 'message',
      subscriberPermissions: [],
      isOrdered: true,
      isSticky: false,
      parameters: { 'dialogType': type }
    }

    commonEventManager.publish(WIFI_NI_ACCEPT_EVENT, options, (err) => {
      if (err) {
        console.info(TAG, 'Wifi dialog accept event publish failed .' + JSON.stringify(err));
      } else {
        console.info(TAG, 'Wifi dialog accept event publish success.');
      }
    })

    let session = AppStorage.get<UIExtensionContentSession>('ConfirmSession');
    if (session) {
      session.terminateSelf();
    }
  }

  build() {
  }
}