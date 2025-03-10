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

import UIAbility from '@ohos.app.ability.UIAbility';
import Want from '@ohos.app.ability.Want';
import notificationManager from '@ohos.notificationManager';
import image from '@ohos.multimedia.image';
import wantAgent from '@ohos.app.ability.wantAgent';
import { WantAgent } from '@ohos.wantAgent';
import { BusinessError } from '@ohos.base';
import { DrawableDescriptor } from '@ohos.arkui.drawableDescriptor';
import opp from '@ohos.bluetooth.opp';
import commonEventManager from '@ohos.commonEventManager';
import backgroundTaskManager from '@ohos.backgroundTaskManager';

const TAG: string = '[BT_SEND_SERVICE]==>'

export default class BluetoothSendUIAbility extends UIAbility {
    private oppProfile = opp.createOppServerProfile();
    private timeInterval: number = 0;
    private capsuleNotificationID: number = 200;
    private cancelTransEvent: string = 'ohos.event.notification.BT.TAP_CANCEL';
    private subscriber: commonEventManager.CommonEventSubscriber | null = null;
    private subscribeinfo: commonEventManager.CommonEventSubscribeInfo = {
        events: [
            'ohos.event.notification.BT.LIVEVIEW_REMOVE'
        ]
    };
    private timerIdCreate: number;
    private timerIdCreated: boolean = false;

    onCreate(want: Want): void {
        console.info(TAG, 'BluetoothSendUIAbility onCreate');
        this.timerIdCreate = setTimeout(() => {
            console.info(TAG, 'onCreate 10 seconds but nothing received');
            this.handleTerminate();
        }, 10000);
        this.timerIdCreated = true;
        this.subscribeTransferState();
        this.readyReceiveEvent();
        this.startContinuousTask();
    }

    onDestroy(): void {
        console.info(TAG, 'BluetoothSendUIAbility onDestroy');
    }

    onForeground(): void {
        console.info(TAG, 'BluetoothSendUIAbility onForeground');
    }

    handleTerminate() {
        this.stopContinuousTask();
        this.context.terminateSelf();
    }

    readyReceiveEvent() {
        try {
            console.info(TAG, 'readyReceiveEvent');
            this.subscriber = commonEventManager.createSubscriberSync(this.subscribeinfo);
            try {
                commonEventManager.subscribe(this.subscriber,
                (err: BusinessError, data: commonEventManager.CommonEventData) => {
                    if (err) {
                        console.error(TAG, `subscribe failed, code is ${err.code}, message is ${err.message}`);
                        this.handleTerminate();
                    } else {
                        this.handleReceivedEvent(data);
                    }
                });
            } catch (error) {
                let err: BusinessError = error as BusinessError;
                console.error(TAG, `subscribe failed, code is ${err.code}, message is ${err.message}`);
                this.handleTerminate();
            }
        } catch (error) {
            let err: BusinessError = error as BusinessError;
            console.error(TAG, `createSubscriber failed, code is ${err.code}, message is ${err.message}`);
            this.handleTerminate();
        }
    }

    handleReceivedEvent(data: commonEventManager.CommonEventData) {
        console.info(TAG, 'handleReceivedEvent： ' + data.event);
        switch (data.event) {
          case 'ohos.event.notification.BT.LIVEVIEW_REMOVE': {
              this.oppProfile.cancelTransfer();
              this.handleTerminate();
              break;
          }
          default: {
              break;
          }
        }
    }

    subscribeTransferState() {
        try {
            this.subscriberLiveViewNotification();
            this.oppProfile.on('transferStateChange', (data: opp.OppTransferInformation) => {
                if (this.timerIdCreated) {
                    this.timerIdCreated = false;
                    clearTimeout(this.timerIdCreate);
                }
                if (data.status == 1) {
                    this.pullUpSendProgressNotification(data.currentBytes / data.totalBytes * 100,
                    this.getFileName(data.filePath));
                } else if (data.status == 2) {
                    if (data.result != 0) {
                        console.info(TAG, 'send fail');
                        this.cancelSendProgressNotification(0);
                        this.pullUpReceiveResultNotification(0, 1);
                    }
                    console.info(TAG, 'transfer success');
                    this.oppProfile.off('transferStateChange');
                }
            });
            console.log(TAG, 'oppProfile.transferStateChange');
        } catch (err) {
            console.error(TAG, 'subscribeTransferState err');
            this.handleTerminate();
        }
    }

