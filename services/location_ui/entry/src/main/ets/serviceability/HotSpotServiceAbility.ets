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

import ServiceExtensionAbility from '@ohos.app.ability.ServiceExtensionAbility';
import Want from '@ohos.app.ability.Want';
import notificationManager from '@ohos.notificationManager';
import Base from '@ohos.base';
import image from '@ohos.multimedia.image';
import fs from '@ohos.file.fs';
import wantAgent from '@ohos.app.ability.wantAgent';
import { WantAgent } from '@ohos.app.ability.wantAgent';
import deviceInfo from '@ohos.deviceInfo';
import LogUtils from '../common/Util';

const TAG: string = '[HotSpotServiceAbility-In]==>'
const HOTSPOT_IDLE_NOTIFICATION_ID: number = 101001;
const THERMAL_STOP_AP_NOTIFICATION_ID: number = 101002;
const HOTSPOT_STA_JOIN_NOTIFICATION_ID: number = 101003;
const DEVICE_TYPE_PC: string = 'pc';
const DEVICE_TYPE_PC_NEW: string = '2in1';

export default class HotSpotServiceAbility extends ServiceExtensionAbility {
    private willPublish: boolean = false;

    private isPC(): boolean {
 	    return deviceInfo.deviceType === DEVICE_TYPE_PC || deviceInfo.deviceType === DEVICE_TYPE_PC_NEW
 	}

    onCreate(want: Want): void {
        LogUtils.i(TAG, `HotSpotServiceAbility onCreate`);
    }

    onDestroy(): void {
        LogUtils.i(TAG, `HotSpotServiceAbility onDestroy`);
    }
    
    onRequest(want: Want, startId: number): void {
        LogUtils.i(TAG, `onRequest`);
        if (!want || !want.parameters) {
            LogUtils.e(TAG, `Invalid want parameters`);
            return;
        }
        let operateType : number = want.parameters?.['operateType'] as number;
        let notificationId : number = want.parameters?.['notificationId'] as number;
        if (operateType == 1) {
            LogUtils.i(TAG, `onRequest start notification id is:${notificationId}`);
            this.willPublish = true;
            this.hotSpsotNotificationStart(notificationId);
        } else {
            LogUtils.i(TAG, `onRequest cancel notification id is:${notificationId}`);
            this.willPublish = false;
            this.cancelHotSpotNotification(notificationId);
        }
    }

    public hotSpsotNotificationStart(id: number) {
        let filePath: string = '/system/etc/wifi/ic_connection_notify_hotspot_unusable.png';
        if (id === HOTSPOT_STA_JOIN_NOTIFICATION_ID) {
            if (!this.isPC()) {
                return;
            }
            filePath = '/system/etc/wifi/ic_hotspot_sta_connection.png';
        }
        let file: fs.File | undefined;
        let imageSourceApi: image.ImageSource | undefined;
        let pixelmap: image.PixelMap | undefined;
        try {
            file = fs.openSync(filePath, fs.OpenMode.READ_ONLY);
            imageSourceApi = image.createImageSource(file.fd);
            imageSourceApi.createPixelMap()
            .then((resultPixelmap: image.PixelMap) => {
                LogUtils.i(TAG, `Create pixelMap success.`);
                pixelmap = resultPixelmap;
                if (id === HOTSPOT_STA_JOIN_NOTIFICATION_ID) {
                    this.publishHotspotSimpleNotification(pixelmap, id);
                } else {
                    this.publishHotSpotWantAgentCommonEvent(pixelmap, id);
                }
            })
            .catch((error: Error) => {
                LogUtils.e(TAG, `Create pixelMap failed: ${JSON.stringify(error)}`);
            });
        } catch (err) {
            LogUtils.e(TAG, `Create pixelMap error: ${JSON.stringify(err)}`);
        } finally {
            if (file) {
                try {
                    fs.closeSync(file);
                } catch (closeErr) {
                    LogUtils.e(TAG, `Failed to close file: ${JSON.stringify(closeErr)}`);
                }
            }
            if (imageSourceApi) {
                try {
                    imageSourceApi.release();
                } catch (releaseErr) {
                    LogUtils.e(TAG, `Failed to release imageSource: ${JSON.stringify(releaseErr)}`);
                }
            }
            if (pixelmap) {
                try {
                    pixelmap.release();
                } catch (releaseErr) {
                    LogUtils.e(TAG, `Failed to release pixelmap: ${JSON.stringify(releaseErr)}`);
                }
            }
        }
    }

