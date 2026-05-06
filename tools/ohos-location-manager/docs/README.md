# ohos-location-manager

## 概述

ohos-location-manager 是 OpenHarmony 位置服务的 CLI 工具，提供定位开关状态查询、缓存位置获取、定位开关控制、启动/停止定位等功能。

## 功能列表

- 查询定位开关状态
- 获取缓存位置信息
- 启用/禁用定位开关
- 启动定位并等待结果
- 停止正在进行的定位

## 依赖

- 系统能力：SystemCapability.Location.Location.Core
- 权限：
  - `ohos.permission.APPROXIMATELY_LOCATION` - 获取位置信息
  - `ohos.permission.CONTROL_LOCATION_SWITCH` - 控制定位开关（系统权限）

## 安装路径

`/system/bin/cli_tool/executable/ohos-location-manager`

## 构建目标

```
//base/location/location/tools/ohos-location-manager:ohos-location-manager
```

## 相关文件

- `src/main.cpp` - 主程序源码
- `BUILD.gn` - 构建配置
- `config.json` - 工具描述文件
- `docs/README.md` - 项目概述
- `docs/USAGE.md` - 使用说明