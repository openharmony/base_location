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
  private WIFI_DIALOG_P2P_WSC_PBC: number = 4;
  private WIFI_DIALOG_P2P_WSC_DISPLAY: number = 6;
  private WIFI_DIALOG_P2P_WSC_KEYPAD: number = 7;
 
  private deviceName : string = '';
  private pinCode : string = '';
  @State inputPinCode: string = '';

  @Builder
  buildPbcDialog() {
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

  @Builder
  buildDisplayDialog() {
    Column() {
      Text($r('app.string.p2p_wsc_display_invitation_sent_title'))
        .fontColor($r('app.color.tv_dialog_font_color'))
        .fontSize($r('app.float.value_vp_20'))
        .lineHeight($r('app.float.value_vp_23'))
        .fontWeight(FontWeight.Medium)
        .textAlign(TextAlign.Center)
      Text(getContext().resourceManager.getStringSync($r('app.string.p2p_wsc_display_to_message')) +
          this.deviceName)
        .margin({ top: $r('app.float.value_vp_24') })
        .fontColor($r('app.color.tv_dialog_font_color'))
        .fontSize($r('app.float.value_fp_16'))
        .fontWeight(FontWeight.Regular)
        .lineHeight($r('app.float.value_vp_19'))
        .textAlign(TextAlign.Center)
      Text(getContext().resourceManager.getStringSync($r('app.string.p2p_wsc_display_show_pin_message')) +
          this.pinCode)
        .margin({ top: $r('app.float.value_vp_18') })
        .fontColor($r('app.color.tv_dialog_font_color'))
        .fontSize($r('app.float.value_fp_16'))
        .fontWeight(FontWeight.Regular)
        .lineHeight($r('app.float.value_vp_19'))
        .textAlign(TextAlign.Center)
      Button($r('app.string.p2p_wsc_display_ok_button'))
        .margin({ top: $r('app.float.value_vp_18') })
        .backgroundColor($r('app.color.tv_dialog_button_background_color'))
        .fontColor($r('app.color.tv_dialog_font_color'))
        .fontSize($r('app.float.value_fp_18'))
        .fontWeight(FontWeight.Medium)
        .width(100)
        .borderRadius($r('app.float.value_vp_20'))
        .onClick(() => {
          console.info(TAG, 'user click ok');
          this.setAcceptResult(this.dialogType, true);
        })
    }
    .width('100%')
    .padding($r('app.float.value_vp_24'))
    .justifyContent(FlexAlign.Center)
    .alignItems(HorizontalAlign.Center)
    .backgroundColor($r('app.color.tv_dialog_background_color'))
  }
 
  @Builder
  buildKeypadDialog() {
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
      Text($r('app.string.p2p_wsc_keypad_enter_pin_message'))
        .margin({ top: $r('app.float.value_vp_24') })
        .fontColor($r('app.color.tv_dialog_font_color'))
        .fontSize($r('app.float.value_fp_16'))
        .fontWeight(FontWeight.Regular)
        .lineHeight($r('app.float.value_vp_19'))
        .textAlign(TextAlign.Center)
      TextInput({ text: this.inputPinCode })
        .showUnderline(true)
        .maxLength(8)
        .onChange((value: string) => {
          this.inputPinCode = value;
        })
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
 
  @Builder
  buildNotTvDisplayDialog() {
    Column() {
      Text($r('app.string.p2p_wsc_display_invitation_sent_title'))
        .textAlign(TextAlign.Start)
        .fontSize($r('app.float.value_vp_20'))
        .lineHeight($r('app.float.value_vp_40'))
        .width('100%')
        .margin({ top: $r('app.float.value_vp_10'), left: $r('app.float.value_vp_20') })
      Text($r('app.string.p2p_wsc_display_to_message'))
        .textAlign(TextAlign.Start)
        .margin({ left: $r('app.float.value_vp_20') })
        .fontSize($r('app.float.value_vp_15'))
        .lineHeight($r('app.float.value_vp_30'))
        .width('100%')
      Text(this.deviceName)
        .textAlign(TextAlign.Start)
        .margin({ left: $r('app.float.value_vp_20') })
        .fontSize($r('app.float.value_vp_15'))
        .lineHeight($r('app.float.value_vp_20'))
        .width('100%')
        .fontWeight(FontWeight.Bolder)
      Text($r('app.string.p2p_wsc_display_show_pin_message'))
        .textAlign(TextAlign.Start)
        .margin({ left: $r('app.float.value_vp_20') })
        .fontSize($r('app.float.value_vp_15'))
        .lineHeight($r('app.float.value_vp_30'))
        .width('100%')
      Text(this.pinCode)
        .textAlign(TextAlign.Start)
        .margin({ left: $r('app.float.value_vp_20') })
        .fontSize($r('app.float.value_vp_15'))
        .lineHeight($r('app.float.value_vp_20'))
        .width('100%')
        .fontWeight(FontWeight.Bolder)
      Row() {
        Button($r('app.string.p2p_wsc_display_ok_button'))
          .margin($r('app.float.value_vp_10'))
          .backgroundColor(Color.Transparent)
          .fontColor($r('app.color.not_tv_dialog_button_font_color'))
          .onClick(() => {
            console.info(TAG, 'user click ok');
            this.setAcceptResult(this.dialogType, true);
          })
      }
      .justifyContent(FlexAlign.End)
      .width('100%')
    }
    .width('100%')
    .justifyContent(FlexAlign.Center)
    .alignItems(HorizontalAlign.Start)
  }
 
  @Builder
  buildNotTvKeypadDialog() {
    Column() {
      Text($r('app.string.p2p_wsc_pbc_dialog_title'))
        .textAlign(TextAlign.Start)
        .fontSize($r('app.float.value_vp_20'))
        .lineHeight($r('app.float.value_vp_25'))
        .width('100%')
      Text(this.text)
        .textAlign(TextAlign.Start)
        .fontSize($r('app.float.value_vp_15'))
        .lineHeight($r('app.float.value_vp_30'))
        .width('100%')
      Text($r('app.string.p2p_wsc_keypad_enter_pin_message'))
        .textAlign(TextAlign.Start)
        .fontSize($r('app.float.value_vp_15'))
        .lineHeight($r('app.float.value_vp_30'))
        .width('100%')
      Row() {
        TextInput({ text: this.inputPinCode })
          .textAlign(TextAlign.Start)
          .maxLength(8)
          .showUnderline(true)
          .width($r('app.float.value_vp_120'))
          .onChange((value: string) => {
            this.inputPinCode = value;
          })
      }
      .width('100%')
      .margin({ bottom: $r('app.float.value_vp_10') })
      .justifyContent(FlexAlign.Start)
      Row() {
        Button($r('app.string.p2p_wsc_pbc_dialog_no_button'))
          .layoutWeight(1)
          .backgroundColor(Color.Transparent)
          .fontColor($r('app.color.not_tv_dialog_button_font_color'))
          .onClick(() => {
            console.info(TAG, 'user click cancel');
            this.setAcceptResult(this.dialogType, false);
          })
        Button($r('app.string.p2p_wsc_pbc_dialog_ok_button'))
          .layoutWeight(1)
          .backgroundColor(Color.Transparent)
          .fontColor($r('app.color.not_tv_dialog_button_font_color'))
          .onClick(() => {
            console.info(TAG, 'user click trust');
            this.setAcceptResult(this.dialogType, true);
          })
      }
      .width('100%')
      .justifyContent(FlexAlign.SpaceBetween)
      .alignItems(VerticalAlign.Center)
    }
    .width('100%')
    .padding($r('app.float.value_vp_24'))
    .justifyContent(FlexAlign.Center)
    .alignItems(HorizontalAlign.Center)
  }
 
  private openNotTvPbcDialog(): void {
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
 
  private openDisplayDialog(): void {
    console.info(TAG, 'openDialog');
    promptAction.openCustomDialog({
      builder: () => {
        if (deviceInfo.deviceType == DEVICE_TYPE_TV) {
          console.info(TAG, `deviceInfo.deviceType == DEVICE_TYPE_TV`);
          this.buildDisplayDialog();
        } else {
          console.info(TAG, `this.buildNotTvDisplayDialog();`);
          this.buildNotTvDisplayDialog();
        }
      },
      autoCancel: false,
      onWillDismiss: (dismissDialogAction: DismissDialogAction) => {
        console.info(TAG, `not allow dismiss dialog by reason ${dismissDialogAction.reason}`);
      }
    }).catch((err: BusinessError) => {
      console.info(TAG, `fail to open dialog, err=${JSON.stringify(err)}`);
      this.setAcceptResult(this.dialogType, false);
    })
  }

  private openDialog(): void {
    console.info(TAG, 'openDialog');
    promptAction.openCustomDialog({
      builder: () => {
        if (this.dialogType == this.WIFI_DIALOG_P2P_WSC_PBC) {
          this.buildPbcDialog();
        } else if (this.dialogType == this.WIFI_DIALOG_P2P_WSC_KEYPAD) {
          if (deviceInfo.deviceType == DEVICE_TYPE_TV) {
            this.buildKeypadDialog();
          } else {
            this.buildNotTvKeypadDialog();
          }
        }
      },
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
    this.deviceName  = AppStorage.get('p2pDeviceName') as string;
    this.pinCode  = AppStorage.get('p2pPinCode') as string;
    this.dialogType = AppStorage.get('wifiDialogType') as number;
    this.text = this.getFormatString($r('app.string.p2p_wsc_pbc_dialog_text'), this.deviceName);
    if (deviceInfo.deviceType != DEVICE_TYPE_TV && this.dialogType == this.WIFI_DIALOG_P2P_WSC_PBC) {
      this.openNotTvPbcDialog();
      return;
    }
 
    if (this.dialogType == this.WIFI_DIALOG_P2P_WSC_DISPLAY) {
      this.openDisplayDialog();
    } else {
      this.openDialog();
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
      parameters: { 'dialogType': dialogType, 'inputPinCode': this.inputPinCode }
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