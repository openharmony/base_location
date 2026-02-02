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
 
#include <string>
#include "location_hiappevent.h"
#include "location_log.h"
#include "common_utils.h"
#ifdef LOCATION_HIAPPEVENT_ENABLE
#include "app_event.h"
#include "app_event_processor_mgr.h"
#endif
 
namespace OHOS {
namespace Location {
LocationHiAppEvent::LocationHiAppEvent()
{
    LBSLOGI(LOCATION_HIAPPEVENT, "ability constructed.");
}
 
LocationHiAppEvent::~LocationHiAppEvent()
{
    LBSLOGI(LOCATION_HIAPPEVENT, "ability destructed.");
}
 
LocationHiAppEvent* LocationHiAppEvent::GetInstance()
{
    static LocationHiAppEvent data;
    return &data;
}
 
void LocationHiAppEvent::AddProcessor()
{
#ifdef LOCATION_HIAPPEVENT_ENABLE
    std::unique_lock<std::mutex> lock(processorIdMutex_);
    if (processorId_ != -1) {
        return;
    }
    HiviewDFX::HiAppEvent::ReportConfig config;
    config.name = "ha_app_event";
    std::string appId = "";
    if (!CommonUtils::GetConfigFromJson(KEY_KIT_REPORT_APPID, appId)) {
        LBSLOGI(LOCATION_HIAPPEVENT, "GetConfigFromJson error appId:%{public}s.", appId.c_str());
        return;
    }
    config.appId = appId;
    config.routeInfo = "AUTO";
    config.triggerCond.timeout = HA_CONFIG_TIMEOUT;
    config.triggerCond.row = HA_CONFIG_ROW;
    config.eventConfigs.clear();
    {
        OHOS::HiviewDFX::HiAppEvent::EventConfig event1;
        event1.domain = "api_diagnostic";
        event1.name = "api_exec_end";
        event1.isRealTime = false;
        config.eventConfigs.push_back(event1);
    }
    {
        OHOS::HiviewDFX::HiAppEvent::EventConfig event2;
        event2.domain = "api_diagnostic";
        event2.name = "api_called_stat";
        event2.isRealTime = true;
        config.eventConfigs.push_back(event2);
    }
    {
        OHOS::HiviewDFX::HiAppEvent::EventConfig event3;
        event3.domain = "api_diagnostic";
        event3.name = "api_called_stat_cnt";
        event3.isRealTime = true;
        config.eventConfigs.push_back(event3);
    }
    processorId_ = HiviewDFX::HiAppEvent::AppEventProcessorMgr::AddProcessor(config);
#endif
}
 
void LocationHiAppEvent::WriteEndEvent(const int64_t beginTime, const int result, const int errCode,
    const std::string& apiName)
{
#ifdef LOCATION_HIAPPEVENT_ENABLE
    AddProcessor();
    // 非应用不支持打点
    if (processorId_ == HA_NOT_SUPPORT_PROCESS_ID) {
        LBSLOGI(LOCATION_HIAPPEVENT, "not support");
        return;
    }
    HiviewDFX::HiAppEvent::Event event("api_diagnostic", "api_exec_end", OHOS::HiviewDFX::HiAppEvent::BEHAVIOR);
    event.AddParam("trans_id", CommonUtils::GenerateUuid());
    event.AddParam("api_name", apiName);
    event.AddParam("sdk_name", std::string("LocationKit"));
    event.AddParam("begin_time", beginTime);
    event.AddParam("end_time", CommonUtils::GetCurrentTimeMilSec());
    event.AddParam("result", result);
    event.AddParam("error_code", errCode);
    OHOS::HiviewDFX::HiAppEvent::Write(event);
    LBSLOGD(LOCATION_HIAPPEVENT, "WriteEndEvent end, apiName:%{public}s", apiName.c_str());
#endif
}

void LocationHiAppEvent::CountEventTimeAndNum(const int64_t startTime, const int errCode)
{
#ifdef LOCATION_HIAPPEVENT_ENABLE
    int64_t currentTimeMilSec = CommonUtils::GetCurrentTimeMilSec();
    // 单词接口耗时统计
    runTime_.push_back(currentTimeMilSec - startTime);
    // 多次调用总耗时
    sumTime_ += currentTimeMilSec - startTime;
    if (beginTime_ == 0) {
        beginTime_ = startTime;
    }
    // 借口调用结果统计
    if (errCode == 0) {
        succCount_++;
    } else {
        // 统计异常次数
        errCodes_[errCode] = !errCodes_[errCode] ? 1 : errCodes_[errCode] + 1;
    }
    // 未上报或上报时间超过1min，触发上报
    if (lastReportTime_ == 0 || currentTimeMilSec - lastReportTime_ >= HA_REPORT_INTERVAL) {
        WriteCallStatusEvent();
        lastReportTime_ = currentTimeMilSec;
    }
#endif
}

void LocationHiAppEvent::WriteCallStatusEvent()
{
#ifdef LOCATION_HIAPPEVENT_ENABLE
    AddProcessor();
    // 非应用不支持打点
    if (processorId_ == HA_NOT_SUPPORT_PROCESS_ID) {
        LBSLOGI(LOCATION_HIAPPEVENT, "not support");
        return;
    }
    // 更新错误码列表
    for (auto it = errCodes_.begin(); it != errCodes_.end(); it++) {
        errCodeType_.push_back(std::to_string(it->first));
        errCodeNum_.push_back(it->second);
    }
    HiviewDFX::HiAppEvent::Event event("api_diagnostic", "api_exec_end", OHOS::HiviewDFX::HiAppEvent::BEHAVIOR);
    event.AddParam("api_name", apiName);
    event.AddParam("sdk_name", std::string("LocationKit"));
    event.AddParam("begin_time", beginTime_);
    event.AddParam("call_times", static_cast<int32_t>(runTime_.size()));
    event.AddParam("success_times", succCount_);
    event.AddParam("max_cost_times", *max_element(runTime_.begin(), runTime_.end()));
    event.AddParam("min_cost_times", *min_element(runTime_.begin(), runTime_.end()));
    event.AddParam("total_cost_time", sumTime_);
    event.AddParam("error_code_types", errCodeType_);
    event.AddParam("error_code_num", errCodeNum_);
    OHOS::HiviewDFX::HiAppEvent::Write(event);
    LBSLOGD(LOCATION_HIAPPEVENT, "WriteCallStatusEvent end, callTimes:%{public}zu", runTime_.size());
#endif
}
}  // namespace Location
}  // namespace OHOS