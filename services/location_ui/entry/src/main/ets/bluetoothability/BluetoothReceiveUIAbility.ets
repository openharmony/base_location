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
import Notification from '@ohos.notification';
import Base from '@ohos.base';
import image from '@ohos.multimedia.image';
import fs from '@ohos.file.fs';
import wantAgent from '@ohos.app.ability.wantAgent';
import { WantAgent } from '@ohos.app.ability.wantAgent';
import commonEventManager from '@ohos.commonEventManager';
import resourceManager from '@ohos.resourceManager';
import opp from '@ohos.bluetooth.opp';
import type { BusinessError } from '@ohos.base';
import fileUri from '@ohos.file.fileuri';
import rpc from '@ohos.rpc';
import { DrawableDescriptor } from '@ohos.arkui.drawableDescriptor';
import backgroundTaskManager from '@ohos.backgroundTaskManager';

const TAG: string = '[BT_RECEIVE_SERVICE]==>'

export default class BluetoothReceiveUIAbility extends UIAbility {
    private subscriber: commonEventManager.CommonEventSubscriber | null = null;
    private subscribeinfo: commonEventManager.CommonEventSubscribeInfo = {
        events: [
            'ohos.event.notification.BT.TAP_ACCEPT',
            'ohos.event.notification.BT.TAP_REJECT',
            'ohos.event.notification.BT.FINISH_NOTIFICATION',
            'ohos.event.notification.BT.GET_URI',
            'usual.event.bluetooth.OPP.RECEIVE'
        ]
    }
    private cancelTransEvent: string = 'ohos.event.notification.BT.TAP_CANCEL';
    private capsuleNotificationID: number = 100;
    private timeInterval: number = 0;
    private uriPath: string = '';
    private fileName: string = '';
    private sandBoxUri: string = '';
    private fileFd: number = -1;
    private oppProfile = opp.createOppServerProfile();

    onCreate(want: Want): void {
        console.info(TAG, 'BluetoothReceiveUIAbility onCreate');
        this.readyReceiveEvent();
        this.startContinuousTask();
    }

    onDestroy(): void {
        console.info(TAG, 'BluetoothReceiveUIAbility onDestroy');
    }