    async getNotificationWantAgent(info: string): Promise<WantAgent> {
        let wantAgentObjUse: WantAgent;
        let wantAgentInfo: wantAgent.WantAgentInfo = {
            wants: [
                {
                    action: info,
                }
            ],
            actionType: wantAgent.OperationType.SEND_COMMON_EVENT,
            requestCode: 0,
            wantAgentFlags: [wantAgent.WantAgentFlags.CONSTANT_FLAG],
        };
        wantAgentObjUse = await wantAgent.getWantAgent(wantAgentInfo);
        console.info(TAG, 'getNotificationWantAgent success for ' + info);
        return wantAgentObjUse;
    }

    async publishFinishNotification(wantAgentObj: WantAgent, successNum: number, failNum: number) {
        console.info(TAG, 'publishFinishNotification');
        let notificationRequest: notificationManager.NotificationRequest = {
            content: {
                notificationContentType: notificationManager.ContentType.NOTIFICATION_CONTENT_BASIC_TEXT,
                normal: {
                    title: this.context.resourceManager.getStringSync($r('app.string.bluetooth_send_finish_title').id),
                    text: this.getFormatString($r('app.string.bluetooth_send_finish_text'),
                        this.getFormatPlural($r('app.plural.bluetooth_send_finish_success_text', successNum, successNum), successNum),
                        this.getFormatPlural($r('app.plural.bluetooth_send_finish_fail_text', failNum, failNum), failNum))
                }
            },
            id: 2,
            notificationSlotType: notificationManager.SlotType.SERVICE_INFORMATION,
            wantAgent: wantAgentObj,
            tapDismissed: true,
        };
        notificationManager.publish(notificationRequest).then(() => {
            console.info(TAG, 'publishFinishNotification success');
        }).catch((err: BusinessError) => {
            console.error(TAG, 'publishFinishNotification fail');
            this.handleTerminate();
        });
    }

    async pullUpReceiveResultNotification(successNum: number, failNum: number) {
        console.info(TAG, 'pullUpReceiveResultNotification');
        let wantAgentObj: WantAgent;
        wantAgentObj = await this.getNotificationWantAgent('ohos.event.notification.BT.FINISH_SEND');
        await this.publishFinishNotification(wantAgentObj, successNum, failNum);
    }

    async publishTransProgessNotification(imagePixelMapButton: image.PixelMap, imagePixelMapCapsule: image.PixelMap,
        wantAgentObjRemove: WantAgent, percent: number, name: string) {
        console.info(TAG, 'publishTransProgessNotification');
        let notificationRequest: notificationManager.NotificationRequest = {
            notificationSlotType: notificationManager.SlotType.LIVE_VIEW,
            id: this.capsuleNotificationID,
            content: {
                notificationContentType: notificationManager.ContentType.NOTIFICATION_CONTENT_SYSTEM_LIVE_VIEW,
                systemLiveView: {
                    title: this.context.resourceManager.getStringSync($r('app.string.bluetooth_transfer_notification_title').id),
                    text: this.context.resourceManager.getStringSync($r('app.string.bluetooth_transfer_send_text').id) + name,
                    typeCode: 8,
                    button: {
                        names: [this.cancelTransEvent],
                        icons: [imagePixelMapButton],
                    },
                    capsule: {
                        title: 'bluetooth',
                        icon: imagePixelMapCapsule,
                        backgroundColor: '#0A59F7',
                    },
                    progress: {
                        maxValue: 100,
                        currentValue: percent,
                        isPercentage: true,
                    },
                }
            },
            tapDismissed: false,
            removalWantAgent: wantAgentObjRemove
        };

        notificationManager.publish(notificationRequest).then(() => {
            console.info(TAG, 'publishTransProgessNotification success');
            if (percent === 100) {
                this.cancelSendProgressNotification(1);
            }
        }).catch((err: BusinessError) => {
            console.error(TAG, 'publishTransProgessNotification fail');
        });
    }