    public async publishHotspotSimpleNotification(pixelMap: image.PixelMap, id: number) {
        let templateTitle: string = '';
        let templateText: string = '';
        try {
            templateTitle = this.context.resourceManager.getStringSync($r('app.string.hotspot_sta_join_notify_title').id);
            templateText = this.context.resourceManager.getStringSync($r('app.string.hotspot_sta_join_notify_text').id);
        } catch (err) {
            LogUtils.e(TAG, `Failed to get resource string: ${JSON.stringify(err)}`);
            return;
        }
        let notificationRequest: notificationManager.NotificationRequest = {
            id: id,
            notificationSlotType: notificationManager.SlotType.SERVICE_INFORMATION,
            content: {
                notificationContentType: notificationManager.ContentType.NOTIFICATION_CONTENT_BASIC_TEXT,
                normal: {
                    title: templateTitle,
                    text: templateText
                }
            },
            smallIcon: pixelMap
        };
        LogUtils.i(TAG, `publishHotspotSimpleNotification-${this.willPublish}`);
        if (this.willPublish) {
            notificationManager.publish(notificationRequest).then(() => {
                LogUtils.i(TAG, `Publish hotspot sta join notification success`);
            }).catch((err: Base.BusinessError) => {
                LogUtils.i(TAG, `Publish hotspot sta join notification fail`);
            });
        }
    }

    public async publishHotSpotWantAgentCommonEvent(pixelMap: image.PixelMap, id: number) {
        let wantAgentObj: WantAgent;
        wantAgentObj = await this.getHotSpotWantAgent();
        LogUtils.i(TAG, `publishHotSpotWantAgentCommonEvent-getWantAgentObj-success`);
        await this.publishHotSpotWantAgent(wantAgentObj, pixelMap, id)
    }

    public async getHotSpotWantAgent(): Promise< WantAgent> {
        let wantAgentObjUse: WantAgent;
        let wantAgentInfo: wantAgent.WantAgentInfo = {
            wants: [
                {
                    action: 'ohos.event.notification.wifi.TAP_ENABLE_HOTSPOT',
                    parameters: {},
                }
            ],
            actionType: wantAgent.OperationType.SEND_COMMON_EVENT,
            requestCode: 0,
            wantAgentFlags: [wantAgent.WantAgentFlags.CONSTANT_FLAG],
        };
        wantAgentObjUse = await wantAgent.getWantAgent(wantAgentInfo);
        LogUtils.i(TAG, `getWantAgent-wantAgentObjUse-success`);
        return wantAgentObjUse;
    }

 public async publishHotSpotWantAgent(wantAgentObj: WantAgent, pixelmap: image.PixelMap, id: number) {
        let templateTitle: string = '';
        let templateText: string = '';
        try {
            templateTitle = this.context.resourceManager.getStringSync($r('app.string.condition_netshare_hotspot_close').id);
            switch (id) {
              case HOTSPOT_IDLE_NOTIFICATION_ID: {
                templateText = this.context.resourceManager.getStringSync($r('app.string.stop_idle_tether_notification_message').id);
                break;
              }
              case THERMAL_STOP_AP_NOTIFICATION_ID: {
                templateText = this.context.resourceManager.getStringSync($r('app.string.thermal_stop_tether_notification_message').id);
                break;
              }
              default: {
                break;
              }
            }
        } catch (err) {
            LogUtils.e(TAG, `Failed to get resource string: ${JSON.stringify(err)}`);
            return;
        }
        let buttonsTitle: string = '';
        try {
            buttonsTitle = this.context.resourceManager.getStringSync($r('app.string.stop_idle_tether_notification_action_text').id);
        } catch (err) {
            LogUtils.e(TAG, `Failed to get button title: ${JSON.stringify(err)}`);
            return;
        }
        let notificationRequest: notificationManager.NotificationRequest = {
            id: id,
            notificationSlotType: notificationManager.SlotType.SERVICE_INFORMATION,
            content: {
                notificationContentType: notificationManager.ContentType.NOTIFICATION_CONTENT_BASIC_TEXT,
                normal: {
                    title: templateTitle,
                    text: templateText
                }
            },
            extraInfo: {
                hw_custom_appname: templateTitle
            },
            notificationControlFlags: 2,
            actionButtons: [
                {
                    title: buttonsTitle, wantAgent: wantAgentObj
                }
            ],
            smallIcon: pixelmap
        };
        LogUtils.i(TAG, `publishHotSpotWantAgent-${this.willPublish}`);
        if (this.willPublish) {
            notificationManager.publish(notificationRequest).then(() => {
                LogUtils.i(TAG, `Publish wifi notification success`);
            }).catch((err: Base.BusinessError) => {
                LogUtils.i(TAG, `Publish wifi notification fail`);
            });
        }
    }

    public async cancelHotSpotNotification(id: number) {
        notificationManager.cancel(id).then(() => {
            LogUtils.i(TAG, `Cancel wifi notification success`);
        }).catch((err: Base.BusinessError) => {
            LogUtils.i(TAG, `Cancel wifi notification fail`);
        });
    }
}