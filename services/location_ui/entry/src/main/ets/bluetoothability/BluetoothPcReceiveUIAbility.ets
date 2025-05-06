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
            'usual.event.bluetooth.OPP.TAP.ACCEPT',
            'usual.event.bluetooth.OPP.TAP.REJECT',
            'usual.event.bluetooth.OPP.TAP.REMOVE',
            'ohos.event.notification.BT.LIVEVIEW_REMOVE',
            'usual.event.bluetooth.OPP.FINISH.NOTIFICATION',
            'usual.event.bluetooth.OPP.FINISH.REMOVE',
            'ohos.event.notification.BT.GET_URI',
            'usual.event.bluetooth.OPP.RECEIVE'
        ]
    }
    private cancelTransEvent: string = 'ohos.event.notification.BT.TAP_CANCEL';
    private capsuleNotificationID: number = 100;
    private timeInterval: number = 0;
    private receiveDirectory: string = '';
    private fileName: string = '';
    private sandBoxUri: string = '';
    private fileFd: number = -1;
    private oppProfile = opp.createOppServerProfile();
    private jumpUriIsAllowed = false;
    private tapEventIsAllowed = true;
    private transfering: boolean = false;
    private timerIdCreate: number;
    private timerIdReceive: number;
    private fileNameToReceiveMap = new Map();

    onCreate(want: Want): void {
        console.info(TAG, 'BluetoothReceiveUIAbility onCreate');
        this.readyReceiveEvent();
        this.startContinuousTask();
        this.timerIdCreate = setTimeout(() => {
            console.info(TAG, 'onCreate 1 second but nothing received');
            this.handleTerminate();
        }, 1000);
    }

    onDestroy(): void {
        console.info(TAG, 'BluetoothReceiveUIAbility onDestroy');
    }

    onForeground(): void {
        console.info(TAG, 'BluetoothReceiveUIAbility onForeground');
    }

    handleTerminate() {
        this.stopContinuousTask();
        commonEventManager.unsubscribe(this.subscriber);
        this.context.terminateSelf();
    }

    async readyReceiveEvent() {
        try {
            console.info(TAG, 'readyReceiveEvent');
            this.subscriber = commonEventManager.createSubscriberSync(this.subscribeinfo);
            try {
                await commonEventManager.subscribe(this.subscriber,
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
        console.info(TAG, 'handleReceivedEvent: ' + data.event);
        switch (data.event) {
          case 'usual.event.bluetooth.OPP.TAP.ACCEPT': {
              console.info(TAG, 'tapEventIsAllowed is ' + this.tapEventIsAllowed);
              if (!this.tapEventIsAllowed) {
                  break;
              }
              clearTimeout(this.timerIdReceive);
              this.subscriberLiveViewNotification();
              this.startOPPReceiveServiceUIAbility();
              this.tapEventIsAllowed = false;
              break;
          }
          case 'usual.event.bluetooth.OPP.TAP.REJECT': {
              console.info(TAG, 'tapEventIsAllowed is ' + this.tapEventIsAllowed);
              if (!this.tapEventIsAllowed) {
                  break;
              }
              clearTimeout(this.timerIdReceive);
              this.transfering = false;
              this.oppProfile.setIncomingFileConfirmation(false, -1);
              this.pullUpReceiveResultNotification(0, 1);
              this.handleTerminate();
              this.tapEventIsAllowed = false;
              break;
          }
          case 'usual.event.bluetooth.OPP.TAP.REMOVE': {
              console.info(TAG, 'tapEventIsAllowed is ' + this.tapEventIsAllowed);
              clearTimeout(this.timerIdReceive);
              this.transfering = false;
              this.oppProfile.setIncomingFileConfirmation(false, -1);
              this.pullUpReceiveResultNotification(0, 1);
              this.handleTerminate();
              this.tapEventIsAllowed = true;
              break;
          }
          case 'ohos.event.notification.BT.LIVEVIEW_REMOVE': {
              this.tapEventIsAllowed = true;
              this.oppProfile.cancelTransfer();
              this.handleTerminate();
              break;
          }
          case 'usual.event.bluetooth.OPP.FINISH.NOTIFICATION': {
              console.info(TAG, 'jumpUriIsAllowed is ' + this.jumpUriIsAllowed);
              if (this.jumpUriIsAllowed) {
                  this.oppProfile.setLastReceivedFileUri(this.receiveDirectory);
              }
              setTimeout(() => {
                  if (this.transfering) {
                      console.info(TAG, 'still transfering, do not terminate');
                      return;
                  }
                  console.info(TAG, 'transfer finished, terminateSelf');
                  this.handleTerminate();
              }, 100);
              break;
          }
          case 'usual.event.bluetooth.OPP.FINISH.REMOVE': {
              setTimeout(() => {
                  if (this.transfering) {
                      console.info(TAG, 'still transfering, do not terminate');
                      return;
                  }
                  console.info(TAG, 'transfer finished, terminateSelf');
                  this.handleTerminate();
              }, 100);
              break;
          }
          case 'ohos.event.notification.BT.GET_URI': {
              this.handleBtUri(data);
              this.jumpUriIsAllowed = true;
              break;
          }
          case 'usual.event.bluetooth.OPP.RECEIVE': {
              clearTimeout(this.timerIdCreate);
              if (data.parameters == undefined) {
                  console.error(TAG, 'data.parameters undefined');
                  break;
              }
              this.fileName = data.parameters?.['fileName'];
              if (this.fileNameToReceiveMap.has(this.fileName)) {
                  console.error(TAG, 'fileName is repeat, discard this OPP.RECEIVE');
                  break;
              }
              this.fileNameToReceiveMap.set(this.fileName, true);
              this.transfering = true;
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

        this.oppProfile.on('transferStateChange', (data: opp.OppTransferInformation) => {
            if (data.status == 1) {
                this.pullUpSendProgressNotification(data.currentBytes / data.totalBytes * 100, this.fileName);
            } else if (data.status == 2) {
                this.tapEventIsAllowed = true;
                if (data.result != 0) {
                    console.info(TAG, 'receive fail');
                    this.cancelSendProgressNotification(0);
                    this.pullUpReceiveResultNotification(0, 1);
                }
                console.info(TAG, 'transferStateChange ' + data.result);
                this.oppProfile.off('transferStateChange');
            }
        });

        this.receiveDirectory = data.parameters.uri;
        if ((data.parameters.uri === undefined) || (data.parameters.uri === null) ||
            (data.parameters.uri.length === 0)) {
            console.error(TAG, 'uri not right');
            return;
        }
        try {
            const uri = new fileUri.FileUri(data.parameters.uri);
            this.sandBoxUri = uri.path + '/' + this.fileName;
            let file = fs.openSync(this.sandBoxUri, fs.OpenMode.CREATE | fs.OpenMode.READ_WRITE);
            this.fileFd = file.fd;
            this.oppProfile.setIncomingFileConfirmation(true, file.fd);
        } catch (err) {
            console.error(TAG, `openFd failed, code: ${err?.code} message:${err?.message}`);
            return;
        }
    }

    handleReceive(data: commonEventManager.CommonEventData) {
        this.pullUpNotification(this.fileName);
    }

    async pullUpNotification(fileName: string) {
        console.info(TAG, 'pullUpNotification');
        let wantAgentObjAccept: WantAgent;
        let wantAgentObjReject: WantAgent;
        let wantAgentObjRemove: WantAgent;
        wantAgentObjAccept = await this.getNotificationWantAgent('ohos.event.notification.BT.TAP_ACCEPT');
        wantAgentObjReject = await this.getNotificationWantAgent('ohos.event.notification.BT.TAP_REJECT');
        wantAgentObjRemove = await this.getNotificationWantAgent('ohos.event.notification.BT.TAP_REMOVE');
        await this.publishReceiveNotification(wantAgentObjReject, wantAgentObjAccept, wantAgentObjRemove, fileName);
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

    async publishReceiveNotification(wantAgentObjReject: WantAgent, wantAgentObjAccept: WantAgent,
        wantAgentObjRemove: WantAgent, fileName: string) {
        let waitTime: number = 50 * 1000;
        let timeout: number = new Date().getTime() + waitTime;
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
            autoDeletedTime: timeout,
            removalWantAgent: wantAgentObjRemove
        };
        notificationManager.publish(notificationRequest).then(() => {
            console.info(TAG, 'publishReceiveNotification success');
            this.timerIdReceive = setTimeout(() => {
                console.info(TAG, 'receive notification 50 seconds but no tap');
                this.transfering = false;
                this.oppProfile.setIncomingFileConfirmation(false, -1);
                this.pullUpReceiveResultNotification(0, 1);
                this.handleTerminate();
            }, waitTime);
        }).catch((err: Base.BusinessError) => {
            console.error(TAG, 'publishReceiveNotification fail');
            this.handleTerminate();
        });
    }

    async publishFinishNotification(wantAgentObj: WantAgent, wantAgentObjRemove: WantAgent,
        successNum: number, failNum: number) {
        console.info(TAG, 'publishFinishNotification');
        let notificationRequest: notificationManager.NotificationRequest = {
            content: {
                notificationContentType: notificationManager.ContentType.NOTIFICATION_CONTENT_BASIC_TEXT,
                normal: {
                    title: this.context.resourceManager.getStringSync($r('app.string.bluetooth_receive_finish_title').id),
                    text: this.getFormatString($r('app.string.bluetooth_receive_finish_text'),
                        this.getFormatPlural($r('app.plural.bluetooth_receive_finish_success_text', successNum, successNum), successNum),
                        this.getFormatPlural($r('app.plural.bluetooth_receive_finish_fail_text', failNum, failNum), failNum))
                }
            },
            id: 2,
            notificationSlotType: notificationManager.SlotType.SERVICE_INFORMATION,
            wantAgent: wantAgentObj,
            tapDismissed: true,
            removalWantAgent: wantAgentObjRemove
        };
        notificationManager.publish(notificationRequest).then(() => {
            console.info(TAG, 'publishFinishNotification success');
        }).catch((err: Base.BusinessError) => {
            console.error(TAG, 'publishFinishNotification fail');
            this.handleTerminate();
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
        let wantAgentObjRemove: WantAgent;
        wantAgentObj = await this.getNotificationWantAgent('ohos.event.notification.BT.FINISH_NOTIFICATION');
        wantAgentObjRemove = await this.getNotificationWantAgent('ohos.event.notification.BT.FINISH_REMOVE');
        await this.publishFinishNotification(wantAgentObj, wantAgentObjRemove, successNum, failNum);
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
            removalWantAgent: wantAgentObjRemove
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

    async createRecevieProgressNotification(name: string, progress: number) {
        let notificationRequest: notificationManager.NotificationRequest = {
            notificationSlotType: notificationManager.SlotType.SOCIAL_COMMUNICATION,
            id: this.capsuleNotificationID
            template: {
                name: 'downloadTemplate',
                data: {
                    title: this.context.resourceManager.getStringSync($r('app.string.bluetooth_transfer_notification_title').id),
                    fileName: this.context.resourceManager.getStringSync($r('app.string.bluetooth_transfer_receive_text').id) + name,
                    progressValue: progress,
                    progressMaxValue: 100
                }
            },
            content: {
                notificationContentType: notificationManager.ContentType.NOTIFICATION_CONTENT_BASIC_TEXT,
                normal: {
                    title: this.context.resourceManager.getStringSync($r('app.string.bluetooth_transfer_notification_title').id),
                    text: this.context.resourceManager.getStringSync($r('app.string.bluetooth_transfer_receive_text').id) + name
                }
            },
            extraInfo: {
                hw_keep_headsup_sticky: true,
                hw_heads_up_enable: true
            }
        };

        notificationManager.publish(notificationRequest).then(() => {
            console.info(TAG, 'createRecevieProgressNotification success');
            if (progress === 100) {
                this.cancelSendProgressNotification(1);
            }
        }).catch((err: Base.BusinessError) => {
            console.error(TAG, 'createRecevieProgressNotification fail');
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

        await this.createRecevieProgressNotification(name, percent);
    }

    cancelSendProgressNotification(successOrNot: number) {
        console.info(TAG, 'cancelSendProgressNotification ready to cancel.');
        notificationManager.cancel(this.capsuleNotificationID).then(() => {
            console.info(TAG, 'Succeeded in canceling notification flag is ' + successOrNot);
            if (successOrNot === 1) {
                this.pullUpReceiveResultNotification(1, 0);
            } else if (successOrNot === 0) {
                this.handleTerminate();
            }
        }).catch((err: BusinessError) => {
            console.error(TAG, `failed to cancel notification. Code is ${err.code}, message is ${err.message}`)
            this.handleTerminate();
        });
        this.transfering = false;
        if (this.fileFd != -1) {
            fs.close(this.fileFd);
            this.fileFd = -1;
        }
    }

    subscriberLiveViewNotification(): void {
        let subscriber: notificationManager.SystemLiveViewSubscriber = {
            onResponse: (id: number, option: notificationManager.ButtonOptions) => {
                switch (option.buttonName) {
                    case this.cancelTransEvent: {
                        console.info(TAG, 'cancel transfer.');
                        this.tapEventIsAllowed = true;
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
