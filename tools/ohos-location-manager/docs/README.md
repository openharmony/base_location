# ohos-location

## 概述

`ohos-location` 是 HO/OH 位置服务的命令行工具，用于查询和管理设备的位置相关功能。该工具提供了查询位置开关状态、启用/禁用位置开关、获取缓存位置、启动和停止定位请求等功能。

## 功能列表

- **查询位置开关状态**：检查设备的位置开关是否已启用
- **启用/禁用位置开关**：控制设备的位置开关
- **获取缓存位置**：获取设备最后一次已知的位置信息
- **启动定位请求**：发起定位请求，支持多种定位优先级
- **停止定位请求**：停止正在进行的定位请求
- **LRO 模式支持**：长时间运行的定位任务支持异步回调

## 依赖

- **系统能力**：
  - `SystemCapability.Location.Location.Core` - 定位核心能力
  - `SystemCapability.Location.Location.Gnss` - GNSS定位能力

- **内部依赖**：
  - `locator_sdk` - 定位 SDK
  - `nlohmann_json` - JSON 处理库
  - `base` - 基础工具库
  - `hilog` - 日志库

- **权限**：无特殊权限要求

## 技术架构

```
ohos-location
├── src/
│   └── main.cpp              # 主程序入口
├── config.json                # 工具描述文件
├── BUILD.gn                   # 构建配置
├── docs/
│   ├── README.md             # 项目文档
│   └── USAGE.md              # 使用说明
└── tests/
    └── test_main.cpp         # 测试代码
```

## 编译说明

### 前置条件

- 确保已同步 OpenHarmony 代码库
- 确保编译环境已配置完成（OHOS/SDK）

### 编译步骤

1. 在代码根目录执行编译：
   ```bash
   ./build.sh --product-name <product-name> --ccache --build-target ohos-location
   ```

2. 或使用完整系统编译：
   ```bash
   ./build.sh --product-name <product-name> --ccache
   ```

### 安装验证

编译成功后，可执行文件将安装到：
```
/system/bin/cli_tool/executable/ohos-location
```

## 使用场景

- **设备调试**：快速检查设备位置开关状态和定位能力
- **自动化测试**：集成到自动化测试脚本中验证位置服务
- **系统运维**：诊断和排查位置服务相关问题
- **应用开发**：辅助开发过程中验证位置接口调用

## 版本信息

- **当前版本**：1.0.0
- **支持平台**：OH 3.1+ / HO
- **维护团队**：Location SIG