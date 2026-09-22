# 围栏功能路由

## 何时读取

修改围栏添加、移除、事件通知或 Fence Extension 时读取。先从调用的 API 确定 GNSS、融合或蓝牙围栏类型，再选择对应实现；不能统一归为 Locator 内的位置进出判断。

## 目录与入口

| 场景 | 目录 | 关键入口或查找对象 |
|------|------|--------------------|
| GNSS 围栏 SDK | `frameworks/native/geofence_sdk/` | `GeofenceManager::AddGnssGeofence` / `GeofenceManager::RemoveGnssGeofence` |
| GNSS 围栏服务端校验 | `services/location_locator/locator/` | `LocatorAbility::AddGnssGeofence` |
| GNSS 围栏注册与驱动事件 | `services/location_gnss/gnss/` | `GnssAbility::AddGnssGeofence`、`GeofenceEventCallback::ReportGeofenceEvent` |
| 融合围栏 | `services/location_gnss/gnss/` | `FusionFenceAbility::AddFusionFence` |
| 蓝牙围栏 | `services/location_locator/locator/` | `BeaconFenceManager::AddBeaconFence` |
| JS 参数及通知适配 | `frameworks/js/napi/` | 按公开 API 名称检索导出与绑定，见 [JS 路由](js-napi-routing.md) |
| 围栏扩展能力 | `frameworks/native/fence_extension_ability/`、`frameworks/js/napi/fence_extension_ability/`、`frameworks/js/napi/fence_extension_ability_context/` | 扩展能力生命周期与上下文 |

GNSS 驱动事件从服务回调进入；涉及其余 HDI 版本或融合围栏时，按所用接口查找相应回调。旧接口 `GeofenceManager::AddFenceV9` 与 GNSS 围栏入口需分别追踪，不能仅凭名称当作同一实现。

## 修改边界

- 修改注册或移除时，核对服务端权限、调用者与围栏归属、返回错误及产品支持能力；联读[权限约束](../expert/permission-constraints.md)。
- 修改事件通知时，区分“注册/移除操作结果”和“进入/离开等围栏事件”，核对 ID 与回调对象对应关系，并验证移除或客户端死亡后的处理。
- 修改 GNSS 围栏协议时，联读 [GNSS 代码导览](../code_map/gnss-service.md)与 [SA/IPC 约束](../expert/sa-ipc-constraints.md)，检查实际启用的 HDI 版本及能力开关。
- 修改 Fence Extension 时，从扩展能力调用方核实用途；不要假设全部围栏 API 都经过该扩展能力。

## 验证入口

- `test/location_geofence/`：SDK、请求数据与客户端回调。
- `test/location_gnss/`：GNSS/融合围栏及驱动事件回调。
- `test/location_locator/`：服务端与蓝牙围栏相关用例。
- 按改动补充无权限、重复注册、移除后事件和客户端死亡等受影响场景；真实进出区域及硬件事件必须在支持相应能力的设备上验证。
- 构建定义以相关目录的 `BUILD.gn` 为准，执行方法见[构建和测试](../verify/build-and-test.md)。
