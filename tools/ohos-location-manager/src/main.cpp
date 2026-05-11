/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
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

#include <iostream>
#include <string>
#include <cstring>
#include <memory>
#include <thread>
#include <chrono>
#include <mutex>
#include <unordered_map>
#include <sstream>
#include <iomanip>
#include <stdio>
#include <iremote_broker.h>
#include "locator_impl.h"
#include "location_log.h"
#include "constant_definition.h"
#include "locator_callback_host.h"
#include "ipc_skeleton.h"
#include "privacy_kit.h"

using json = nlohmann::json;
namespace OHOS {
namespace Location {
const int PARAM_NUM_TWO = 2;
const int64_t MILLI_PER_SECS = 1000L;
namespace {
struct Command {
    const char* name;
    const char* description;
    int (*handler)(int argc, char** argv);

    Command(const char* n, const char* d, int (*h)(int, char**)) : name(n), description(d), handler(h) {}
};

static std::unordered_map<std::string, Command> g_commands;
static const char* PROGRAM_NAME = "ohos-location";
static const char* TOOL_DESCRIPTION =
    "Location service CLI tool for querying and controlling device location features";
const int64_t USER_ID_MAX = 999;

#define CLI_LOG(fmt, ...) do { \
    fprintf(stdout, "[INFO] " fmt "\n", ##__VA_ARGS__); \
} while (0)

#define CLI_ERROR(fmt, ...) do { \
    fprintf(stdout, "[ERROR] " fmt "\n", ##__VA_ARGS__); \
} while (0)

void PrintUsage(const char* prog)
{
    CLI_ERROR("Usage: %s <command> [options...]", prog);
    CLI_ERROR("Run '%s help' for more information", prog);
}

static void RegisterCommand(const char* name, const char* description, int (*handler)(int, char**))
{
    g_commands.emplace(name, Command(name, description, handler));
}

int OutputLog(const std::string& message)
{
    json response;
    response["type"] = "log";
    response["level"] = "info";
    response["message"] = message;
    std::cout << response.dump() << std::endl;
    return 0;
}

int OutputSuccess(const json& data)
{
    json response;
    response["type"] = "result";
    response["status"] = "success";
    response["data"] = data;
    std::cout << response.dump() << std::endl;
    return 0;
}

static int OutputError(const std::string& code, const std::string& message, const std::string& suggestion = "")
{
    json response;
    response["type"] = "result";
    response["status"] = "failed";
    response["errCode"] = code;
    response["errMsg"] = message;
    if (!suggestion.empty()) {
        response["suggestion"] = suggestion;
    }
    std::cout << response.dump() << std::endl;
    return 1;
}

void  LocationToJson(const std::unique_ptr<Location>& location, json& data)
{
    if (location == nullptr) {
        return;
    }
    data["latitude"] = location->GetLatitude();
    data["longitude"] = location->GetLongitude();
    data["altitude"] = location->GetAltitude();
    data["accuracy"] = location->GetAccuracy();
    data["speed"] = location->GetSpeed();
    data["direction"] = location->GetDirection();
    data["timeStamp"] = location->GetTimeStamp();
    data["timeSinceBoot"] = location->GetTimeSinceBoot();
    data["isFromMock"] = location->GetIsFromMock();
    data["sourceType"] = location->GetLocationSourceType();
    data["altitudeAccuracy"] = location->GetAltitudeAccuracy();
    data["speedAccuracy"] = location->GetSpeedAccuracy();
    data["directionAccuracy"] = location->GetDirectionAccuracy();
    data["uncertaintyOfTimeSinceBoot"] = location->GetUncertaintyOfTimeSinceBoot();
}

static int AllCmdHelp()
{
    CLI_LOG("ohos-location - %s", TOOL_DESCRIPTION);
    CLI_LOG("");
    CLI_LOG("Usage:");
    CLI_LOG("  %s <command> [options]", PROGRAM_NAME);
    CLI_LOG("");
    CLI_LOG("Parameters:");
    CLI_LOG("  --help             Display this help message");
    CLI_LOG("");
    CLI_LOG("SubCommands:");
    CLI_LOG("  is-enabled          Query the current status of the device location switch");
    CLI_LOG("  enable              Enable the device location switch");
    CLI_LOG("  disable             Disable the device location switch");
    CLI_LOG("  get-last-approximate-location   Get the last cached approximate location from the system");
    CLI_LOG("  get-last-precise-location Get the last cached precise location from the system");
    CLI_LOG("  get-current-approximate-location   Get current location approximate with timeout waiting");
    CLI_LOG("  get-current-precise-location Get current location precise with timeout waiting");
    CLI_LOG("");
    CLI_LOG("Examples:");
    CLI_LOG("  # Show help for all commands");
    CLI_LOG("  %s --help", PROGRAM_NAME);
    CLI_LOG("");
    CLI_LOG("  # Query location switch status");
    CLI_LOG("  %s is-enabled", PROGRAM_NAME);
    CLI_LOG("");
    CLI_LOG("  # Get last cached location");
    CLI_LOG("  %s get-last-approximate-location", PROGRAM_NAME);
    CLI_LOG("");
    CLI_LOG("  # Get current location with custom timeout");
    CLI_LOG("  %s get-current-approximate-location --timeout 5000", PROGRAM_NAME);
    return 0;
}

static int IsEnabledCmdHelp()
{
    CLI_LOG("ohos-location is-enabled - Query the current status of the device location switch");
    CLI_LOG("");
    CLI_LOG("Usage:");
    CLI_LOG("  %s is-enabled", PROGRAM_NAME);
    CLI_LOG("");
    CLI_LOG("Parameters:");
    CLI_LOG("");
    CLI_LOG("Examples:");
    CLI_LOG("  # Query location switch status for current user");
    CLI_LOG("  %s is-enabled", PROGRAM_NAME);
    return 0;
}

static int EnableCmdHelp()
{
    CLI_LOG("ohos-location enable - Enable the device location switch");
    CLI_LOG("");
    CLI_LOG("Usage:");
    CLI_LOG("  %s enable [--userId <id>]", PROGRAM_NAME);
    CLI_LOG("");
    CLI_LOG("Parameters:");
    CLI_LOG("  --userId         Optional. User ID (range: -1 or 0-999, default: -1 for current user)");
    CLI_LOG("");
    CLI_LOG("Examples:");
    CLI_LOG("  # Enable location switch for current user");
    CLI_LOG("  %s enable", PROGRAM_NAME);
    CLI_LOG("");
    CLI_LOG("  # Enable location switch for user 100");
    CLI_LOG("  %s enable --userId 100", PROGRAM_NAME);
    return 0;
}

static int DisableCmdHelp()
{
    CLI_LOG("ohos-location disable - Disable the device location switch");
    CLI_LOG("");
    CLI_LOG("Usage:");
    CLI_LOG("  %s disable [--userId <id>]", PROGRAM_NAME);
    CLI_LOG("");
    CLI_LOG("Parameters:");
    CLI_LOG("  --userId         Optional. User ID (range: -1 or 0-999, default: -1 for current user)");
    CLI_LOG("");
    CLI_LOG("Examples:");
    CLI_LOG("  # Disable location switch for current user");
    CLI_LOG("  %s disable", PROGRAM_NAME);
    CLI_LOG("");
    CLI_LOG("  # Disable location switch for user 100");
    CLI_LOG("  %s disable --userId 100", PROGRAM_NAME);
    return 0;
}

static int GetLastLocationCmdHelp(std::string cmd)
{
    CLI_LOG("ohos-location %s - Get the last cached location from the system", cmd.c_str());
    CLI_LOG("");
    CLI_LOG("Usage:");
    CLI_LOG("  %s %s", PROGRAM_NAME, cmd.c_str());
    CLI_LOG("");
    CLI_LOG("Description:");
    CLI_LOG("  Retrieves the last known location that was cached by the system.");
    CLI_LOG("  This command requires LOCATION permission.");
    CLI_LOG("  Returns location data including latitude, longitude, altitude, accuracy, etc.");
    CLI_LOG("");
    CLI_LOG("Examples:");
    CLI_LOG("  # Get the last cached location");
    CLI_LOG("  %s %s", PROGRAM_NAME, cmd.c_str());
    return 0;
}

static int GetCurrentLocationCmdHelp(std::string cmd)
{
    CLI_LOG("ohos-location %s - Get current location with timeout waiting", cmd.c_str());
    CLI_LOG("");
    CLI_LOG("Usage:");
    CLI_LOG("  %s %s [--timeout <Millisecond>]", PROGRAM_NAME, cmd.c_str());
    CLI_LOG("");
    CLI_LOG("Parameters:");
    CLI_LOG("  --timeout         Optional. Timeout in Millisecond (range: 1000-10000, default: 2000)");
    CLI_LOG("");
    CLI_LOG("Description:");
    CLI_LOG("  Requests a fresh location fix and waits for the result.");
    CLI_LOG("  This command requires LOCATION permission.");
    CLI_LOG("  The --timeout parameter specifies how long to wait for location data.");
    CLI_LOG("");
    CLI_LOG("Examples:");
    CLI_LOG("  # Get current location with default timeout (2000 Millisecond)");
    CLI_LOG("  %s %s", PROGRAM_NAME, cmd.c_str());
    CLI_LOG("");
    CLI_LOG("  # Get current location with 5000 Millisecond timeout");
    CLI_LOG("  %s %s --timeout 5000", PROGRAM_NAME, cmd.c_str());
    return 0;
}

static int ProcessCmdHelp(int argc, char** argv)
{
    return AllCmdHelp();
}

static bool ParseIntArg(int argc, char** argv, const char* argName, int& value, bool required = false)
{
    if (argv == nullptr) {
        CLI_ERROR("argv is null");
        return false;
    }
    if (argName == nullptr) {
        CLI_ERROR("argName is null");
        return false;
    }
    for (int i = 0; i < argc; ++i) {
        if (argv[i] == nullptr) {
            continue;
        }
        if (strncmp(argv[i], argName, strlen(argName)) == 0) {
            if (i + 1 < argc && argv[i + 1] != nullptr && CommonUtils::isValidInteger(argv[i + 1])) {
                value = std::stoi(argv[i + 1]);
                return true;
            }
            return false;
        }
    }
    if (required) {
        CLI_ERROR("Missing required argument: %s", argName);
    }
    return false;
}

static bool ParseStringArg(int argc, char** argv, const char* argName, std::string& value, bool required = false)
{
    if (argv == nullptr) {
        CLI_ERROR("argv is null");
        return false;
    }
    if (argName == nullptr) {
        CLI_ERROR("argName is null");
        return false;
    }
    for (int i = 0; i < argc; ++i) {
        if (argv[i] == nullptr) {
            continue;
        }
        if (strncmp(argv[i], argName, strlen(argName)) == 0) {
            if (i + 1 < argc && argv[i + 1] != nullptr) {
                value = argv[i + 1];
                return true;
            }
            return false;
        }
    }
    if (required) {
        CLI_ERROR("Missing required argument: %s", argName);
    }
    return false;
}

class LocatorCallbackForCli : public IRemoteStub<ILocatorCallback> {
public:
    LocatorCallbackForCli();
    virtual ~LocatorCallbackForCli();
    int OnRemoteRequest(
        uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option) override;
    void OnLocationReport(const std::unique_ptr<Location>& location) override;
    void OnErrorReport(const int errCode) override;
    void OnLocatingStatusChange(const int status) override;
    bool WaitForResult(int timeoutSeconds, std::unique_ptr<Location>& outLocation, int& outErrCode);
private:
    std::mutex mutex_;
    std::condition_variable cv_;
    bool hasResult_ = false;
    std::unique_ptr<Location> location_ = nullptr;
    int errCode_ = 0;
};

LocatorCallbackForCli::LocatorCallbackForCli()
{
}

LocatorCallbackForCli::~LocatorCallbackForCli()
{
}

void LocatorCallbackForCli::OnLocatingStatusChange(const int status)
{
}

int LocatorCallbackForCli::OnRemoteRequest(uint32_t code,
    MessageParcel& data, MessageParcel& reply, MessageOption& option)
{
    if (data.ReadInterfaceToken() != GetDescriptor()) {
        return -1;
    }

    OutputLog("LocatorCallbackForCli::OnRemoteRequest:" + std::to_string(code));
    switch (code) {
        case RECEIVE_LOCATION_INFO_EVENT: {
            std::unique_ptr<Location> location = Location::UnmarshallingMakeUnique(data);
            OnLocationReport(location);
            break;
        }
        case RECEIVE_LOCATION_STATUS_EVENT: {
            int status = data.ReadInt32();
            OnLocatingStatusChange(status);
            break;
        }
        case RECEIVE_ERROR_INFO_EVENT: {
            int errCode = data.ReadInt32();
            OnErrorReport(errCode);
            break;
        }
        default: {
            IPCObjectStub::OnRemoteRequest(code, data, reply, option);
            break;
        }
    }
    return 0;
}

void LocatorCallbackForCli::OnLocationReport(const std::unique_ptr<Location>& location)
{
    if (location == nullptr) {
        return;
    }

    OutputLog("LocatorCallbackForCli::OnLocationReport:");
    std::unique_lock<std::mutex> lock1(mutex_);
    if (!hasResult_ && location != nullptr) {
        location_ = std::make_unique<Location>(*location);
        hasResult_ = true;
        cv_.notify_all();
    }
}

void LocatorCallbackForCli::OnErrorReport(const int errCode)
{
    std::unique_lock<std::mutex> lock1(mutex_);
    if (!hasResult_) {
        errCode_ = errCode;
        hasResult_ = true;
        cv_.notify_all();
    }
}

bool LocatorCallbackForCli::WaitForResult(int timeoutSeconds, std::unique_ptr<Location>& outLocation, int& outErrCode)
{
    std::unique_lock<std::mutex> lock(mutex_);
    bool success = cv_.wait_for(lock, std::chrono::seconds(timeoutSeconds),
        [this] { return hasResult_; });
    if (success && location_ != nullptr) {
        outLocation = std::move(location_);
        return true;
    }
    outErrCode = errCode_;
    return false;
}

static int ProcessCmdIsLocationEnabled(int argc, char** argv)
{
    if (argc == PARAM_NUM_TWO && argv != nullptr && std::string(argv[1]) == "--help") {
        return IsEnabledCmdHelp();
    }
    auto locator = LocatorImpl::GetInstance();
    bool isEnabled = false;
    locator->IsLocationEnabledV9(isEnabled);
    json data;
    data["isEnabled"] = isEnabled;
    return OutputSuccess(data);
}

static int ProcessCmdEnableLocation(int argc, char** argv)
{
    bool enable = true;
    int userId = -1;
    if (argv == nullptr) {
        return OutputError("ERR_ARG_INVALID", "ERR_ARG_INVALID", "");
    }
    if (argc == PARAM_NUM_TWO && argv[1] != nullptr && std::string(argv[1]) == "--help") {
        return EnableCmdHelp();
    }
    ParseIntArg(argc, argv, "--userId", userId, false);
    OutputLog("ProcessCmdEnableLocation userId:" + std::to_string(userId));
    if (userId > USER_ID_MAX) {
        return OutputError("ERR_ARG_INVALID", "The input parameter is incorrect. The value of the input \
            parameter is not within the valid range.", "Changing the value of an input parameter.");
    }
    auto locator = LocatorImpl::GetInstance();
    LocationErrCode ret;
    if (userId >= 0) {
        ret = locator->EnableAbilityForUser(enable, userId);
    } else {
        ret = locator->EnableAbilityV9(enable);
    }
    if (ret != ERRCODE_SUCCESS) {
        Security::AccessToken::PrivacyKit::AddPermissionUsedRecord(IPCSkeleton::GetCallingTokenID(),
            "ohos.permission.cli.CONTROL_LOCATION_SWITCH", 0, 1);
        return OutputError("ERR_LOC_ENABLE_FAIL", CommonUtils::GetErrorMsgByCode(ret), "");
    } else {
        Security::AccessToken::PrivacyKit::AddPermissionUsedRecord(IPCSkeleton::GetCallingTokenID(),
            "ohos.permission.cli.CONTROL_LOCATION_SWITCH", 1, 0);
    }
    json data;
    data["isEnabled"] = true;
    return OutputSuccess(data);
}

static int ProcessCmdDisableLocation(int argc, char** argv)
{
    bool enable = false;
    int userId = -1;
    if (argv == nullptr) {
        return OutputError("ERR_ARG_INVALID", "ERR_ARG_INVALID", "");
    }
    if (argc == PARAM_NUM_TWO && argv[1] != nullptr && std::string(argv[1]) == "--help") {
        return DisableCmdHelp();
    }
    ParseIntArg(argc, argv, "--userId", userId, false);
    OutputLog("ProcessCmdDisableLocation userId:" + std::to_string(userId));
    if (userId > USER_ID_MAX) {
        return OutputError("ERR_ARG_INVALID", "The input parameter is incorrect. The value of the input \
            parameter is not within the valid range.", "Changing the value of an input parameter.");
    }
    auto locator = LocatorImpl::GetInstance();
    LocationErrCode ret;
    if (userId >= 0) {
        ret = locator->EnableAbilityForUser(enable, userId);
    } else {
        ret = locator->EnableAbilityV9(enable);
    }
    if (ret != ERRCODE_SUCCESS) {
        Security::AccessToken::PrivacyKit::AddPermissionUsedRecord(IPCSkeleton::GetCallingTokenID(),
            "ohos.permission.cli.CONTROL_LOCATION_SWITCH", 0, 1);
        return OutputError("ERR_LOC_DISABLE_FAIL", CommonUtils::GetErrorMsgByCode(ret), "");
    } else {
        Security::AccessToken::PrivacyKit::AddPermissionUsedRecord(IPCSkeleton::GetCallingTokenID(),
            "ohos.permission.cli.CONTROL_LOCATION_SWITCH", 1, 0);
    }
    json data;
    data["isEnabled"] = false;
    return OutputSuccess(data);
}

static int ProcessCmdGetCachedLocation(int argc, char** argv)
{
    if (argc == PARAM_NUM_TWO && argv != nullptr && std::string(argv[1]) == "--help") {
        return GetLastLocationCmdHelp(argv[0]);
    }
    auto locator = LocatorImpl::GetInstance();
    std::unique_ptr<Location> location = nullptr;
    LocationErrCode ret = locator->GetCachedLocationV9(location);
    if (ret != ERRCODE_SUCCESS || location == nullptr) {
        return OutputError("ERR_LOC_GET_LAST_LOCATION_FAIL", CommonUtils::GetErrorMsgByCode(ret), "");
    }
    json data;
    LocationToJson(location, data);
    return OutputSuccess(data);
}

static int ProcessCmdStartLocating(int argc, char** argv)
{
    std::string priority = "accuracy";
    int timeInterval = 1000;
    int timeout = 2000;
    int fixNumber = 1;
    const int64_t TIME_OUT_MAX = 10000; //ms
    const int64_t TIME_OUT_MIN = 1000; //ms
    if (argv == nullptr) {
        return OutputError("ERR_INVALID_ARGUMENTS", "INPUT_PARAMS_ERROR", "");
    }
    if (argc == PARAM_NUM_TWO && argv[1] != nullptr && std::string(argv[1]) == "--help") {
        return GetCurrentLocationCmdHelp(argv[0]);
    }
    ParseStringArg(argc, argv, "--priority", priority, false);
    OutputLog("priority:" + priority);
    ParseIntArg(argc, argv, "--timeout", timeout, false);
    if (timeout > TIME_OUT_MAX || timeout < TIME_OUT_MIN) {
        return OutputError("ERR_ARG_INVALID", "The input parameter is incorrect. The value of the input \
            parameter is not within the valid range.", "Changing the value of an input parameter.");
    }
    OutputLog("timeout" + std::to_string(timeout));
    auto requestConfig = std::make_unique<RequestConfig>();
    requestConfig->SetPriority(PRIORITY_ACCURACY);
    if (priority == "speed") {
        requestConfig->SetPriority(PRIORITY_FAST_FIRST_FIX);
    }
    requestConfig->SetTimeInterval(timeInterval);
    requestConfig->SetFixNumber(fixNumber);
    requestConfig->SetTimeOut(timeout);
    sptr<LocatorCallbackForCli> callbackPtr = sptr<LocatorCallbackForCli>(new LocatorCallbackForCli());
    sptr<ILocatorCallback> callback = sptr<ILocatorCallback>(callbackPtr);
    LocationErrCode ret = LocatorImpl::GetInstance()->StartLocatingV9(requestConfig, callback);
    // 等待结果
    std::unique_ptr<Location> location = nullptr;
    int errCode = 0;
    if (ret == ERRCODE_SUCCESS) {
        OutputLog("WaitForResult start");
        callbackPtr->WaitForResult(timeout / MILLI_PER_SECS, location, errCode);
        OutputLog("WaitForResult end");
    } else {
        return OutputError("ERR_LOC_GET_CURRENT_LOCATION_FAIL", CommonUtils::GetErrorMsgByCode(ret), "");
    }
    // 停止定位
    LocatorImpl::GetInstance()->StopLocating(callback);
    LocatorImpl::GetInstance()->GetCachedLocationV9(location);
    json data;
    LocationToJson(location, data);
    return OutputSuccess(data);
}

static void InitCommands()
{
    RegisterCommand("help", "Show help message", ProcessCmdHelp);
    RegisterCommand("is-enabled", "Check if location switch is enabled", ProcessCmdIsLocationEnabled);
    RegisterCommand("enable", "Enable location switch", ProcessCmdEnableLocation);
    RegisterCommand("disable", "Disable location switch", ProcessCmdDisableLocation);
    RegisterCommand("get-last-approximate-location", "Get last known cached approximate location",
        ProcessCmdGetCachedLocation);
    RegisterCommand("get-last-precise-location", "Get last known cached precise location",
        ProcessCmdGetCachedLocation);
    RegisterCommand("get-current-approximate-location", "Get current approximate location",
        ProcessCmdStartLocating);
    RegisterCommand("get-current-precise-location", "Get current precise location", ProcessCmdStartLocating);
}
}
}
}

int main(int argc, char** argv)
{
    if (argc < PARAM_NUM_TWO) {
        OHOS::Location::PrintUsage(argv[0]);
        return 1;
    }
    OHOS::Location::InitCommands();
    std::string cmdName = argv[1];
    if (cmdName == "help" || cmdName == "--help" || cmdName == "-h") {
        return OHOS::Location::AllCmdHelp();
    }
    auto it = OHOS::Location::g_commands.find(cmdName);
    if (it == OHOS::Location::g_commands.end()) {
        json response;
        response["type"] = "result";
        response["status"] = "failed";
        response["errCode"] = "ERR_UNKNOWN_COMMAND";
        response["errMsg"] = "Unknown command: " + cmdName;
        response["suggestion"] =
            "Use '" + std::string(OHOS::Location::PROGRAM_NAME) + " --help' to see available commands.";
        std::cout << response.dump() << std::endl;
        return 1;
    }
    int cmdArgc = argc - 1;
    char** cmdArgv = argv + 1;
    return it->second.handler(cmdArgc, cmdArgv);
}