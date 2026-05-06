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

#include <iostream>
#include <string>
#include <unordered_map>
#include <functional>
#include <cstring>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <chrono>

#include "nlohmann/json.hpp"
using json = nlohmann::json;

#include "locator_impl.h"
#include "i_locator_callback.h"
#include "request_config.h"
#include "location.h"
#include "constant_definition.h"

#define CLI_LOG(fmt, ...) std::cerr << fmt << std::endl
#define CLI_ERROR(fmt, ...) std::cerr << "[ERROR] " << fmt << std::endl

namespace OHOS {
namespace Location {
namespace CliTool {

constexpr int DEFAULT_LOCATE_TIMEOUT = 30;
constexpr int MIN_ARGC_NUM = 2;

typedef std::function<int(int, char**)> CommandHandler;

struct Command {
    const char* name;
    const char* description;
    CommandHandler handler;
};

static std::unordered_map<std::string, Command> g_commands;

#define REGISTER_CMD(name, desc, handler) \
    g_commands[name] = {name, desc, handler}

static std::shared_ptr<LocatorImpl> g_locator = nullptr;

static void InitLocator()
{
    if (g_locator == nullptr) {
        g_locator = LocatorImpl::GetInstance();
    }
}

class LocationCallbackStub : public IRemoteStub<ILocatorCallback> {
public:
    LocationCallbackStub() : hasResult_(false), errorCode_(0) {}
    ~LocationCallbackStub() override = default;

    void OnLocationReport(const std::unique_ptr<Location>& location) override
    {
        if (location != nullptr) {
            location_ = std::make_unique<Location>(*location);
            hasResult_ = true;
            cv_.notify_one();
        }
    }

    void OnLocatingStatusChange(const int status) override
    {
        CLI_LOG("Locating status changed: " + std::to_string(status));
    }

    void OnErrorReport(const int errorCode) override
    {
        errorCode_ = errorCode;
        hasResult_ = true;
        cv_.notify_one();
    }

    int OnRemoteRequest(uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option) override
    {
        return 0;
    }

