/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef POI_INFO_CALLBACK_NAPI_H
#define POI_INFO_CALLBACK_NAPI_H

#include "iremote_stub.h"
#include "i_poi_info_callback.h"
#include "constant_definition.h"

namespace OHOS {
namespace Location {
class PoiInfoCallbackNapi : public IRemoteStub<IPoiInfoCallback> {
public:
    PoiInfoCallbackNapi();
    ~PoiInfoCallbackNapi();
    int32_t OnRemoteRequest(uint32_t code,
        MessageParcel &data, MessageParcel &reply, MessageOption &option) override;
    void OnPoiInfoChange(std::shared_ptr<PoiInfo> &results) override;
    void OnErrorReport(const std::string errorCode) override;
    void Wait();
    std::shared_ptr<PoiInfo> GetResult();
    ErrCode GetErrorCode();
    Poi ParsePoiInfoFromJson(std::string &poiJson);
private:
    bool ready_ = false;
    ErrCode errorCode_ = ERRCODE_SUCCESS;
    std::shared_ptr<PoiInfo> result_ = nullptr;
    std::mutex mutex_;
    std::condition_variable condition_;
};
} // namespace Location
} // namespace OHOS
#endif // POI_INFO_CALLBACK_NAPI_H