    onForeground(): void {
        console.info(TAG, 'BluetoothReceiveUIAbility onForeground');
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
          case 'ohos.event.notification.BT.TAP_ACCEPT': {
              this.subscriberLiveViewNotification();
              this.startOPPReceiveServiceUIAbility();
              break;
          }
          case 'ohos.event.notification.BT.TAP_REJECT': {
              this.oppProfile.setIncomingFileConfirmation(false, -1);
              break;
          }
          case 'ohos.event.notification.BT.FINISH_NOTIFICATION': {
              fs.close(this.fileFd);
              this.oppProfile.setLastReceivedFileUri(this.uriPath);
              this.stopContinuousTask();
              this.context.terminateSelf();
              break;
          }
          case 'ohos.event.notification.BT.GET_URI': {
              this.handleBtUri(data);
              break;
          }
          case 'usual.event.bluetooth.OPP.RECEIVE': {
              this.handleReceive(data);
              break;
          }
          default: {
              break;
          }
        }
    }

    handleBtUri(data: commonEventManager.CommonEventData) {
        if (data.parameters == undefined) {
            console.error(TAG, 'data.parameters undefined');
            return;
        }

        console.info(TAG, 'handleBtUri ' + data.parameters.uri);
        this.oppProfile.on('transferStateChange', (data: opp.OppTransferInformation) => {
            if (data.status == 1) {
                this.pullUpSendProgressNotification(data.currentBytes / data.totalBytes * 100, this.fileName);
            } else if (data.status == 2) {
                console.info(TAG, 'transferStateChange ' + data.result);
                this.oppProfile.off('transferStateChange');
            }
        });
        
        this.uriPath = data.parameters.uri + '/' + this.fileName;
        const uri = new fileUri.FileUri(data.parameters.uri);
        this.sandBoxUri = uri.path + '/' + this.fileName;
        let file = fs.openSync(this.sandBoxUri, fs.OpenMode.CREATE | fs.OpenMode.READ_WRITE);
        this.fileFd = file.fd;
        console.info(TAG, 'handleBtUri sandBoxUri: ' + this.sandBoxUri + file.fd);
        this.oppProfile.setIncomingFileConfirmation(true, file.fd);
    }

    handleReceive(data: commonEventManager.CommonEventData) {
        if (data.parameters == undefined) {
            console.error(TAG, 'data.parameters undefined');
            return;
        }
        console.info(TAG, 'handleReceive fileName: ' + data.parameters.fileName);
        this.fileName = data.parameters.fileName;
        this.pullUpNotification(this.fileName);
    }

    async pullUpNotification(fileName: string) {
        console.info(TAG, 'pullUpNotification');
        let wantAgentObjAccept: WantAgent;
        let wantAgentObjReject: WantAgent;
        wantAgentObjAccept = await this.getNotificationWantAgent('ohos.event.notification.BT.TAP_ACCEPT');
        wantAgentObjReject = await this.getNotificationWantAgent('ohos.event.notification.BT.TAP_REJECT');
        await this.publishReceiveNotification(wantAgentObjReject, wantAgentObjAccept, fileName);
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

    async publishReceiveNotification(wantAgentObjReject: WantAgent, wantAgentObjAccept: WantAgent, fileName: string) {
        console.info(TAG, 'publishReceiveNotification for ' + fileName);
        let notificationRequest: notificationManager.NotificationRequest = {
            content: {
                notificationContentType: notificationManager.ContentType.NOTIFICATION_CONTENT_BASIC_TEXT,
                normal: {
                    title: this.context.resourceManager.getStringSync($r('app.string.bluetooth_receive_notification_title').id),
                    text: fileName,
                }
            },
            id: 1,
            notificationSlotType: notificationManager.SlotType.SERVICE_INFORMATION,
            actionButtons: [
                {
                    title: this.context.resourceManager.getStringSync($r('app.string.bluetooth_receive_notification_button_reject').id),
                    wantAgent: wantAgentObjReject
                },
                {
                    title: this.context.resourceManager.getStringSync($r('app.string.bluetooth_receive_notification_button_accept').id),
                    wantAgent: wantAgentObjAccept
                }
            ],
            tapDismissed: true,
        };
        notificationManager.publish(notificationRequest).then(() => {
            console.info(TAG, 'publishReceiveNotification success');
        }).catch((err: Base.BusinessError) => {
            console.error(TAG, 'publishReceiveNotification fail');
        });
    }

    async publishFinishNotification(wantAgentObj: WantAgent, successNum: number, failNum: number) {
        console.info(TAG, 'publishFinishNotification');
        let notificationRequest: notificationManager.NotificationRequest = {
            content: {
                notificationContentType: notificationManager.ContentType.NOTIFICATION_CONTENT_BASIC_TEXT,
                normal: {
                    title: this.context.resourceManager.getStringSync($r('app.string.bluetooth_receive_finish_title').id),
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
        }).catch((err: Base.BusinessError) => {
            console.error(TAG, 'publishFinishNotification fail');
        });
    }

    startOPPReceiveServiceUIAbility() {
        console.info(TAG, 'startOPPReceiveServiceUIAbility');
        AppStorage.setOrCreate('oppProfile', this.oppProfile);
        this.oppProfile.setIncomingFileConfirmation(true, -1);
    }

    async pullUpReceiveResultNotification(successNum: number, failNum: number) {
        console.info(TAG, 'pullUpReceiveResultNotification');
        let wantAgentObj: WantAgent;
        wantAgentObj = await this.getNotificationWantAgent('ohos.event.notification.BT.FINISH_NOTIFICATION');
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
                    text: this.context.resourceManager.getStringSync($r('app.string.bluetooth_transfer_receive_text').id) + name,
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
        }).catch((err: Base.BusinessError) => {
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
