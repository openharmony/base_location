# Locator主服务代码地图

## 模块职责

Locator服务是位置服务的核心入口，负责：
- 接收应用定位请求
- 选择合适的定位方式（GNSS/网络/被动）
- 位置结果缓存管理
- 多客户端请求管理

## 关键文件

| 文件 | 职责 |
|------|------|
| `services/location_locator/locator/` | Locator SA主目录 |
| `services/location_locator/callback/` | 回调处理 |
| `services/location_locator/hisysevent.yaml` | HiSysEvent配置 |

## 高风险入口

| 入口函数 | 说明 |
|----------|------|
| `OnStart()` | SA服务启动，可能触发配置加载 |
| `OnRequest()` | 处理位置请求消息 |
| `OnLocation报告()` | 接收底层定位结果 |
| `GetLocation()` | 同步获取位置（注意ANR风险） |

## 调用链路

```
应用请求 → Locator SDK → IPC → Locator SA → 选择定位策略
                                              ├── GNSS定位 → location_gnss SA
                                              ├── 网络定位 → location_network SA
                                              └── 被动定位 → location_passive SA
```

## 配置依赖

- `services/utils/` 提供配置解析公共能力
- `bundle.json` 中的 `hisysevent_config` 指定事件配置文件

## 编译单元

```gn
//base/location/location/services/location_locator/locator:lbsservice_locator
```