    bool WaitForResult(int timeoutSeconds, std::unique_ptr<Location>& outLocation, int& outErrorCode)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        auto deadline = std::chrono::seconds(timeoutSeconds);
        if (cv_.wait_for(lock, deadline, [this] { return hasResult_.load(); })) {
            if (location_ != nullptr) {
                outLocation = std::move(location_);
                outErrorCode = 0;
                return true;
            } else {
                outErrorCode = errorCode_;
                return false;
            }
        }
        outErrorCode = ERRCODE_REQUEST_TIMEOUT;
        return false;
    }

private:
    std::atomic<bool> hasResult_;
    std::mutex mutex_;
    std::condition_variable cv_;
    std::unique_ptr<Location> location_;
    int errorCode_;
};

static sptr<LocationCallbackStub> g_activeCallback = nullptr;

static int OutputSuccess(const json& data)
{
    json response;
    response["code"] = ERRCODE_SUCCESS;
    response["data"] = data;
    std::cout << response.dump() << std::endl;
    return 0;
}

static int OutputError(int code, const std::string& message, const std::string& suggestion = "")
{
    json response;
    response["code"] = code;
    response["error"]["message"] = message;
    if (!suggestion.empty()) {
        response["suggestion"] = suggestion;
    }
    std::cout << response.dump() << std::endl;
    return 1;
}

static json LocationToJson(const std::unique_ptr<Location>& location)
{
    json locJson;
    if (location == nullptr) {
        return locJson;
    }
    locJson["latitude"] = location->GetLatitude();
    locJson["longitude"] = location->GetLongitude();
    locJson["altitude"] = location->GetAltitude();
    locJson["accuracy"] = location->GetAccuracy();
    locJson["speed"] = location->GetSpeed();
    locJson["direction"] = location->GetDirection();
    locJson["timestamp"] = location->GetTimeStamp();
    locJson["timeSinceBoot"] = location->GetTimeSinceBoot();
    locJson["altitudeAccuracy"] = location->GetAltitudeAccuracy();
    locJson["speedAccuracy"] = location->GetSpeedAccuracy();
    locJson["directionAccuracy"] = location->GetDirectionAccuracy();
    locJson["sourceType"] = location->GetLocationSourceType();
    return locJson;
}

static std::string ParseStringArg(int argc, char** argv, const std::string& argName)
{
    for (int i = 0; i < argc; i++) {
        std::string arg = argv[i];
        if (arg.find("--" + argName + "=") == 0) {
            return arg.substr(argName.length() + 3);
        }
        if (arg == "--" + argName && i + 1 < argc) {
            return argv[i + 1];
        }
    }
    return "";
}

static int ParseIntArg(int argc, char** argv, const std::string& argName, int defaultValue)
{
    std::string value = ParseStringArg(argc, argv, argName);
    if (value.empty()) {
        return defaultValue;
    }
    try {
        return std::stoi(value);
    } catch (...) {
        return defaultValue;
    }
}

static bool ParseBoolFlag(int argc, char** argv, const std::string& flagName)
{
    for (int i = 0; i < argc; i++) {
        if (argv[i] == "--" + flagName) {
            return true;
        }
    }
    return false;
}

int CmdGetSwitchState(int argc, char** argv)
{
    InitLocator();
    if (g_locator == nullptr) {
        return OutputError(ERRCODE_SERVICE_UNAVAILABLE,
            "Failed to get location service instance",
            "Check if location service is running");
    }
    bool isEnabled = false;
    LocationErrCode errCode = g_locator->IsLocationEnabledV9(isEnabled);
    if (errCode != ERRCODE_SUCCESS) {
        return OutputError(errCode, 
            "Failed to query location switch state",
            "Check if location service is available");
    }
    json data;
    data["enabled"] = isEnabled;
    return OutputSuccess(data);
}

int CmdGetCachedLocation(int argc, char** argv)
{
    InitLocator();
    if (g_locator == nullptr) {
        return OutputError(ERRCODE_SERVICE_UNAVAILABLE,
            "Failed to get location service instance",
            "Check if location service is running");
    }
    std::unique_ptr<Location> location;
    LocationErrCode errCode = g_locator->GetCachedLocationV9(location);

    if (errCode != ERRCODE_SUCCESS) {
        std::string suggestion;
        if (errCode == ERRCODE_SWITCH_OFF) {
            suggestion = "Enable location switch first";
        } else if (errCode == ERRCODE_LOCATING_CACHE_FAIL) {
            suggestion = "No cached location available, try start-locate first";
        } else if (errCode == ERRCODE_PERMISSION_DENIED) {
            suggestion = "Request ohos.permission.APPROXIMATELY_LOCATION permission";
        }
        return OutputError(errCode, "Failed to get cached location", suggestion);
    }
    json data;
    data["location"] = LocationToJson(location);
    return OutputSuccess(data);
}

int CmdEnableSwitch(int argc, char** argv)
{
    InitLocator();
    if (g_locator == nullptr) {
        return OutputError(ERRCODE_SERVICE_UNAVAILABLE,
            "Failed to get location service instance",
            "Check if location service is running");
    }
    bool enable = ParseBoolFlag(argc, argv, "enable");
    bool disable = ParseBoolFlag(argc, argv, "disable");

    if (!enable && !disable) {
        return OutputError(ERRCODE_INVALID_PARAM,
            "Missing --enable or --disable parameter",
            "Use --enable to turn on location switch, or --disable to turn off");
    }
    bool targetState = enable;
    LocationErrCode errCode = g_locator->EnableAbilityV9(targetState);

    if (errCode != ERRCODE_SUCCESS) {
        std::string suggestion;
        if (errCode == ERRCODE_PERMISSION_DENIED || errCode == LOCATION_ERRCODE_PERMISSION_DENIED) {
            suggestion = "Request ohos.permission.CONTROL_LOCATION_SWITCH (system permission)";
        }
        return OutputError(errCode,
            std::string("Failed to ") + (enable ? "enable" : "disable") + " location switch",
            suggestion);
    }
    json data;
    data["message"] = std::string("Location switch ") + (enable ? "enabled" : "disabled");
    return OutputSuccess(data);
}

int CmdStartLocate(int argc, char** argv)
{
    InitLocator();
    if (g_locator == nullptr) {
        return OutputError(ERRCODE_SERVICE_UNAVAILABLE,
            "Failed to get location service instance",
            "Check if location service is running");
    }
    int timeout = ParseIntArg(argc, argv, "timeout", DEFAULT_LOCATE_TIMEOUT);
    int scenario = ParseIntArg(argc, argv, "scenario", SCENE_DAILY_LIFE_SERVICE);
    int interval = ParseIntArg(argc, argv, "interval", 1);

    auto requestConfig = std::make_unique<RequestConfig>();
    requestConfig->SetScenario(scenario);
    requestConfig->SetTimeInterval(interval);

    g_activeCallback = new LocationCallbackStub();
    if (g_activeCallback == nullptr) {
        return OutputError(ERRCODE_SERVICE_UNAVAILABLE,
            "Failed to create callback stub",
            "Check system resources");
    }
    LocationErrCode errCode = g_locator->StartLocatingV9(requestConfig, g_activeCallback);
    if (errCode != ERRCODE_SUCCESS) {
        g_activeCallback = nullptr;
        std::string suggestion;
        if (errCode == ERRCODE_SWITCH_OFF) {
            suggestion = "Enable location switch first using enable-switch --enable";
        } else if (errCode == ERRCODE_PERMISSION_DENIED || errCode == LOCATION_ERRCODE_PERMISSION_DENIED) {
            suggestion = "Request ohos.permission.APPROXIMATELY_LOCATION permission";
        }
        return OutputError(errCode, "Failed to start locating", suggestion);
    }
    std::unique_ptr<Location> location;
    int resultError = 0;
    bool success = g_activeCallback->WaitForResult(timeout, location, resultError);
    g_locator->StopLocatingV9(g_activeCallback);
    g_activeCallback = nullptr;
    if (!success) {
        std::string suggestion;
        if (resultError == ERRCODE_REQUEST_TIMEOUT) {
            suggestion = "Increase timeout value or check GPS signal";
        } else if (resultError == ERRCODE_LOCATING_FAIL) {
            suggestion = "Check GPS/network availability";
        }
        return OutputError(resultError, "Locating failed or timed out", suggestion);
    }
    json data;
    data["location"] = LocationToJson(location);
    return OutputSuccess(data);
}

int CmdStopLocate(int argc, char** argv)
{
    InitLocator();
    if (g_locator == nullptr) {
        return OutputError(ERRCODE_SERVICE_UNAVAILABLE,
            "Failed to get location service instance",
            "Check if location service is running");
    }
    if (g_activeCallback == nullptr) {
        return OutputError(ERRCODE_INVALID_PARAM,
            "No active locating session",
            "Run start-locate first to create a locating session");
    }
    LocationErrCode errCode = g_locator->StopLocatingV9(g_activeCallback);
    g_activeCallback = nullptr;
    if (errCode != ERRCODE_SUCCESS) {
        return OutputError(errCode, "Failed to stop locating",
            "Check if locating session is still active");
    }
    json data;
    data["message"] = "Locating stopped";
    return OutputSuccess(data);
}

int CmdHelp(int argc, char** argv)
{
    CLI_LOG("ohos-location-manager - Location service CLI tool");
    CLI_LOG("");
    CLI_LOG("Available commands:");
    for (const auto& pair : g_commands) {
        CLI_LOG("  " + std::string(pair.second.name) + "    " + std::string(pair.second.description));
    }
    CLI_LOG("");
    CLI_LOG("Run 'ohos-location-manager <command> --help' for more information on each command.");
    return 0;
}

void InitCommands()
{
    REGISTER_CMD("help", "Show help message", CmdHelp);
    REGISTER_CMD("get-switch-state", "Query location switch state (no permission required)", CmdGetSwitchState);
    REGISTER_CMD("get-cached-location", "Get cached location (requires APPROXIMATELY_LOCATION)", CmdGetCachedLocation);
    REGISTER_CMD("enable-switch", "Enable/disable location switch (requires CONTROL_LOCATION_SWITCH)", CmdEnableSwitch);
    REGISTER_CMD("start-locate",
        "Start locating and wait for result (requires APPROXIMATELY_LOCATION)", CmdStartLocate);
    REGISTER_CMD("stop-locate", "Stop active locating session (requires APPROXIMATELY_LOCATION)", CmdStopLocate);
}

void PrintUsage(const char* prog)
{
    CLI_ERROR("Usage: " + std::string(prog) + " <command> [options]");
    CLI_ERROR("Run '" + std::string(prog) + " help' for more information");
}

} // namespace CliTool
} // namespace Location
} // namespace OHOS

int main(int argc, char** argv)
{
    using namespace OHOS::Location::CliTool;
    if (argc < MIN_ARGC_NUM) {
        PrintUsage(argv[0]);
        return 1;
    }
    InitCommands();
    std::string cmdName = argv[1];
    auto it = g_commands.find(cmdName);

    if (it == g_commands.end()) {
        CLI_ERROR("Unknown command: " + cmdName);
        PrintUsage(argv[0]);
        return 1;
    }
    int cmdArgc = argc - MIN_ARGC_NUM;
    char** cmdArgv = argv + MIN_ARGC_NUM;
    return it->second.handler(cmdArgc, cmdArgv);
}