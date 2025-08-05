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
import fs from '@ohos.file.fs';
import systemParameterEnhance from '@ohos.systemParameterEnhance';

const TAG: string = '[BT_SEND_SERVICE]==>'
const EVENT_TYPE_TRANSACTION: number = 6;
const BT_TRANSACTION_EVENT: string = 'usual.event.bluetooth.REPORT.TRANSACTION';

const RESULT_SUCCESS: number = 0;
const RESULT_ERROR_UNSUPPORTED_TYPE: number = 1;
const RESULT_ERROR_BAD_REQUEST: number = 2;
const RESULT_ERROR_NOT_ACCEPTABLE: number = 3;
const RESULT_ERROR_CANCELED: number = 4;
const RESULT_ERROR_CONNECTION_FAILED: number = 5;
const RESULT_ERROR_TRANSFER_FAILED: number = 6;
const RESULT_ERROR_UNKNOWN: number = 7;
const RESULT_ERROR_NO_MESSAGE: number = 8;
const STATUS_PENDING: number = 0;
const STATUS_RUNNING: number = 1;
const STATUS_FINISH: number = 2;

const btTransactionStatisticsType = {
    TRANSACTION_TYPE_OPP_SEND : 1,
    TRANSACTION_TYPE_OPP_RECEIVE : 2,
};

const btTransactionStatisticsResult = {
    TRANSACTION_RESULT_TOTAL : 1,
    TRANSACTION_RESULT_SUCCESS : 2,
    TRANSACTION_RESULT_FAIL : 3,
};

