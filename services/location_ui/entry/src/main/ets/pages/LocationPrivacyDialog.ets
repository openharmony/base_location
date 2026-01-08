/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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
import common from '@ohos.app.ability.common';
import router from '@ohos.router';
import resourceManager from '@ohos.resourceManager';
import { BusinessError } from '@ohos.base';
import osAccount from '@ohos.account.osAccount';
import deviceInfo from '@ohos.deviceInfo'

let TAG = '[Location-Privacy]==>';
const LOCATION_PRIVACY_ACCEPT_EVENT: string = 'usual.event.LOCATION_PRIVACY_ACCEPT';
const LOCATION_PRIVACY_REJECT_EVENT: string = 'usual.event.LOCATION_PRIVACY_REJECT';
const BODY_L = getNumberByResourceId(125830970, 16);
const TITLE_S = getNumberByResourceId(125830966, 20);
const BUTTON_HORIZONTAL_SPACE: number = getNumberByResourceId(125831051, 8);
const BUTTON_LAYOUT_WEIGHT: number = 1;
const FLAG_BEGIN: string = '<a';
const FLAG_BEGIN_END: string = '>';
const FLAG_END: string = '</a>';

@CustomDialog
struct LocationCustomDialog {
  controller?: CustomDialogController
  cancel: () => void = () => {}
  accept: () => void = () => {}
  private context = getContext(this) as common.UIAbilityContext;

  build() {
    Column() {
      Column() {
        Row() {
          Text($r('app.string.location_target_statement_title'))
            .fontSize(`${TITLE_S}fp`)
            .fontWeight(FontWeight.Bold)
            .textAlign(TextAlign.Start)
            .width("100%")
            .padding(this.getTitleAreaPadding())
        }
      }
      Column() {
        Row() {
          Text() {
            Span(this.getStatementContent()[0])
            Span(this.getStatementContent()[1])
              .fontColor($r('sys.color.ohos_id_color_text_hyperlink'))
              .fontWeight(FontWeight.Bold)
              .onClick(() => {
                console.info(TAG, 'click privacy page');
                this.routePage();
              })
            Span(this.getStatementContent()[2])
          }
          .focusable(true)
          .fontSize(`${BODY_L}fp`)
          .padding(this.getContentPadding())
        }
      }
      Row() {
        Button($r('app.string.cancel_button'))
          .onClick(() => {
            this.cancel();
            if (this.controller) {
              this.controller.close();
            }
          })
          .layoutWeight(BUTTON_LAYOUT_WEIGHT)
          .buttonStyle(ButtonStyleMode.TEXTUAL)
          .labelStyle({
            maxLines: 1,
            maxFontSize: `${BODY_L}fp`,
            minFontSize: 9
          })
        Row() {
          Divider()
            .width($r('sys.float.alert_divider_width'))
            .height($r('sys.float.alert_divider_height'))
            .color($r('sys.color.alert_divider_color'))
            .vertical(true)
        }
        .width(BUTTON_HORIZONTAL_SPACE * 2)
        .justifyContent(FlexAlign.Center)
        Button($r('app.string.confirm_button'))
          .onClick(() => {
            this.accept();
            if (this.controller) {
              this.controller.close();
            }
          })
          .layoutWeight(BUTTON_LAYOUT_WEIGHT)
          .type(ButtonType.ROUNDED_RECTANGLE)
          .labelStyle({
            maxLines: 1,
            maxFontSize: `${BODY_L}fp`,
            minFontSize: 9
          })
      }
      .padding(this.getButtonPadding())
    }
  }

  async routePage() {
    let options: router.RouterOptions = {
      url: 'pages/PrivacyLoadPage',
    }
    try {
      await router.pushUrl(options);
    } catch (err) {
      console.info(TAG, `fail callback, code: ${(err as BusinessError).code}`);
    }
  }

  private getStatementContent(): Array<string> {
    let result: Array<string> = ['', '', ''];
    try {
      let text = this.context.resourceManager.getStringByNameSync("location_target_statement_content");
      let beginIndex = text.indexOf(FLAG_BEGIN);
      if (beginIndex !== 0 && beginIndex !== -1) {
        result[0] = text.substr(0, beginIndex);
        text = text.substr(beginIndex, text.length);
      }
      let beginEndIndex = text.indexOf(FLAG_BEGIN_END);
      let endIndex = text.indexOf(FLAG_END);
      if (beginEndIndex !== -1 && endIndex !== -1) {
        result[1] = text.substr(beginEndIndex + 1, endIndex - FLAG_END.length + 1);
        result[2] = text.substr(endIndex + FLAG_END.length, text.length);
      }
      return result;
    } catch (error) {
      let code = (error as BusinessError).code;
      let message = (error as BusinessError).message;
      console.error(TAG, `getStatementContent failed, error code is ${code}, message is ${message}`);
      return result;
    }
  }

