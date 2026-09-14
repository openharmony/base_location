# GNSS定位服务代码地图

## 模块职责

GNSS服务负责：
- 接收卫星信号
- 计算设备位置
- 管理GNSS会话状态
- 输出位置到Locator服务

## 关键文件

| 文件 | 职责 |
|------|------|
| `services/location_gnss/gnss/` | GNSS SA主目录 |
| `services/location_gnss/gnss/` | 核心定位算法 |

## 高风险入口

| 入口函数 | 说明 |
|----------|------|
| `StartGnss()` | 启动GNSS定位 |
| `StopGnss()` | 停止GNSS定位 |
| `ReportLocation()` | 报告定位结果（高频） |

## 性能约束

- GNSS回调是高频路径，**禁止**在回调中执行：
  - 字符串格式化
  - 复杂日志打印
  - 同步IPC调用
- 位置计算应异步完成

## 编译单元

```gn
//base/location/location/services/location_gnss/gnss:lbsservice_gnss
```
