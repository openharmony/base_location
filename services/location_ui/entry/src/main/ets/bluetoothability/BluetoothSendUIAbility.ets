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
import fs from '@ohos.file.fs';
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
    private filePathToFdMap = new Map();
    private timeInterval: number = 0;
    private capsuleNotificationID: number = 200;
    private cancelTransEvent: string = 'ohos.event.notification.BT.TAP_CANCEL';
    private filePaths: Array<string> = [];
    private fileHolders: Array<opp.FileHolder> = [];
    private subscriber: commonEventManager.CommonEventSubscriber | null = null;
    private subscribeinfo: commonEventManager.CommonEventSubscribeInfo = {
        events: [
            'usual.event.bluetooth.OPP.SEND'
        ]
    }

    onCreate(want: Want): void {
        console.info(TAG, 'BluetoothSendUIAbility onCreate');
        this.readyReceiveEvent();
        this.startContinuousTask();
    }

    onDestroy(): void {
        console.info(TAG, 'BluetoothSendUIAbility onDestroy');
    }

    onForeground(): void {
        console.info(TAG, 'BluetoothSendUIAbility onForeground');
    }

    readyReceiveEvent() {
        try {
            console.info(TAG, 'readyReceiveEvent');
            this.subscriber = commonEventManager.createSubscriberSync(this.subscribeinfo);
            try {
                commonEventManager.subscribe(this.subscriber,
                (err: BusinessError, data: commonEventManager.CommonEventData) => {
                    if (err) {
                        console.error(TAG, 'subscribe failed, code is ${err.code}, message is ${err.message}');
                    } else {
                        this.handleReceivedEvent(data);
                    }
                });
            } catch (error) {
                let err: BusinessError = error as BusinessError;
                console.error(TAG, 'subscribe failed, code is ${err.code}, message is ${err.message}');
            }
        } catch (error) {
            let err: BusinessError = error as BusinessError;
            console.error(TAG, 'createSubscriber failed, code is ${err.code}, message is ${err.message}');
        }
    }

    handleReceivedEvent(data: commonEventManager.CommonEventData) {
        console.info(TAG, 'handleReceivedEvent' + data.event);
        switch (data.event) {
          case 'usual.event.bluetooth.OPP.SEND': {
              this.handleSend(data);
              break;
          }
          default: {
              break;
          }
        }
    }

    handleSend(data: commonEventManager.CommonEventData) {
        if (data.parameters == undefined) {
            console.error(TAG, 'data.parameters undefined');
            return;
        }
        this.filePaths = data.parameters?.['filePaths'] as Array<string>;
        let serverMac: string = data.parameters?.['serverMac'] as string;
        try {
            for (let i = 0; i < this.filePaths.length; i++) {
                let filePath = this.filePaths[i];
                console.log(TAG, 'deal uri: ' + filePath);
                let file = fs.openSync(filePath, fs.OpenMode.READ_ONLY);
                let stat: fs.Stat = fs.statSync(file.fd);
                let fileHolder: opp.FileHolder = {
                    filePath: filePath,
                    fileSize: stat.size,
                    fileFd: file.fd
                };
                this.filePathToFdMap.set(filePath, file.fd);
                this.fileHolders.push(fileHolder);
                console.info(TAG, 'fileHolder info: ' + JSON.stringify(fileHolder));
            }
            this.subscriberLiveViewNotification();
            this.oppProfile.on('transferStateChange', (data: opp.OppTransferInformation) => {
                if (data.status == 1) {
                    this.pullUpSendProgressNotification(data.currentBytes / data.totalBytes * 100,
                    this.getFileName(data.filePath));
                } else if (data.status == 2) {
                    console.info(TAG, 'transfer success');
                    this.oppProfile.off('transferStateChange');
                    fs.close(this.filePathToFdMap.get(data.filePath));
                }
            });
            console.log(TAG, 'oppProfile.sendFile');
            this.oppProfile.sendFile(serverMac, this.fileHolders);
        } catch (err) {
            console.error(TAG, 'onCreate err');
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
                    text: this.getFormatString($r('app.string.bluetooth_receive_finish_text'), successNum, failNum)
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
        });
    }

    async pullUpReceiveResultNotification(successNum: number, failNum: number) {
        console.info(TAG, 'pullUpReceiveResultNotification');
        let wantAgentObj: WantAgent;
        wantAgentObj = await this.getNotificationWantAgent('ohos.event.notification.BT.FINISH_SEND');
        await this.publishFinishNotification(wantAgentObj, successNum, failNum);
    }

    async publishTransProgessNotification(
        imagePixelMapButton: image.PixelMap, imagePixelMapCapsule: image.PixelMap, percent: number, name: string) {
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
            let drawableDescriptor2: DrawableDescriptor = this.context.resourceManager.getDrawableDescriptor($r('app.media.app_icon').id);
            imagePixelMapCapsule = drawableDescriptor2.getPixelMap();
        } catch (error) {
            let code = (error as BusinessError).code;
            let message = (error as BusinessError).message;
            console.error(TAG, 'getDrawableDescriptor failed, error code is ${code}, message is ${message}');
            return;
        }
        await this.publishTransProgessNotification(imagePixelMapButton, imagePixelMapCapsule, percent, name);
    }

    cancelSendProgressNotification(successOrNot: number) {
        console.info(TAG, 'cancelSendProgressNotification ready to cancel.');
        notificationManager.cancel(this.capsuleNotificationID).then(() => {
            console.info(TAG, 'Succeeded in canceling notification.');
            if (successOrNot === 1) {
                this.pullUpReceiveResultNotification(1, 0);
            }
            this.stopContinuousTask();
            this.context.terminateSelf();
        }).catch((err: BusinessError) => {
            console.error(TAG, 'failed to cancel notification. Code is ${err.code}, message is ${err.message}')
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

    getFormatString(resource: Resource, value1: number, value2: number): string {
        let result = this.context.resourceManager.getStringSync(resource.id);
        result = result.replace('%1$s', value1.toString());
        result = result.replace('%2$s', value2.toString());
        return result;
    }

    async startContinuousTask() {
        let wantAgentObj: WantAgent;
        wantAgentObj = await this.getNotificationWantAgent('ohos.event.notification.BT.BACK_RUNNING');
        backgroundTaskManager.startBackgroundRunning(this.context,
        backgroundTaskManager.BackgroundMode.BLUETOOTH_INTERACTION, wantAgentObj).then(() => {
            console.info(TAG, 'Succeeded in operationing startBackgroundRunning.');
        }).catch((err: BusinessError) => {
            console.error(TAG, 'Failed to operation startBackgroundRunning. Code is ${err.code}, message is ${err.message}');
        });
    }

    stopContinuousTask() {
        backgroundTaskManager.stopBackgroundRunning(this.context).then(() => {
            console.info(TAG, 'Succeeded in operationing stopBackgroundRunning.');
        }).catch((err: BusinessError) => {
            console.error(TAG, 'Failed to operation stopBackgroundRunning. Code is ${err.code}, message is ${err.message}');
        });
    }
}