  /**
   * get title area padding
   *
   * @returns padding
   */
  private getTitleAreaPadding(): Padding {
    return {
      top: $r('sys.float.alert_title_padding_top'),
      right: $r('sys.float.alert_title_padding_right'),
      left: $r('sys.float.alert_title_padding_left'),
      bottom: 0,
    };
  }

  /**
   * get content padding
   *
   * @returns padding
   */
  private getContentPadding(): Padding {
    return {
      top: $r('sys.float.alert_content_default_padding'),
      right: $r('sys.float.alert_content_default_padding'),
      left: $r('sys.float.alert_content_default_padding'),
      bottom: $r('sys.float.alert_content_default_padding'),
    };
  }

  /**
   * get button padding
   *
   * @returns padding
   */
  private getButtonPadding(): Padding {
    return {
      top: 0,
      right: $r('sys.float.alert_content_default_padding'),
      left: $r('sys.float.alert_content_default_padding'),
      bottom: $r('sys.float.alert_content_default_padding'),
    };
  }
}

@Entry
@Component
struct dialogPlusPage {
  dialogController: CustomDialogController = new CustomDialogController({
    builder: LocationCustomDialog({
      cancel: this.onCancel,
      accept: this.onAccept,
    }),
    cancel: this.existApp,
    autoCancel: false,
    alignment: DialogAlignment.Bottom,
    customStyle: false
  })

  aboutToAppear() {
    console.info(TAG, 'aboutToAppear execute LocationPrivacyDialog')
  }

  aboutToDisappear() {
    let session = AppStorage.get<UIExtensionContentSession>('ConfirmSession');
    if (session) {
      session.terminateSelf();
    }
  }

  async getForegroundOsAccountLocalId(): Promise<number>
  {
    let localId = 100;
    let accountManager = osAccount.getAccountManager();
    try {
      if (deviceInfo.sdkApiVersion >= 23) {
        let isMain = await accountManager.isMainOsAccount();
        if (isMain) {
          localId = await accountManager.getForegroundOsAccountLocalId();
        } else {
          let displayId = AppStorage.get<number>('screenId') as number || 0;
          localId = await accountManager.getForegroundOsAccountLocalId(displayId);
        }
      } else {
        localId = await accountManager.getForegroundOsAccountLocalId();
      }
      return localId;
    } catch (err) {
        console.info(TAG, `getForegroundOsAccountLocalId exception: ${err.code} ${err.message}`)
      return localId;
    }
  }

  async onCancel() {
    console.info(TAG, 'Callback when the first button is clicked')
    let userId = await this.getForegroundOsAccountLocalId();
    const options: commonEventManager.CommonEventPublishData = {
      code: 0,
      data: userId.toString(),
      isSticky: false,
      parameters: { 'message': 'reject' }
    }

    commonEventManager.publish(LOCATION_PRIVACY_REJECT_EVENT, options, (err) => {
      if (err) {
        console.info(TAG, '[CommonEvent] PublishCallBack err=' + JSON.stringify(err));
      } else {
        console.info(TAG, '[CommonEvent] Publish success')
      }
    })

    let session = AppStorage.get<UIExtensionContentSession>('ConfirmSession');
    if (session) {
      session.terminateSelf();
    }
  }

  async onAccept() {
    console.info(TAG, 'Callback when the second button is clicked')
    let userId = await this.getForegroundOsAccountLocalId();
    const options: commonEventManager.CommonEventPublishData = {
      code: 0,
      data: userId.toString(),
      isSticky: false,
      parameters: { 'message': 'accept' }
    }

    commonEventManager.publish(LOCATION_PRIVACY_ACCEPT_EVENT, options, (err) => {
      if (err) {
        console.info(TAG, '[CommonEvent] PublishCallBack err=' + JSON.stringify(err));
      } else {
        console.info(TAG, '[CommonEvent] Publish success')
      }
    })

    let session = AppStorage.get<UIExtensionContentSession>('ConfirmSession');
    if (session) {
      session.terminateSelf();
    }
  }

  existApp() {
    console.info(TAG, 'Click the callback in the blank area')
    let session = AppStorage.get<UIExtensionContentSession>('ConfirmSession');
    if (session) {
      session.terminateSelf();
    }
  }

  build() {
    Column(this.dialogController.open()) {}
  }
}

/**
  * get resource size
  *
  * @param resourceId resource id
  * @param defaultValue default value
  * @returns resource size
  */
function getNumberByResourceId(resourceId: number, defaultValue: number, allowZero?: boolean): number {
  try {
    let sourceValue: number = resourceManager.getSystemResourceManager().getNumber(resourceId);
    if (sourceValue > 0 || allowZero) {
      return sourceValue;
    } else {
      return defaultValue;
    }
  } catch (error) {
    return defaultValue;
  }
}