const btTransactionStatisticsSceneCode = {
    TRANSACTION_SCENECODE_NA : 0,
    TRANSACTION_SCENECODE_1 : 1,
    TRANSACTION_SCENECODE_2 : 2,
    TRANSACTION_SCENECODE_3 : 3,
    TRANSACTION_SCENECODE_4 : 4,
    TRANSACTION_SCENECODE_5 : 5,
    TRANSACTION_SCENECODE_6 : 6,
    TRANSACTION_SCENECODE_7 : 7,
    TRANSACTION_SCENECODE_8 : 8,
    TRANSACTION_SCENECODE_9 : 9,
    TRANSACTION_SCENECODE_10 : 10,
    TRANSACTION_SCENECODE_11 : 11,
    TRANSACTION_SCENECODE_12 : 12,
    TRANSACTION_SCENECODE_13 : 13,
    TRANSACTION_SCENECODE_14 : 14,
    TRANSACTION_SCENECODE_15 : 15,
    TRANSACTION_SCENECODE_16 : 16,
};

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
    private totalCount: number = 0;
    private currentCount: number = 0;
    private successCount: number = 0;
    private failedCount: number = 0;
    private isEnabledLive2: boolean = false;

    onCreate(want: Want): void {
        console.info(TAG, 'BluetoothSendUIAbility onCreate');
        this.timerIdCreate = setTimeout(() => {
            console.info(TAG, 'onCreate 51 seconds but nothing received');
            this.reportBtChrDataByResult(RESULT_ERROR_NO_MESSAGE);
            this.handleTerminate();
        }, 51000);
        this.timerIdCreated = true;
        this.subscribeTransferState();
        let fileName : string = want.parameters?.['fileName'] as string;
        if (fileName != undefined && fileName != '') {
            this.pullUpSendProgressNotification(0, fileName);
        }
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
        console.info(TAG, 'BluetoothSendUIAbility handleTerminate');
        this.stopContinuousTask();
        this.context.terminateSelf();
    }

    reportBtChrDataByResult(dataResult: number) {
        if (this.successCount + this.failedCount > this.totalCount) {
            return;
        }
        if (dataResult == RESULT_SUCCESS) {
            this.reportBtTransactionChr(btTransactionStatisticsResult.TRANSACTION_RESULT_SUCCESS,
                1, btTransactionStatisticsSceneCode.TRANSACTION_SCENECODE_NA, 0);
        } else if ((dataResult == RESULT_ERROR_UNSUPPORTED_TYPE ||
            dataResult == RESULT_ERROR_BAD_REQUEST) && this.totalCount > 0) {
            this.reportBtTransactionChr(btTransactionStatisticsResult.TRANSACTION_RESULT_FAIL,
                this.totalCount, dataResult, this.totalCount);
            this.reportBtTransactionChr(btTransactionStatisticsResult.TRANSACTION_RESULT_TOTAL,
                this.totalCount - 1, btTransactionStatisticsSceneCode.TRANSACTION_SCENECODE_NA, 0);
        } else if (dataResult == RESULT_ERROR_CANCELED ||
            dataResult == RESULT_ERROR_TRANSFER_FAILED) {
            this.reportBtTransactionChr(btTransactionStatisticsResult.TRANSACTION_RESULT_FAIL, 1, dataResult, 1);
        } else {
            this.reportBtTransactionChr(btTransactionStatisticsResult.TRANSACTION_RESULT_FAIL, 1, dataResult, 1);
        }
        this.reportBtTransactionChr(btTransactionStatisticsResult.TRANSACTION_RESULT_TOTAL,
            1, btTransactionStatisticsSceneCode.TRANSACTION_SCENECODE_NA, 0);
    }

    reportBtTransactionChr(result: number, resultCount: number, sceneCode: number, sceneCodeCount: number) {
        const options: commonEventManager.CommonEventPublishData = {
            code: 0,
            data: 'message',
            subscriberPermissions: [],
            isOrdered: true,
            isSticky: false,
            parameters: { 'transactionType': btTransactionStatisticsType.TRANSACTION_TYPE_OPP_SEND, 'result': result,
                'resultCount': resultCount, 'sceneCode': sceneCode, 'sceneCodeCount': sceneCodeCount}
        }
        commonEventManager.publish(BT_TRANSACTION_EVENT, options, (err) => {
            if (err) {
            console.info(TAG, 'get bt transaction event publish failed.' + JSON.stringify(err));
            } else {
            console.info(TAG, 'get bt transaction event publish success.');
            }
        })
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
        console.info(TAG, 'handleReceivedEvent: ' + data.event);
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
                this.totalCount = data.totalCount;
                console.info(TAG, 'data.status =  ' + data.status + ' data.currentCount = ' + data.currentCount);
                if (this.timerIdCreated) {
                    this.timerIdCreated = false;
                    clearTimeout(this.timerIdCreate);
                }
                if (data.status == STATUS_RUNNING) {
                    this.pullUpSendProgressNotification(data.currentBytes / data.totalBytes * 100,
                        this.getFileName(data.filePath));
                    this.currentCount = data.currentCount;
                } else if (data.status == STATUS_FINISH) {
                    data.result == RESULT_SUCCESS ? this.successCount++ : this.failedCount++;
                    this.reportBtChrDataByResult(data.result);
                    if (data.currentCount == data.totalCount || data.result == RESULT_ERROR_UNSUPPORTED_TYPE) {
                        this.currentCount++;
                        this.cancelSendProgressNotification();
                        this.pullUpReceiveResultNotification();
                        console.info(TAG, 'transferStateChange' + data.result);
                        this.oppProfile.off('transferStateChange');
                    }
                    console.info(TAG, 'transfer finished');
                    let dirPath = this.context.filesDir;
                    fs.rmdirSync(dirPath);
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

    async pullUpReceiveResultNotification() {
        console.info(TAG, 'pullUpNotification successCount' + this.successCount + 'failedCount' + this.failedCount);
        if (this.successCount + this.failedCount != this.totalCount && this.totalCount >= this.successCount) {
            this.failedCount = this.totalCount - this.successCount;
        }
        let wantAgentObj: WantAgent;
        wantAgentObj = await this.getNotificationWantAgent('ohos.event.notification.BT.FINISH_SEND');
        await this.publishFinishNotification(wantAgentObj, this.successCount, this.failedCount);
        this.handleTerminate();
    }

    async publishTransProgessNotification(imagePixelMapButton: image.PixelMap, imagePixelMapCapsule: image.PixelMap,
        wantAgentObjRemove: WantAgent, percent: number, name: string, currentCount: number, totalCount: number) {
        console.info(TAG, 'publishTransProgessNotification');
        let notificationRequest: notificationManager.NotificationRequest = {
            notificationSlotType: notificationManager.SlotType.LIVE_VIEW,
            id: this.capsuleNotificationID,
            content: {
                notificationContentType: notificationManager.ContentType.NOTIFICATION_CONTENT_SYSTEM_LIVE_VIEW,
                systemLiveView: {
                    title: this.getFormatNum($r('app.string.bluetooth_send_count_text'), currentCount, totalCount),
                    text: name,
                    typeCode: 8,
                    button: {
                        names: [this.cancelTransEvent],
                        iconsResource: [$r('app.media.public_cancel_filled')],
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
            if (percent == 100 && this.currentCount > this.totalCount) {
                this.cancelSendProgressNotification();
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
        this.isEnabledLive2 = systemParameterEnhance.getSync('persist.systemui.live2', 'false') == 'true';
        console.info(TAG, 'this.isEnabledLive2 = ' + this.isEnabledLive2);
        let imagePixelMapButton: image.PixelMap | undefined = undefined;
        let imagePixelMapCapsule: image.PixelMap | undefined = undefined;
        try {
            let drawableDescriptor1: DrawableDescriptor = this.context.resourceManager.getDrawableDescriptor($r('app.media.public_cancel_filled').id);
            imagePixelMapButton = drawableDescriptor1.getPixelMap();
            let drawableDescriptor2: DrawableDescriptor;
            if (!this.isEnabledLive2) {
                drawableDescriptor2 = 
                    this.context.resourceManager.getDrawableDescriptor($r('app.media.foreground').id);
            } else {
                drawableDescriptor2 =
                    this.context.resourceManager.getDrawableDescriptor($r('app.media.foregroundSmall').id);
            }
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
            percent, name, this.currentCount, this.totalCount);
    }

    cancelSendProgressNotification() {
        console.info(TAG, 'cancelSendProgressNotification ready to cancel.');
        notificationManager.cancel(this.capsuleNotificationID).then(() => {
            console.info(TAG, 'Succeeded in canceling notification.');
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
                        this.cancelSendProgressNotification();
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

    getFormatNum(resource: Resource, value1: number, value2: number): string {
        let result = this.context.resourceManager.getStringSync(resource.id);
        result = result.replace('%1$d', value1.toString());
        result = result.replace('%2$d', value2.toString());
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