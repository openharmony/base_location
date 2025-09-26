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
import systemParameterEnhance from '@ohos.systemParameterEnhance';
import PermissionUtils from 'common/PermissionUtils';

const TAG: string = '[BT_RECEIVE_SERVICE]==>'
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
const RESULT_SUCCESS_DISCONNECT: number = 9;

const STATUS_PENDING: number = 0;
const STATUS_RUNNING: number = 1;
const STATUS_FINISH: number = 2;
const OPP_TRANSFER_USER_ACCEPT: number = 0;
const OPP_TRANSFER_AUTO_ACCEPT: number = 1;
const UNKNOWN_VALUE: number = -1;

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
        ],
        publisherPermission: 'ohos.permission.MANAGE_BLUETOOTH'
    }
    private cancelTransEvent: string = 'ohos.event.notification.BT.TAP_CANCEL';
    private capsuleNotificationID: number = 100;
    private timeInterval: number = 0;
    private receiveDirectory: string = '';
    private fileName: string = '';
    private sandBoxUri: string = '';
    private oppProfile = opp.createOppServerProfile();
    private tapEventIsAllowed = true;
    private transfering: boolean = false;
    private timerIdCreate: number;
    private timerIdReceive: number;
    private fileNameToReceiveMap = new Map();
    private totalCount: number = 0;
    private currentCount: number = 0;
    private successCount: number = 0;
    private failedCount: number = 0;
    private isEnabledLive2: boolean = false;

    onCreate(want: Want): void {
        if (!PermissionUtils.checkBluetoothShareCallerBluetoothPermission(want)) {
            console.info(TAG, `caller not has permission`);
            return;
        }
        console.info(TAG, 'BluetoothReceiveUIAbility onCreate');
        this.readyReceiveEvent();
        this.startContinuousTask();
        this.timerIdCreate = setTimeout(() => {
            console.info(TAG, 'onCreate 1 second but nothing received');
            this.reportBtTransactionChr(btTransactionStatisticsResult.TRANSACTION_RESULT_FAIL,
                1, btTransactionStatisticsSceneCode.TRANSACTION_SCENECODE_1, 1);
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
        console.info(TAG, 'handleTerminate');
        this.stopContinuousTask();
        this.context.terminateSelf();
    }

    reportBtChrDataByResult(dataResult: number) {
        console.info(TAG, 'reportBtChrDataByResult dataResult ' + dataResult +
            ' totalCount' + this.totalCount + ' successCount' + this.successCount);
        if (dataResult == RESULT_SUCCESS) {
            this.reportBtTransactionChr(btTransactionStatisticsResult.TRANSACTION_RESULT_SUCCESS,
                1, btTransactionStatisticsSceneCode.TRANSACTION_SCENECODE_NA, 0);
        } else if ((dataResult == RESULT_ERROR_NOT_ACCEPTABLE ||
            dataResult == RESULT_ERROR_BAD_REQUEST) && this.totalCount > 0) {
            this.reportBtTransactionChr(btTransactionStatisticsResult.TRANSACTION_RESULT_FAIL,
                this.totalCount, dataResult, this.totalCount);
            this.reportBtTransactionChr(btTransactionStatisticsResult.TRANSACTION_RESULT_TOTAL,
                this.totalCount - 1, btTransactionStatisticsSceneCode.TRANSACTION_SCENECODE_NA, 0);
        } else if ((dataResult == RESULT_ERROR_CANCELED || dataResult == RESULT_ERROR_TRANSFER_FAILED) &&
            this.totalCount - this.successCount >= 1) {
            this.reportBtTransactionChr(btTransactionStatisticsResult.TRANSACTION_RESULT_FAIL,
                this.totalCount - this.successCount, dataResult, this.totalCount - this.successCount);
            this.reportBtTransactionChr(btTransactionStatisticsResult.TRANSACTION_RESULT_TOTAL,
                this.totalCount - this.successCount -1, btTransactionStatisticsSceneCode.TRANSACTION_SCENECODE_NA, 0);
        } else if (dataResult == RESULT_ERROR_CONNECTION_FAILED) {
            this.reportBtTransactionChr(btTransactionStatisticsResult.TRANSACTION_RESULT_FAIL, 1, dataResult, 1);
        }
        this.reportBtTransactionChr(btTransactionStatisticsResult.TRANSACTION_RESULT_TOTAL,
            1, btTransactionStatisticsSceneCode.TRANSACTION_SCENECODE_NA, 0);
    }

    reportBtTransactionChr(result: number, resultCount: number, sceneCode: number, sceneCodeCount: number) {
        const options: commonEventManager.CommonEventPublishData = {
            code: 0,
            data: 'message',
            subscriberPermissions: ['ohos.permission.MANAGE_BLUETOOTH'],
            isOrdered: true,
            isSticky: false,
            parameters: { 'transactionType': btTransactionStatisticsType.TRANSACTION_TYPE_OPP_RECEIVE, 'result': result,
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

    async readyReceiveEvent() {
        try {
            console.info(TAG, 'readyReceiveEvent');
            clearTimeout(this.timerIdCreate);
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
              console.info(TAG, 'OPP.TAP.ACCEPT tapEventIsAllowed is ' + this.tapEventIsAllowed);
              if (data.parameters == undefined) {
                  console.error(TAG, `data.parameters undefined`);
                  break;
              }
              let acceptType = data.parameters?.['acceptType'];
              console.info(TAG, 'acceptType is ' + acceptType);
              if (acceptType == OPP_TRANSFER_USER_ACCEPT && this.tapEventIsAllowed) {
                  clearTimeout(this.timerIdReceive);
                  this.currentCount = 1;
                  this.subscriberLiveViewNotification();
                  this.startOPPReceiveServiceUIAbility();
                  this.tapEventIsAllowed = false;
              } else if (acceptType == OPP_TRANSFER_AUTO_ACCEPT) {
                  this.fileName = data.parameters?.['fileName'];
                  this.currentCount = data.parameters?.['currentCount'];
                  this.totalCount = data.parameters?.['totalCount'];
                  this.startOPPReceiveServiceUIAbility();
              }
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
              this.reportBtTransactionChr(btTransactionStatisticsResult.TRANSACTION_RESULT_FAIL,
                  this.totalCount, btTransactionStatisticsSceneCode.TRANSACTION_SCENECODE_2, this.totalCount);
              this.reportBtTransactionChr(btTransactionStatisticsResult.TRANSACTION_RESULT_TOTAL,
                  this.totalCount, btTransactionStatisticsSceneCode.TRANSACTION_SCENECODE_NA, 0);
              this.pullUpReceiveResultNotification();
              this.handleTerminate();
              this.tapEventIsAllowed = false;
              break;
          }
          case 'usual.event.bluetooth.OPP.TAP.REMOVE': {
              console.info(TAG, 'tapEventIsAllowed is ' + this.tapEventIsAllowed);
              clearTimeout(this.timerIdReceive);
              this.transfering = false;
              this.oppProfile.setIncomingFileConfirmation(false, -1);
              this.reportBtTransactionChr(btTransactionStatisticsResult.TRANSACTION_RESULT_FAIL,
                  this.totalCount, btTransactionStatisticsSceneCode.TRANSACTION_SCENECODE_5, this.totalCount);
              this.reportBtTransactionChr(btTransactionStatisticsResult.TRANSACTION_RESULT_TOTAL,
                  this.totalCount, btTransactionStatisticsSceneCode.TRANSACTION_SCENECODE_NA, 0);
              this.pullUpReceiveResultNotification();
              this.handleTerminate();
              this.tapEventIsAllowed = true;
              break;
          }
          case 'ohos.event.notification.BT.LIVEVIEW_REMOVE': {
              this.tapEventIsAllowed = true;
              this.oppProfile.cancelTransfer();
              this.pullUpReceiveResultNotification();
              break;
          }
          case 'usual.event.bluetooth.OPP.FINISH.NOTIFICATION': {
              this.oppProfile.setLastReceivedFileUri(this.receiveDirectory);
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
              break;
          }
          case 'usual.event.bluetooth.OPP.RECEIVE': {
              clearTimeout(this.timerIdCreate);
              if (data.parameters == undefined) {
                  console.error(TAG, 'data.parameters undefined');
                  break;
              }
              this.fileName = data.parameters?.['fileName'];
              this.totalCount = data.parameters?.['totalCount'];
              if (this.fileNameToReceiveMap.has(this.fileName)) {
                  console.error(TAG, 'fileName is repeat, discard this OPP.RECEIVE');
                  break;
              }
              this.fileNameToReceiveMap.set(this.fileName, true);
              this.transfering = true;
              this.pullUpNotification(this.fileName);
              break;
          }
          default: {
              break;
          }
        }
    }

    async handleBtUri(data: commonEventManager.CommonEventData) {
        if (data.parameters == undefined) {
            console.error(TAG, 'data.parameters undefined');
            return;
        }

        this.oppProfile.off('transferStateChange');
        this.oppProfile.on('transferStateChange', (data: opp.OppTransferInformation) => {
            this.totalCount = data.totalCount;
            this.currentCount = data.currentCount;
            console.info(TAG, 'transferStateChange status' + data.status + ' result ' + data.result);
            if (data.status == STATUS_RUNNING) {
                this.pullUpSendProgressNotification(data.currentBytes / data.totalBytes * 100, this.fileName);
            } else if (data.status == STATUS_FINISH) {
                if (data.result != RESULT_SUCCESS_DISCONNECT) {
                    data.result == RESULT_SUCCESS ? this.successCount++ : this.failedCount++;
                    this.reportBtChrDataByResult(data.result);
                }
                if (data.result != RESULT_SUCCESS && data.filePath != undefined && data.filePath.trim().length > 0) {
                    this.tapEventIsAllowed = true;
                    console.info(TAG, `unlink file begin`);
                    const uri = new fileUri.FileUri(this.receiveDirectory);
                    let deleteSandBoxUri = uri.path;
                    fs.unlinkSync(deleteSandBoxUri);
                }
                if (data.currentCount == data.totalCount || data.result == RESULT_ERROR_CANCELED ||
                    data.result == RESULT_ERROR_TRANSFER_FAILED) {
                    this.tapEventIsAllowed = true;
                    this.cancelSendProgressNotification();
                    this.pullUpReceiveResultNotification();
                    this.oppProfile.off('transferStateChange');
                    console.info(TAG, 'transferStateChange' + data.result);
                }
            }
        });

        if ((data.parameters.uri == undefined) || (data.parameters.uri == null) ||
            (data.parameters.uri.length == 0)) {
            console.error(TAG, 'uri not right');
            return;
        }
        this.receiveDirectory = data.parameters.uri;
        try {
            const uri = new fileUri.FileUri(data.parameters.uri);
            if (!fs.accessSync(uri.path)) {
                console.error(TAG, 'uri path is not access');
                return;
            }
            if (data.parameters.fileType == 'files') {
                this.sandBoxUri = uri.path + '/' + this.fileName;
                this.receiveDirectory = this.receiveDirectory + '/' + this.fileName;
            } else {
                this.sandBoxUri = this.receiveDirectory;
            }
            let file = fs.openSync(this.sandBoxUri, fs.OpenMode.CREATE | fs.OpenMode.READ_WRITE);
            await this.oppProfile.setIncomingFileConfirmation(true, file.fd);
            fs.close(file.fd);
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
                this.reportBtChrDataByResult(RESULT_ERROR_NOT_ACCEPTABLE);
                this.pullUpReceiveResultNotification();
                this.handleTerminate();
            }, waitTime);
        }).catch((err: Base.BusinessError) => {
            console.error(TAG, 'publishReceiveNotification fail');
            this.handleTerminate();
        });
    }

    resetCount() {
        console.info(TAG, 'resetCount');
        this.totalCount = 0;
        this.currentCount = 0;
        this.successCount = 0;
        this.failedCount = 0;
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
            console.info(TAG, 'publishFinishNotification show success');
            this.resetCount();
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

    async pullUpReceiveResultNotification() {
        console.info(TAG, 'pullUpNotification successCount' + this.successCount +
            'failedCount' + this.failedCount + 'totalCount' + this.totalCount);
        if (this.successCount + this.failedCount != this.totalCount && this.totalCount >= this.successCount) {
            this.failedCount = this.totalCount - this.successCount;
        }
        let wantAgentObj: WantAgent;
        let wantAgentObjRemove: WantAgent;
        wantAgentObj = await this.getNotificationWantAgent('ohos.event.notification.BT.FINISH_NOTIFICATION');
        wantAgentObjRemove = await this.getNotificationWantAgent('ohos.event.notification.BT.FINISH_REMOVE');
        await this.publishFinishNotification(wantAgentObj, wantAgentObjRemove, this.successCount, this.failedCount);
    }

    async publishTransProgessNotification(imagePixelMapButton: image.PixelMap, imagePixelMapCapsule: image.PixelMap,
        wantAgentObjRemove: WantAgent, percent: number, name: string, currentCount: number, totalCount: number) {
        console.info(TAG, 'publishTransProgessNotification');
        let progressTitle = this.getFormatNum($r('app.string.bluetooth_receive_count_text'), currentCount, totalCount);
        let progressName = name;
        if (this.totalCount == UNKNOWN_VALUE) {
            progressTitle =
                this.context.resourceManager.getStringSync($r('app.string.bluetooth_transfer_notification_title').id);
            progressName =
                this.context.resourceManager.getStringSync($r('app.string.bluetooth_transfer_receive_text').id) + name;
        }
        let notificationRequest: notificationManager.NotificationRequest = {
            notificationSlotType: notificationManager.SlotType.LIVE_VIEW,
            id: this.capsuleNotificationID,
            content: {
                notificationContentType: notificationManager.ContentType.NOTIFICATION_CONTENT_SYSTEM_LIVE_VIEW,
                systemLiveView: {
                    title: progressTitle,
                    text: progressName,
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
            if (percent == 100 && this.currentCount == this.totalCount) {
                this.cancelSendProgressNotification();
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
        await this.publishTransProgessNotification(imagePixelMapButton, imagePixelMapCapsule,
            wantAgentObjRemove, percent, name, this.currentCount, this.totalCount);
    }

    cancelSendProgressNotification() {
        console.info(TAG, 'cancelSendProgressNotification ready to cancel.');
        notificationManager.cancel(this.capsuleNotificationID).then(() => {
            console.info(TAG, 'Succeeded in canceling notification');
        }).catch((err: BusinessError) => {
            console.error(TAG, `failed to cancel notification. Code is ${err.code}, message is ${err.message}`)
        });
        this.transfering = false;
        if (this.successCount == 0) {
            console.info(TAG, 'no success file, terminate it.');
            this.handleTerminate();
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
                        this.cancelSendProgressNotification();
                        this.pullUpReceiveResultNotification();
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
