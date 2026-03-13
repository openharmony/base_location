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

void LocationHiAppEvent::CountEventTimeAndNum(const std::string apiName, const int64_t startTime, const int errCode)
{
#ifdef LOCATION_HIAPPEVENT_ENABLE
    int64_t currentTimeMilSec = CommonUtils::GetCurrentTimeMilSec();
    if (currentTimeMilSec - startTime < 0) {
        return;
    }
    HaEventInfo eventInfo;
    std::unique_lock<std::mutex> lock(haEventInfoMapMutex_);
    auto it = haEventInfoMap_.find(apiName);
    if (it != haEventInfoMap_.end()) {
        eventInfo = it->second;
    }
    // 单次接口耗时统计
    eventInfo.runTime.push_back(currentTimeMilSec - startTime);
    // 多次调用总耗时
    eventInfo.sumTime += currentTimeMilSec - startTime;
    if (eventInfo.beginTime == 0) {
        eventInfo.beginTime = startTime;
    }
    // 接口调用结果统计
    if (errCode == 0) {
        eventInfo.succCount++;
    } else {
        // 统计异常次数
        eventInfo.errCodes[errCode] = !eventInfo.errCodes[errCode] ? 1 : eventInfo.errCodes[errCode] + 1;
    }
    haEventInfoMap_[apiName] = eventInfo;
    // 未上报或上报时间超过1min，触发上报
    if (eventInfo.lastReportTime == 0 || currentTimeMilSec - eventInfo.lastReportTime >= HA_REPORT_INTERVAL) {
        WriteCallStatusEvent(apiName, eventInfo);
        eventInfo.lastReportTime = currentTimeMilSec;
        // 重置数据
        std::vector<int64_t>().swap(eventInfo.runTime);
        eventInfo.sumTime = 0;
        eventInfo.succCount = 0;
        std::vector<std::string>().swap(eventInfo.errCodeType);
        std::vector<int64_t>().swap(eventInfo.errCodeNum);
        std::map<int, int64_t>().swap(eventInfo.errCodes);
        eventInfo.beginTime = 0;
        haEventInfoMap_[apiName] = eventInfo;
    }
#endif
}

void LocationHiAppEvent::WriteCallStatusEvent(const std::string apiName, HaEventInfo eventInfo)
{
#ifdef LOCATION_HIAPPEVENT_ENABLE
    AddProcessor();
    // 非应用不支持打点
    if (processorId_ == HA_NOT_SUPPORT_PROCESS_ID) {
        LBSLOGI(LOCATION_HIAPPEVENT, "not support");
        return;
    }
    // 更新错误码列表
    for (auto it = eventInfo.errCodes.begin(); it != eventInfo.errCodes.end(); it++) {
        eventInfo.errCodeType.push_back(std::to_string(it->first));
        eventInfo.errCodeNum.push_back(it->second);
    }
    HiviewDFX::HiAppEvent::Event event("api_diagnostic", "api_called_stat", OHOS::HiviewDFX::HiAppEvent::BEHAVIOR);
    event.AddParam("api_name", apiName);
    event.AddParam("sdk_name", std::string("LocationKit"));
    event.AddParam("begin_time", eventInfo.beginTime);
    event.AddParam("call_times", static_cast<int32_t>(eventInfo.runTime.size()));
    event.AddParam("success_times", eventInfo.succCount);
    event.AddParam("max_cost_time", *max_element(eventInfo.runTime.begin(), eventInfo.runTime.end()));
    event.AddParam("min_cost_time", *min_element(eventInfo.runTime.begin(), eventInfo.runTime.end()));
    event.AddParam("total_cost_time", eventInfo.sumTime);
    event.AddParam("error_code_types", eventInfo.errCodeType);
    event.AddParam("error_code_num", eventInfo.errCodeNum);
    OHOS::HiviewDFX::HiAppEvent::Write(event);
    LBSLOGD(LOCATION_HIAPPEVENT, "WriteCallStatusEvent end, apiName:%{public}s, callTimes:%{public}zu",
        apiName.c_str(), eventInfo.runTime.size());
#endif
}
}  // namespace Location
}  // namespace OHOS