# Locator 主服务代码导览

## 何时读取

修改定位请求、停止订阅、位置缓存、结果分发，或 `services/location_locator/` 时读取。权限及后台行为还需联读[权限约束](../expert/permission-constraints.md)。

## 职责与目录

Locator 接收客户端请求，检查调用条件，管理请求并协调定位子服务；位置结果的分发与缓存处理也在本模块。不要把所有围栏事件或所有服务回调都视为普通定位订阅。

| 目录 | 查找内容 |
|------|----------|
| `services/location_locator/locator/` | 服务入口、请求管理、结果处理与缓存 |
| `services/location_locator/callback/` | 向客户端发送回调的代理实现 |
| `frameworks/native/locator_sdk/` | 客户端 SDK 及 Locator IPC 接口 |

## 关键入口

以下入口均在 `services/location_locator/locator/`，从符号检索继续追踪当前实现。

| 入口 | 适用问题 |
|------|----------|
| `LocatorAbility::StartLocating` / `LocatorAbility::StopLocating` | 请求校验、订阅启动与停止 |
| `LocatorAbility::GetCacheLocation` | 缓存位置的访问条件与返回行为 |
| `LocatorAbility::ReportLocation` | 子服务上报的位置如何进入 Locator |
| `ReportManager::ReportLocationByCallback` | 客户端回调及发送前检查 |

## 修改边界与验证

- 修改请求或缓存路径时，核对服务端调用身份、位置开关、精确/模糊定位及后台访问分支；不能以 SDK 已校验替代服务端校验。
- 修改结果分发或停止请求时，检查回调生命周期、客户端死亡及停止后的结果处理；增加对应回归用例，不能只验证正常启动。
- 修改 IPC 或跨服务交互时，联读 [SA/IPC 约束](../expert/sa-ipc-constraints.md)；涉及具体定位方式时再读 [GNSS](gnss-service.md)或[网络定位](network-location.md)。
- 测试入口：`test/location_locator/`、`test/location_manager/`；先按受影响入口查找用例，再补充缺失场景。
- 构建入口：`services/location_locator/locator/BUILD.gn` 的 `lbsservice_locator`；环境、测试目标与执行方法统一见[构建和测试](../verify/build-and-test.md)。
