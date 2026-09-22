# GNSS 服务代码导览

## 何时读取

修改 GNSS 会话、驱动接口对接、卫星状态或位置回传，以及 `services/location_gnss/` 时读取。围栏相关修改还需读取[围栏路由](../routing/geofence-routing.md)。

## 职责与目录

本模块对接 GNSS HDI，管理定位会话并接收、转换和上报驱动结果。本仓库不应被当作卫星信号解算算法的实现目录。

| 目录 | 查找内容 |
|------|----------|
| `services/location_gnss/gnss/` | GNSS 服务、HDI 对接、事件回调及围栏能力 |
| `services/location_locator/locator/` | 请求调度及子服务结果接收 |
| `test/location_gnss/` | GNSS 服务、接口与回调用例 |

## 关键入口

以下入口均在 `services/location_gnss/gnss/`；具体 HDI 版本与能力开关以该目录的构建定义为准。

| 入口 | 适用问题 |
|------|----------|
| `GnssAbility::SendLocationRequest` | 从 Locator 接收定位请求 |
| `GnssAbility::StartGnss` / `GnssAbility::StopGnss` | GNSS 定位会话启动与停止 |
| `GnssEventCallback::ReportLocation` | 驱动位置结果进入服务 |

## 修改边界与验证

- 修改启动、停止或 HDI 交互时，检查驱动不可用、重复请求、最后一个请求结束后的资源释放，以及产品能力开关关闭的行为。
- 修改定位回调时，评估新增操作的频率和开销；新增阻塞等待、逐次日志或格式化必须说明必要性及性能验证方式，保留必要故障诊断。
- 修改位置回传时，联读 [Locator 代码导览](locator-service.md)及[权限约束](../expert/permission-constraints.md)，确认结果仍经相应服务端处理后交付客户端。
- 测试入口为 `test/location_gnss/`；驱动真实定位、卫星状态和功耗需设备验证，单元测试通过不能替代这些结论。
- 构建入口：`services/location_gnss/gnss/BUILD.gn` 的 `lbsservice_gnss`；执行步骤统一见[构建和测试](../verify/build-and-test.md)。
