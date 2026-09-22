# 位置权限与结果边界

适用：修改定位请求、位置缓存、结果上报、调用身份或位置开关逻辑。SDK 的检查不能替代服务端鉴权；先联读 [Locator 入口](../code_map/locator-service.md)。以下路径均相对仓库根目录。

## 从哪里查

| 目录 | 关键入口 | 用途 |
| --- | --- | --- |
| `services/location_locator/locator/` | `LocatorAbility::StartLocating`、`GetCacheLocation` | 请求入口的调用资格、开关和权限检查 |
| 同上 | `LocatorAbility::GetAppIdentityInfo` | 从 IPC 上下文取得调用身份 |
| 同上 | `ReportManager::GetPermittedLocation` | 结果返回前的权限、用户与精度处理 |
| 同上 | `ReportManager::IsAppBackground`、`LocatorAbility::NeedReportCacheLocation` | 后台状态判定与请求中的缓存返回 |
| `frameworks/location_common/common/` | `PermissionManager::CheckPermission` | 调用者 token 与 first token 的权限验证 |

只从这些入口追踪本次修改涉及的分支，不在本文维护内部调用链或逐行实现。

## 必须保持的边界

| 修改什么 | 必须保持什么 | 核查与验证 |
| --- | --- | --- |
| SDK、IPC 或请求身份传递 | 服务端使用真实调用身份；保留 token、full token、first token 及用户身份语义 | 对照 `GetAppIdentityInfo` 和 `CheckPermission`；覆盖无效身份、直接调用和代理调用，不能随意清零 first token |
| 定位请求或权限判断 | 保留对应服务端入口的调用资格和权限校验，不能因 SDK 已校验就删除 | 检查 `StartLocating` 及本次涉及的其余入口；验证拒绝时不向该调用者返回或上报位置数据，错误码符合接口约定 |
| 返回位置或修改精度 | 仅有模糊位置权限时不得返回精确结果；保留现有接口的精确权限要求及旧接口兼容语义 | 检查 `GetPermittedLocation`；覆盖无权限、仅模糊权限、精确权限组合，不能把各入口的权限条件统一替换 |
| 后台请求、持续上报或缓存 | 保留各路径的后台判定、权限重检、活动用户隔离及授权精度处理 | 联查 `IsAppBackground`、`NeedReportCacheLocation`、`GetPermittedLocation`；覆盖前后台切换、撤权、切换用户和缓存命中 |
| 位置开关与忽略开关功能 | 普通请求遵守开关；现有受控例外不得扩大到其余调用者 | 查 `LocatorAbility::SetLocationSwitchIgnored` 和受影响请求入口；覆盖开关关闭、无忽略权限和已有合法例外 |

位置权限并非所有接口都采用同一个组合：`LocatorAbility::CheckLocationPermission`、`CheckPreciseLocationPermissions` 和结果过滤承担不同职责，应以本次入口的条件为准。

后台访问也不能简化成“应用在后台就一律拒绝”或“已有定位权限就放行”。后台判定包含现有业务条件，结果路径还区分请求类型；改缓存时必须沿返回路径检查，不能直接返回未经授权过滤的缓存坐标。

## 修改与评审要求

- 表中规则是默认保持的安全与兼容边界。任务明确要求调整授权策略时，应记录受影响入口、调用者范围、返回精度和拒绝行为，并补充对应回归证据；不能以性能优化或重构为由扩大授权。
- 权限使用记录和权限状态变化处理属于访问链路的一部分。移动返回路径时检查是否遗漏记录、撤权后的停止或拒绝处理。
- 新增日志应沿用仓内日志机制和隐私标记；不要为了诊断将坐标或调用身份改为公开输出。

## 验证入口

在 `test/location_locator/source/` 查找并按改动补充：

- `permission_manager_test.cpp`：权限组合与 token 传递。
- `location_without_permission_test.cpp`、`location_approximately_permission_test.cpp`：拒绝访问与模糊结果。
- `report_manager_test.cpp`、`locator_service_test.cpp`：结果处理与服务入口。

这些文件是测试起点，不代表已覆盖上表全部场景。按 [构建与验证](../verify/build-and-test.md) 执行相关目标；报告实际执行内容、结果和未验证项。
