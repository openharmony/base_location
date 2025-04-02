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
import promptAction from '@ohos.promptAction'
import { BusinessError } from '@ohos.base';
import deviceInfo from '@ohos.deviceInfo';

const TAG = '[WIFI-NI:WifiDialog]==>';
const DEVICE_TYPE_TV = 'tv';

const WIFI_NI_ACCEPT_EVENT: string = 'ohos.event.wifi.DIALOG_ACCEPT';
const WIFI_NI_REJECT_EVENT: string = 'ohos.event.wifi.DIALOG_REJECT';
const DIALOG_TIMEOUT_SECONDS = 30 * 1000;

@Entry
@Component
struct dialogPlusPage {
  @State text: string = '';
  private dialogType: number;
  private customDialogComponentId: number = -1;
  private closeDialogTaskId: number = -1;
  private WIFI_DIALOG_P2P_INVITATION_RECEIVE: number = 4;

  @Builder
  buildDialog() {
    Column() {
      Text($r('app.string.p2p_wsc_pbc_dialog_title'))
        .fontColor($r('app.color.tv_dialog_font_color'))
        .fontSize($r('app.float.value_vp_20'))
        .lineHeight($r('app.float.value_vp_23'))
        .fontWeight(FontWeight.Medium)
        .textAlign(TextAlign.Center)
      Text(this.text)
        .margin({ top: $r('app.float.value_vp_24') })
        .fontColor($r('app.color.tv_dialog_font_color'))
        .fontSize($r('app.float.value_fp_16'))
        .fontWeight(FontWeight.Regular)
        .lineHeight($r('app.float.value_vp_19'))
        .textAlign(TextAlign.Center)
      Row({ space: $r('app.float.value_vp_24') }) {
        Button($r('app.string.p2p_wsc_pbc_dialog_no_button'))
          .backgroundColor($r('app.color.tv_dialog_button_background_color'))
          .layoutWeight(1)
          .fontColor($r('app.color.tv_dialog_font_color'))
          .fontSize($r('app.float.value_fp_18'))
          .fontWeight(FontWeight.Medium)
          .borderRadius($r('app.float.value_vp_20'))
          .onClick(() => {
            console.info(TAG, 'user click cancel');
            this.setAcceptResult(this.dialogType, false);
          })
        Button($r('app.string.p2p_wsc_pbc_dialog_ok_button'))
          .layoutWeight(1)
          .backgroundColor($r('app.color.tv_dialog_button_background_color'))
          .fontColor($r('app.color.tv_dialog_font_color'))
          .fontSize($r('app.float.value_fp_18'))
          .fontWeight(FontWeight.Medium)
          .borderRadius($r('app.float.value_vp_20'))
          .onClick(() => {
            console.info(TAG, 'user click trust');
            this.setAcceptResult(this.dialogType, true);
          })
      }.margin({ top: $r('app.float.value_vp_24') })
      .width('100%')
      .justifyContent(FlexAlign.SpaceBetween)
      .alignItems(VerticalAlign.Center)
    }
    .width('100%')
    .padding($r('app.float.value_vp_24'))
    .justifyContent(FlexAlign.Center)
    .alignItems(HorizontalAlign.Center)
    .backgroundColor($r('app.color.tv_dialog_background_color'))
  }

  private openDialog(): void {
    console.info(TAG, 'openDialog');
    promptAction.openCustomDialog({
      builder: () => this.buildDialog(),
      autoCancel: false,
      onWillDismiss: (dismissDialogAction: DismissDialogAction) => {
        console.info(TAG, `not allow dismiss dialog by reason ${dismissDialogAction.reason}`);
      }
    }).then((dialogId: number) => {
      console.info(TAG, 'dialog open success');
      this.customDialogComponentId = dialogId;
      this.closeDialogTaskId = setTimeout(() => {
          console.info(TAG, 'user not select and time out');
          this.setAcceptResult(this.dialogType, false);
      }, DIALOG_TIMEOUT_SECONDS);
    }).catch((err: BusinessError) => {
      console.info(TAG, `fail to open dialog, err=${JSON.stringify(err)}`);
      this.setAcceptResult(this.dialogType, false);
    })
  }

  aboutToAppear() {
    console.info(TAG, 'aboutToAppear');
    let deviceName : string = AppStorage.get('p2pDeviceName') as string;
    this.dialogType = AppStorage.get('wifiDialogType') as number;
    this.text = this.getFormatString($r('app.string.p2p_wsc_pbc_dialog_text'), deviceName);
    if (deviceInfo.deviceType == DEVICE_TYPE_TV) {
      this.openDialog();
    } else {
      AlertDialog.show(
        {
          title: $r('app.string.p2p_wsc_pbc_dialog_title'),
          message: this.text,
          autoCancel: false,
          primaryButton: {
            value: $r('app.string.p2p_wsc_pbc_dialog_no_button'),
            action: () => {
              this.setAcceptResult(this.dialogType, false);
            }
          },
          secondaryButton: {
            value: $r('app.string.p2p_wsc_pbc_dialog_ok_button'),
            action: () => {
              this.setAcceptResult(this.dialogType, true);
            }
          }
        }
      )
    }
  }

  aboutToDisappear() {
    let session = AppStorage.get<UIExtensionContentSession>('ConfirmSession');
    if (session) {
      session.terminateSelf();
    }
  }

  getFormatString(resource: Resource, subStr: string): string {
      let result = getContext().resourceManager.getStringSync(resource.id);
      return result.replace(new RegExp('%s', 'gm'), subStr);
  }

  private async setAcceptResult(dialogType : number, isAccept : boolean): Promise<void> {
    console.info(TAG, 'Dialog type ${dialogType} click ${isAccept}.');
    clearTimeout(this.closeDialogTaskId);
    try {
      promptAction.closeCustomDialog(this.customDialogComponentId);
    } catch (err) {
      console.info(TAG, `fail to close custom dialog, err:${JSON.stringify(err)}`);
    }
    const options: commonEventManager.CommonEventPublishData = {
      code: 0,
      data: 'message',
      subscriberPermissions: [],
      isOrdered: true,
      isSticky: false,
      parameters: { 'dialogType': dialogType }
    }

    if (isAccept) {
      commonEventManager.publish(WIFI_NI_ACCEPT_EVENT, options, (err) => {
        if (err) {
          console.info(TAG, 'Wifi dialog accept event publish failed .' + JSON.stringify(err));
        } else {
          console.info(TAG, 'Wifi dialog accept event publish success.');
        }
      })
    } else {
      commonEventManager.publish(WIFI_NI_REJECT_EVENT, options, (err) => {
        if (err) {
          console.info(TAG, 'Wifi dialog cancel event publish failed .' + JSON.stringify(err));
        } else {
          console.info(TAG, 'Wifi dialog cancel event publish success.');
        }
      })
    }

    let session = AppStorage.get<UIExtensionContentSession>('ConfirmSession');
    if (session) {
      session.terminateSelf();
    }
  }

  build() {
  }
}