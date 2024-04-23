/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

let TAG = '[Location-Privacy]==>';
const LOCATION_PRIVACY_ACCEPT_EVENT: string = "usual.event.LOCATION_PRIVACY_ACCEPT";
const LOCATION_PRIVACY_REJECT_EVENT: string = "usual.event.LOCATION_PRIVACY_REJECT";

@Entry
@Component
struct dialogPlusPage {
  @State title: string = '';
  @State message: string = '';

  aboutToAppear() {
    console.info(TAG, 'aboutToAppear execute LocationPrivacyDialog')
    AlertDialog.show(
      {
        title: $r('app.string.location_target_statement_title'),
        message: $r('app.string.location_target_statement_content'),
        autoCancel: false,
        primaryButton: {
          value: $r('app.string.cancel_button'),
          action: () => {
            this.onCancel();
          }
        },
        secondaryButton: {
          enabled: true,
          defaultFocus: true,
          style: DialogButtonStyle.HIGHLIGHT,
          value: $r('app.string.confirm_button'),
          action: () => {
            this.onAccept();
          }
        },
      }
    )
  }

  aboutToDisappear() {
    let session = AppStorage.get<UIExtensionContentSession>('ConfirmSession');
    if (session) {
      session.terminateSelf();
    }
  }

  onCancel() {
    console.info(TAG, 'Callback when the first button is clicked')

    const options: commonEventManager.CommonEventPublishData = {
      code: 0,
      data: 'message',
      subscriberPermissions: [''],
      isOrdered: true,
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

  onAccept() {
    console.info(TAG, 'Callback when the second button is clicked')

    const options: commonEventManager.CommonEventPublishData = {
      code: 0,
      data: 'message',
      subscriberPermissions: [''],
      isOrdered: true,
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
  }
}