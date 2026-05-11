# ohos-location 封装报告

## 1. 概述

| 项目 | 内容 |
|------|------|
| **CLI 名称** | ohos-location |
| **功能描述** | 定位服务 CLI 工具，提供位置开关查询、定位请求启停、缓存位置获取等功能 |
| **仓库类型** | 开源仓（ohos- 前缀） |
| **代码路径** | base/location/location/tools/ohos-location/ |

## 2. 接口分析

### 2.1 候选接口列表

| 接口名 | 权限 | 来源 |
|--------|------|------|
| IsLocationEnabledV9() | 无需权限（系统 API） | locator_sdk |
| StartLocatingV9() | 无需权限（系统 API） | locator_sdk |
| StopLocatingV9() | 无需权限（系统 API） | locator_sdk |
| EnableAbilityV9() | 无需权限（系统 API） | locator_sdk |
| GetCachedLocationV9() | 无需权限（系统 API） | locator_sdk |

### 2.2 权限需求分析

| 权限标识 | 用途说明 | 涉及命令 |
|---------|---------|----------|
| 无特殊权限 | 系统 API，内部检查权限 | 所有命令 |

### 2.3 异步分析结果

| 接口 | 类型 | 判断依据 | 封装决策 |
|------|------|---------|----------|
| IsLocationEnabledV9() | 同步 | 直接返回 bool 引用 | ✅ 封装 |
| StartLocatingV9() | 同步提交 | 方法同步返回，位置通过回调异步报告 | ✅ 封装（回调输出到 stdout） |
| StopLocatingV9() | 同步 | 直接返回错误码 | ✅ 封装 |
| EnableAbilityV9() | 同步 | 直接返回错误码 | ✅ 封装 |
| GetCachedLocationV9() | 同步 | 直接返回 location 指针 | ✅ 封装 |

## 3. 命令设计

### 3.1 命令列表

| CLI 命令 | 对应接口 | 权限 | 参数 | 说明 |
|---------|---------|------|------|------|
| is-enabled | IsLocationEnabledV9() | 无 | 无 | 查询位置开关状态 |
| enable | EnableAbilityV9() | 无 | --enable, --userId | 启用/禁用位置开关 |
| get-last-location | GetCachedLocationV9() | 无 | 无 | 获取缓存位置 |
| start-locating | StartLocatingV9() | 无 | --priority, --time-interval, --fix-number | 启动定位请求 |
| stop-locating | StopLocatingV9() | 无 | 无 | 停止定位请求 |
| help | - | 无 | 无 | 显示帮助信息 |

### 3.2 命令依赖关系

| CLI 命令 | 前置依赖命令 | 依赖条件 | 说明 |
|---------|------------|---------|------|
| is-enabled | 无 | - | 直接调用系统接口 |
| enable | 无 | - | 直接调用系统接口 |
| get-last-location | 无 | - | 直接调用系统接口 |
| start-locating |  |  |  |
| stop-locating | start-locating | - | 停止前需先启动 |

## 4. 代码实现

### 4.1 文件结构

```text
tools/ohos-location/
├── src/
│   └── main.cpp              # 主入口，6 个命令处理器
├── tests/
│   └── test_main.cpp        # 测试文件
├── docs/
│   ├── README.md            # 项目说明
│   └── USAGE.md             # 使用说明
├── BUILD.gn                  # 构建配置
├── config.json               # 工具描述文件
└── ohos-location_report.md
```

### 4.2 关键代码说明

**命令分发**：使用 `std::unordered_map` 静态命令表
```cpp
static std::unordered_map<std::string, Command> g_commands;

static void RegisterCommand(const char* name, const char* description, int (*handler)(int, char**)) {
    g_commands.emplace(name, Command(name, description, handler));
}

static void InitCommands() {
    RegisterCommand("help", "Show help message", ProcessCmdHelp);
    RegisterCommand("is-enabled", "Check if location switch is enabled", ProcessCmdIsLocationEnabled);
    RegisterCommand("enable", "Enable location switch", ProcessCmdEnableLocation);
    RegisterCommand("get-last-location", "Get last known cached location", ProcessCmdGetCachedLocation);
    RegisterCommand("start-locating", "Start location request (LRO mode)", ProcessCmdStartLocating);
}
```

**错误码对齐**：
```cpp
constexpr int32_t MY_ERR_OK = 0;  // 与 OHOS::Location::LocationErrCode::SUCCESS 对齐
```

