# 地理围栏知识路由

## 功能概述

地理围栏（Geofence）是基于位置的触发机制，当设备进入或离开特定区域时通知应用。

## 围栏相关组件

| 组件 | 代码路径 | 职责 |
|------|---------|------|
| **geofence_sdk** | `frameworks/native/geofence_sdk/` | 围栏SDK |
| **fence_extension_ability** | `frameworks/native/fence_extension_ability/` | 扩展能力 |
| **fence_extension_ability (JS)** | `frameworks/js/napi/fence_extension_ability/` | JS NAPI |

## 围栏触发流程

```
应用设置围栏 → Geofence SDK → IPC → Locator SA → 位置监控
                                                    ↓
                                              进入/离开判断
                                                    ↓
                                              回调通知应用
```

## 路由场景

| 场景 | 先读 |
|------|------|
| **添加围栏** | `frameworks/native/geofence_sdk/` 下的实现 |
| **围栏触发调试** | Locator服务的位置监控逻辑 |
| **围栏回调问题** | JS NAPI层回调绑定 |
