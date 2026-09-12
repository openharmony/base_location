# 网络定位服务代码地图

## 模块职责

网络定位服务负责：
- 基站定位
- WLAN/蓝牙定位
- 混合定位策略

## 关键文件

| 文件 | 职责 |
|------|------|
| `services/location_network/` | 网络定位SA目录 |

## 高风险入口

| 入口函数 | 说明 |
|----------|------|
| `StartNetworkLocation()` | 启动网络定位 |
| `GetNetworkLocation()` | 获取网络位置 |

## 依赖服务

- 可能依赖 `wifi`、`bluetooth` 子系统
- 可能依赖 `cellular_data` 获取基站信息

## 编译单元

```gn
//base/location/location/services/location_network/network:lbsservice_network
```