**输出格式**：
- `stdout`: JSON 格式结果输出
- `stderr`: INFO/ERROR 日志

**异步回调处理**：
```cpp
class LocatorCallbackForCli : public ILocatorCallback {
public:
    LocatorCallbackForCli(const std::string& task_id) : taskId_(task_id) {}
    void OnLocationReport(const std::unique_ptr<OHOS::Location::Location>& location) override;
    // ... 位置变化通过 stdout JSON 输出
};
```

### 4.3 BUILD.gn 关键配置

```gn
ohos_executable("ohos-location") {
  sources = [ "src/main.cpp" ]

  deps = [
    "//base/location/location/frameworks/native/locator_sdk:locator_sdk_static",
  ]

  external_deps = [
    "ipc:ipc_core",
    "ipc:ipc_single",
    "samgr:samgr_proxy",
    "hilog:libhilog",
  ]

  install_enable = true
  install_images = ["system"]
  relative_install_dir = "cli_tool/executable"
}
```

## 5. 编译验证

### 5.1 编译配置

| 项目 | 值 |
|------|-----|
| **ABI 类型** | generic_generic_arm_64only |
| **设备类型** | general_all_phone_standard |
| **编译命令** | `./build_system.sh --abi-type generic_generic_arm_64only --device-type general_all_phone_standard --ccache --build-variant root -j64 --build-target ohos-location` |

### 5.2 编译过程

**尝试 1**：
- **结果**: 失败
- **错误**: `fatal error: no matching constructor for initialization of 'Command'`
- **修复**: 修改 Command 结构体，添加显式构造函数，初始化方式改用 RegisterCommand 辅助函数

**尝试 2**：
- **结果**: 失败
- **错误**: `undefined symbol: OHOS::RefBase::RefBase()`
- **修复**: 添加 `ipc:ipc_core` 到 `external_deps`

**尝试 3**：
- **结果**: 失败
- **错误**: 仍存在 RefBase 符号未定义
- **修复**: 使用 `locator_sdk_static` 替代 `locator_sdk`，添加 `ipc:ipc_single` 和 `samgr:samgr_proxy`

**尝试 4**：
- **结果**: 成功
- **产物路径**: `out/generic_generic_arm_64only/general_all_phone_standard/location/location/ohos-location`

### 5.3 最终结果

✅ **编译成功**
- **尝试次数**: 4
- **产物路径**: `out/generic_generic_arm_64only/general_all_phone_standard/location/location/ohos-location`
- **文件大小**: 103KB

## 6. 使用说明

### 6.1 安装路径

```
/system/bin/cli_tool/executable/ohos-location
```

### 6.2 命令示例

**检查位置开关状态**：
```bash
ohos-location is-enabled
```

**启用位置开关**：
```bash
ohos-location enable --enable=true
```

**获取缓存位置**：
```bash
ohos-location get-last-location
```

**启动定位（高精度模式）**：
```bash
ohos-location start-locating --priority=high --time-interval=1000 --fix-number=1
```

**启动定位（低功耗模式）**：
```bash
ohos-location start-locating --priority=low
```

**停止定位**：
```bash
ohos-location stop-locating
```

**显示帮助**：
```bash
ohos-location help
```

### 6.3 输出格式

成功响应（stdout）：
```json
{"success":true,"data":{"isEnabled":true,"errCode":0,"errMsg":"success"}}
```

错误响应（stdout）：
```json
{"success":false,"error":{"code":"SERVICE_UNAVAILABLE","message":"Failed to get LocatorImpl instance","suggestion":"Ensure location service is available"}}
```

位置回调事件（stdout）：
```json
{"event":"location_report","task_id":"task_1234567890","location":{"latitude":31.123456,"longitude":121.654321,"altitude":10.5,"accuracy":5.0,"time":1234567890}}
```

## 7. 待办事项

### 7.1 未封装接口

| 接口名 | 原因 |
|--------|------|
| 无 | 已按需求封装全部 5 个接口 |

### 7.2 优化建议

1. 支持多用户场景的 userId 参数传递
2. 添加 `--scenario` 参数支持定位场景切换（导航/出行/设备姿态）
3. 支持连续定位模式（--continuous 参数）
4. 添加超时控制参数
5. 支持输出格式切换（--format=json/text）

## 8. 参考文档

- [接口探索指南](../references/01-interface-exploration.md)
- [接口分析指南](../references/02-interface-analysis.md)
- [代码实现指南](../references/03-0-code-implementation.md)
- [编译指南](../references/04-build-guide.md)