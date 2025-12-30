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

import ServiceExtensionAbility from '@ohos.app.ability.ServiceExtensionAbility';
import Want from '@ohos.app.ability.Want';
import rpc from '@ohos.rpc';
import NotificationManager from '@ohos.notificationManager';
import Notification from '@ohos.notification';
import Base from '@ohos.base';
import image from '@ohos.multimedia.image';
import fs from '@ohos.file.fs';
import wantAgent from '@ohos.app.ability.wantAgent';
import { WantAgent } from '@ohos.app.ability.wantAgent'

const TAG: string = '[WIFI_SERVICE_NI]==>'

export default class WifiServiceAbility extends ServiceExtensionAbility {
    private WIFI_PORTAL_CONNECTED: number = 0;
    private WIFI_PORTAL_TIMEOUT: number = 1;
    private WIFI_PORTAL_FOUND: number = 2;
    private WIFI_5G_AUTO_IDENTIFY_CONN_FOUND: number = 3;
    private willPublish: boolean = false;

    onCreate(want: Want): void {
        console.info(TAG, `WifiServiceAbility onCreate`);
    }

    onDestroy(): void {
        console.info(TAG, `WifiServiceAbility onDestroy`);
    }

    onRequest(want: Want, startId: number): void {
        let logWant = { ...want };
        let originalSSID = want.parameters?.['ssid'] as string;
        delete logWant.parameters?.['ssid'];
        logWant.parameters = {
            ...logWant.parameters,
            'ssid': this.maskSSID(originalSSID)
        };
        console.info(TAG, `WifiServiceAbility onRequest, want: ${JSON.stringify(logWant)}`);        
        let file: fs.File | undefined;
        let imageSourceApi: image.ImageSource | undefined;
        let pixelmap: image.PixelMap | undefined;
        let operateType : number = want.parameters?.['operateType'] as number;
        let notificationId : number = want.parameters?.['notificationId'] as number;
        if (operateType == 1) {
            this.willPublish = true;
            let status : number = want.parameters?.['status'] as number;
            let ssid : string = want.parameters?.['ssid'] as string;
            try {
                let file = fs.openSync('/system/etc/wifi/portal_notification.png', fs.OpenMode.READ_ONLY);
                const imageSourceApi: image.ImageSource = image.createImageSource(file.fd);
                imageSourceApi.createPixelMap()
                .then((pixelmap: image.PixelMap) => {
                    console.info(TAG, `Create pixelMap success.`);
                    this.publishWantAgentCommonEvent(pixelmap, notificationId, status, ssid);
                })
                .catch((error: string) => {
                    console.info(TAG, `Create pixelMap failed`);
                });
            } catch (err) {
                console.info(TAG, `Create pixelMap error: ${JSON.stringify(err)}`);
            } finally {
                if (file) {
                    fs.closeSync(file);
                }
                imageSourceApi?.release();
                pixelmap?.release();
            }
        } else {
            this.willPublish = false;
            this.cancelNotification(notificationId);
        }
    }

    publishWantAgentCommonEvent = async (pixelmap: image.PixelMap, id: number, status: number, ssid: string) => {
        let wantAgentObj: WantAgent;
        let wantAgentInfo: wantAgent.WantAgentInfo = {
            wants: [
                {
                    action: 'ohos.event.notification.wifi.TAP_NOTIFICATION',
                    parameters: { 'notificationId': id },
                }
            ],
            actionType: wantAgent.OperationType.SEND_COMMON_EVENT,
            requestCode: 0,
            wantAgentFlags: [wantAgent.WantAgentFlags.CONSTANT_FLAG],
        };

        wantAgent.getWantAgent(wantAgentInfo, (err: Base.BusinessError, data: WantAgent) => {
            if (err) {
                console.info(TAG, `Failed to get want agent. Code is ${err.code}, message is ${err.message}`);
                return;
            }
            console.info(TAG, 'Succeeded in getting want agent.');
            wantAgentObj = data;
            this.publishWantAgent(wantAgentObj, pixelmap, id, status, ssid);
        });
    }

    publishWantAgent = async (wantAgentObj: WantAgent, pixelmap: image.PixelMap, id: number, status: number, ssid: string) => {
        let templateTitle: string = '';
        let templateText: string = '';
        switch (status) {
          case this.WIFI_PORTAL_CONNECTED:
            templateTitle = this.getFormatString($r('app.string.wifi_portal_connected_notify_title'), ssid);
            templateText = this.context.resourceManager.getStringSync($r('app.string.wifi_portal_connected_notify_text').id);
            break;
          case this.WIFI_PORTAL_TIMEOUT:
            templateTitle = this.context.resourceManager.getStringSync($r('app.string.wifi_portal_connect_timeout_notify_title').id);
            templateText = this.getFormatString($r('app.string.wifi_portal_connect_timeout_notify_text'), ssid);
            break;
          case this.WIFI_PORTAL_FOUND:
            templateTitle = this.context.resourceManager.getStringSync($r('app.string.wifi_portal_found_notify_title').id);
            templateText = this.getFormatString($r('app.string.wifi_portal_found_notify_text'), ssid);
            break;
          case this.WIFI_5G_AUTO_IDENTIFY_CONN_FOUND:
            templateTitle = this.context.resourceManager.getStringSync($r('app.string.wifipro_better_network_pop_up_title').id);
            templateText = this.getFormatString($r('app.string.wifipro_better_network_notify_text'), ssid);
            break;
          default:
            break;
        }
        let notificationRequest: NotificationManager.NotificationRequest = {
            id: id,
            content: {
                contentType: Notification.ContentType.NOTIFICATION_CONTENT_BASIC_TEXT,
                normal: {
                    title: templateTitle,
                    text: templateText
                }
            },
            extraInfo: {
                hw_custom_appname: templateTitle
            },
            slotType: Notification.SlotType.SERVICE_INFORMATION,
            notificationControlFlags: 2,
            isUnremovable: true,
            wantAgent: wantAgentObj,
            smallIcon: pixelmap
        };

        if (this.willPublish) {
            NotificationManager.publish(notificationRequest).then(() => {
                console.info(TAG, 'Publish wifi notification success');
            }).catch((err: Base.BusinessError) => {
                console.error(TAG, 'Publish wifi notification fail');
            });
        }
    }

    cancelNotification = async (id: number) => {
        NotificationManager.cancel(id).then(() => {
            console.info(TAG, 'Cancel wifi notification success');
        }).catch((err: Base.BusinessError) => {
            console.error(TAG, 'Cancel wifi notification fail');
        });
    }

    getFormatString(resource: Resource, subStr: string): string {
        let result = this.context.resourceManager.getStringSync(resource.id);
        return result.replace(new RegExp('%s', 'gm'), subStr);
    }

    maskSSID(ssid: string): string {
        if (!ssid) {
            return '';
        }
        return ssid.length > 2 ? ssid.substring(0, 2) + '****' : '****';
    }
}