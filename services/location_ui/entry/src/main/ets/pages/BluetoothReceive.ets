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
import photoAccessHelper from '@ohos.file.photoAccessHelper';

let TAG = '[BT_RECEIVE_PAGE]===>';
const BT_NI_GET_URI_EVENT: string = 'ohos.event.notification.BT.GET_URI';

@Entry
@Component
struct btCreateFilePickerPage {

  /**
   * 文件类型
   */
  private static readonly FILE_TYPE_FILES: string = 'files';
  private static readonly FILE_TYPE_IMAGE: string = 'image';
  private static readonly FILE_TYPE_VIDEO: string = 'video';

  private context = getContext(this) as common.UIAbilityContext;
  private phAccessHelper = photoAccessHelper.getPhotoAccessHelper(getContext(this));
  private extension: string = '';
  private fileName: string = '';
  private fileUri: string = '';
  private fileType: string = '';
  private removeFileUri: string = '';


  aboutToAppear() {
      console.info(TAG, `aboutToAppear`);
      this.removeFileUri = AppStorage.Get('removeFileUri') as string;
      if (this.removeFileUri != undefined && this.removeFileUri.length > 1) {
          AppStorage.Delete('removeFileUri');
          this.removeAssets();
      } else {
          this.dealFileReceiveUri();
      }
  }

  /**
   * 删除媒体库资源
   */
  private async removeAssets() {
      try {
          let photoUris: string[] = [];
          photoUris.push(this.removeFileUri);
          await photoAccessHelper.MediaAssetChangeRequest.deleteLocalAssetsPermanentlyWithUri(this.context, photoUris);
          console.info('removeAssets successfully');
      } catch (error) {
          let err: BusinessError = error as BusinessError;
          console.error(TAG, `unlink failed, code is ${err.code}, message is ${err.message}`);
      }
      this.context.terminateSelf();
  }

  /**
   * 处理接收的文件URI生成、发布流程
   */
  private async dealFileReceiveUri() {
      try {
          await this.generateFileUri();
          this.publishFileUriEvent();
      } catch (error) {
          let err: BusinessError = error as BusinessError;
          console.error(TAG, `dealFileReceiveUri failed, code is ${err.code}, message is ${err.message}`);
      }
      this.context.terminateSelf();
  }

  /**
   * 根据不同的文件类型生成不同的文件路径
   */
  private async generateFileUri() {
      try {
          await this.getFileType();
          switch (this.fileType) {
            case btCreateFilePickerPage.FILE_TYPE_FILES:
                await this.generateFileManagerUri();
                break;
            case btCreateFilePickerPage.FILE_TYPE_IMAGE:
                await this.generatePhotoUri(photoAccessHelper.PhotoType.IMAGE);
                break;
            case btCreateFilePickerPage.FILE_TYPE_VIDEO:
                await this.generatePhotoUri(photoAccessHelper.PhotoType.VIDEO);
                break;
            default:
                console.error(TAG, `invalid fileType`);
                break;
          }
      } catch (error) {
          let err: BusinessError = error as BusinessError;
          console.error(TAG, `generateFileUri failed, code is ${err.code}, message is ${err.message}`);
          this.context.terminateSelf();
      }
  }

  /**
   * 发布获取文件路径成功事件
   */
  private publishFileUriEvent() {
      const options: commonEventManager.CommonEventPublishData = {
          code: 0,
          data: 'message',
          subscriberPermissions: ['ohos.permission.MANAGE_BLUETOOTH'],
          isOrdered: false,
          isSticky: false,
          parameters: { 'uri': this.fileUri, 'fileType': this.fileType }
      }
      commonEventManager.publish(BT_NI_GET_URI_EVENT, options, (err: BusinessError) => {
          if (err) {
              console.info(TAG, `get file URI event publish failed, code is ${err.code}, message is ${err.message}`);
          } else {
              console.info(TAG, `get file URI event publish success.`);
          }
      });
  }

  /**
   * 获取文件后缀名
   */
  private getFileExtension() : string {
      this.fileName = AppStorage.Get('fileName') as string;
      AppStorage.Delete('fileName');
      if (this.fileName == undefined || this.fileName.length < 1 ||
          !this.fileName.includes('.') || this.fileName.charAt(this.fileName.length - 1) == '.') {
        console.error(TAG, `undefined fileName`);
        return '';
      }
      return this.fileName.slice(this.fileName.lastIndexOf('.') + 1).toLowerCase();
  }

  /**
   * 获取文件类型:  IMAGE or VIDEO or DOC
   */
  private async getFileType() {
      try {
        console.log(TAG, `getFileType start`);
        this.extension = this.getFileExtension();
        let imageList : string[] =
            await this.phAccessHelper.getSupportedPhotoFormats(photoAccessHelper.PhotoType.IMAGE);
        if (imageList.includes(this.extension)) {
            console.log(TAG, `file extension is PhotoType.IMAGE`);
            this.fileType = btCreateFilePickerPage.FILE_TYPE_IMAGE;
            return;
        }
        let videoList : string[] =
            await this.phAccessHelper.getSupportedPhotoFormats(photoAccessHelper.PhotoType.VIDEO);
        if (videoList.includes(this.extension)) {
            console.log(TAG, `file extension is PhotoType.VIDEO`);
            this.fileType = btCreateFilePickerPage.FILE_TYPE_VIDEO;
        } else {
            this.fileType = btCreateFilePickerPage.FILE_TYPE_FILES;
        }
      } catch (error) {
          let err: BusinessError = error as BusinessError;
          console.error(TAG, `getFileType failed, code is ${err.code}, message is ${err.message}`);
          this.context.terminateSelf();
      }
  }


  /**
   * 生成图库文件路径
   */
  private async generatePhotoUri(photoType : photoAccessHelper.PhotoType) {
      try {
        console.log(TAG, `createAsset start`);
        this.fileUri = await this.phAccessHelper.createAsset(photoType, this.extension);
      } catch (error) {
          let err: BusinessError = error as BusinessError;
          console.error(TAG, `createAsset failed, code is ${err.code}, message is ${err.message}`);
          this.context.terminateSelf();
      }
  }


  /**
   * 生成文件管理文件路径
   */
  private async generateFileManagerUri() {
      try {
          let documentSaveOptions = new picker.DocumentSaveOptions();
          documentSaveOptions.pickerMode = picker.DocumentPickerMode.DOWNLOAD;
          const documentPicker = new picker.DocumentViewPicker();
          let documentSaveResult: string[] = await documentPicker.save(documentSaveOptions);
          console.info(TAG, `opp documentViewPicker.save to file succeed` + documentSaveResult.length);
          if (documentSaveResult.length > 0) {
              this.fileUri = documentSaveResult[0];
          }
      } catch (error) {
          let err: BusinessError = error as BusinessError;
          console.error(TAG, `generateFileManagerUri failed, code is ${err.code}, message is ${err.message}`);
          this.context.terminateSelf();
      }
  }

  aboutToDisappear() {
      let session = AppStorage.get<UIExtensionContentSession>('ConfirmSession');
      if (session) {
          console.info(TAG, `session.terminateSelf`);
          session.terminateSelf();
      }
  }

  onCancel() {
      console.info(TAG, `onCancel is called`)
  }

  build() {
  }
}