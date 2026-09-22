# 地理编码服务代码导览

## 何时读取

修改地址与坐标转换、地理编码服务连接，或 `services/location_geocode/` 时读取。

## 职责与目录

本模块处理正向与反向地理编码请求，对接配置的转换服务。是否支持离线转换取决于产品及提供方能力，不能由本仓库的请求入口推定。

| 目录 | 查找内容 |
|------|----------|
| `services/location_geocode/geocode/` | 转换服务入口、连接与请求处理 |
| `services/location_locator/locator/` | 面向客户端的地理编码入口及服务转发 |
| `frameworks/native/locator_sdk/` | SDK 调用与地理编码结果回调 |

## 关键入口

以下入口均在 `services/location_geocode/geocode/`，按转换方向检索当前实现。

| 入口 | 适用问题 |
|------|----------|
| `GeoConvertService::GetAddressByLocationName` | 按位置名称发起正向地理编码 |
| `GeoConvertService::GetAddressByCoordinate` | 按坐标发起反向地理编码 |
| `GeoConvertService::CheckGeoConvertAvailable` | 转换服务配置及安装状态检查，不保证请求成功 |

## 修改边界与验证

- 修改请求或返回数据时，核对名称/坐标、语言、结果数量及错误返回的约定；联读 [Locator 代码导览](locator-service.md)和 [SA/IPC 约束](../expert/sa-ipc-constraints.md)。
- 修改连接与回调时，覆盖提供方未安装、不可连接、请求失败及回调生命周期，不能把请求已受理等同于转换成功。
- 修改反向地理编码 mock 时，分别验证模拟结果与真实提供方路径，不能用 mock 成功证明在线或离线能力。
- 测试入口为 `test/location_geocode/`；客户端回调还需检查 `test/location_locator/` 中相关用例，真实转换结果需提供方集成验证。
- 构建入口：`services/location_geocode/geocode/BUILD.gn` 的 `lbsservice_geocode`；执行步骤统一见[构建和测试](../verify/build-and-test.md)。
