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

import router from '@ohos.router';

const TAG = 'PrivacyLoadPage';

@Entry
@Component
struct PrivacyLoadPage {
  want: Want = {
    bundleName: 'com.huawei.hmos.location',
    abilityName: 'LocationPrivacyDetailAbility',
    parameters: {
      'ability.want.params.uiExtensionType': 'sys/commonUI'
    }
  }

  build() {
    Row() {
      Column() {
        UIExtensionComponent(this.want)
          .size({ width: '100%', height: '100%' })
          .onRemoteReady((proxy) => {})
          .onReceive((data) => {
            console.info(TAG, 'on receive ' + data['action']);
            if (data['action'] === 'exit') {
              router.clear();
            }
          })
          .onTerminated((data) => {
            console.info(TAG, 'onTerminated ' + data?.code);
            if (data?.code === 100) {
              router.clear();
            }
          })
          .onError((error) => {})
      }
      .width('100%')
    }
    .height('100%')
  }
}