    async pullUpSendProgressNotification(percent: number, name: string) {
        const currentDate: Date = new Date();
        const currentTimeInMsUsingGetTime: number = currentDate.getTime();
        if (percent !== 100 && (currentTimeInMsUsingGetTime - this.timeInterval) < 1000) {
            return;
        }
        this.timeInterval = currentTimeInMsUsingGetTime;

        console.info(TAG, 'ready to pullUpSendProgressNotification');
        let imagePixelMapButton: image.PixelMap | undefined = undefined;
        let imagePixelMapCapsule: image.PixelMap | undefined = undefined;
        try {
            let drawableDescriptor1: DrawableDescriptor = this.context.resourceManager.getDrawableDescriptor($r('app.media.public_cancel_filled').id);
            imagePixelMapButton = drawableDescriptor1.getPixelMap();
            let drawableDescriptor2: DrawableDescriptor = this.context.resourceManager.getDrawableDescriptor($r('app.media.foreground').id);
            imagePixelMapCapsule = drawableDescriptor2.getPixelMap();
        } catch (error) {
            let code = (error as BusinessError).code;
            let message = (error as BusinessError).message;
            console.error(TAG, `getDrawableDescriptor failed, error code is ${code}, message is ${message}`);
            return;
        }

        let wantAgentObjRemove: WantAgent;
        wantAgentObjRemove = await this.getNotificationWantAgent('ohos.event.notification.BT.LIVEVIEW_REMOVE');
        await this.publishTransProgessNotification(imagePixelMapButton, imagePixelMapCapsule, wantAgentObjRemove,
            percent, name);
    }

    cancelSendProgressNotification(successOrNot: number) {
        console.info(TAG, 'cancelSendProgressNotification ready to cancel.');
        notificationManager.cancel(this.capsuleNotificationID).then(() => {
            console.info(TAG, 'Succeeded in canceling notification.');
            if (successOrNot === 1) {
                this.pullUpReceiveResultNotification(1, 0);
            }
            this.handleTerminate();
        }).catch((err: BusinessError) => {
            console.error(TAG, `failed to cancel notification. Code is ${err.code}, message is ${err.message}`)
            this.handleTerminate();
        });
    }

    subscriberLiveViewNotification(): void {
        let subscriber: notificationManager.SystemLiveViewSubscriber = {
            onResponse: (id: number, option: notificationManager.ButtonOptions) => {
                switch (option.buttonName) {
                    case this.cancelTransEvent: {
                        console.info(TAG, 'cancel transfer.');
                        this.oppProfile.cancelTransfer();
                        this.cancelSendProgressNotification(0);
                        break;
                    }
                    default: {
                        break;
                    }
                }
            }
        };
        try {
            notificationManager.subscribeSystemLiveView(subscriber);
        } catch(e) {
            console.error(TAG, 'subscriberLiveViewNotification fail');
        }
    }

    getFileName(filePath: string): string {
        let extension = filePath.substring(filePath.lastIndexOf('/') + 1);
        return extension;
    }

    getFormatString(resource: Resource, value1: string, value2: string): string {
        let result = this.context.resourceManager.getStringSync(resource.id);
        result = result.replace('%1$s', value1);
        result = result.replace('%2$s', value2);
        return result;
    }

    getFormatPlural(resource: Resource, value: number): string {
        let result = this.context.resourceManager.getPluralStringValueSync(resource.id, value);
        result = result.replace('%d', value.toString());
        return result;
    }

    async startContinuousTask() {
        let wantAgentObj: WantAgent;
        wantAgentObj = await this.getNotificationWantAgent('ohos.event.notification.BT.BACK_RUNNING');
        backgroundTaskManager.startBackgroundRunning(this.context,
        backgroundTaskManager.BackgroundMode.BLUETOOTH_INTERACTION, wantAgentObj).then(() => {
            console.info(TAG, `Succeeded in operationing startBackgroundRunning.`);
        }).catch((err: BusinessError) => {
            console.error(TAG, `Failed to operation startBackgroundRunning. Code is ${err.code}, message is ${err.message}`);
        });
    }

    stopContinuousTask() {
        backgroundTaskManager.stopBackgroundRunning(this.context).then(() => {
            console.info(TAG, `Succeeded in operationing stopBackgroundRunning.`);
        }).catch((err: BusinessError) => {
            console.error(TAG, `Failed to operation stopBackgroundRunning. Code is ${err.code}, message is ${err.message}`);
        });
    }
}