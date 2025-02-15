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
import picker from '@ohos.file.picker';
import type { BusinessError } from '@ohos.base';
import common from '@ohos.app.ability.common';
import commonEventManager from '@ohos.commonEventManager';

let TAG = '[BT_RECEIVE_PAGE]===>';
const BT_NI_GET_URI_EVENT: string = 'ohos.event.notification.BT.GET_URI';

@Entry
@Component
struct btCreateFilePickerPage {
  private context = getContext(this) as common.UIAbilityContext;

  aboutToAppear() {
    console.info(TAG, 'aboutToAppear');
    let documentSaveOptions = new picker.DocumentSaveOptions();
    documentSaveOptions.pickerMode = picker.DocumentPickerMode.DOWNLOAD;
    const documentPicker = new picker.DocumentViewPicker();
    documentPicker.save(documentSaveOptions).then((documentSaveResult: Array<string>) => {
      console.info('opp documentViewPicker.save to file succeed and uris are:' + documentSaveResult.length + documentSaveResult[0]);
      const options: commonEventManager.CommonEventPublishData = {
        code: 0,
        data: 'message',
        subscriberPermissions: [],
        isOrdered: true,
        isSticky: false,
        parameters: { 'uri': documentSaveResult[0] }
      }
      commonEventManager.publish(BT_NI_GET_URI_EVENT, options, (err) => {
        if (err) {
          console.info(TAG, 'get file URI event publish failed.' + JSON.stringify(err));
        } else {
          console.info(TAG, 'get file URI event publish success.');
        }
      })
      this.context.terminateSelf();
    }).catch((err: BusinessError) => {
      this.context.terminateSelf();
      console.error(`opp Invoke documentViewPicker.save failed, code is ${err.code}, message is ${err.message}`);
    })
  }

  aboutToDisappear() {
    let session = AppStorage.get<UIExtensionContentSession>('ConfirmSession');
    if (session) {
      console.info(TAG, 'session.terminateSelf');
      session.terminateSelf();
    }
  }

  onCancel() {
    console.info(TAG, 'onCancel is called')
  }

  build() {
  }
}