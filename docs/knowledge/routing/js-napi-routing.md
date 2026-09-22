# 多语言接口绑定路由

## 何时读取

修改 JS NAPI、ETS/Taihe、CJ/FFI 或 Native C 接口绑定、异步结果及事件订阅时读取。这些绑定分别位于 `frameworks/js/napi/`、`frameworks/ets/taihe/`、`frameworks/cj/` 及 `frameworks/native/` 下的 Native C 接口目录；先确认任务涉及的语言、API 和调用方式。

## 目录与入口

| 场景 | 目录 | 关键入口或查找对象 |
|------|------|--------------------|
| JS 模块导出 | `frameworks/js/napi/source/` | `Init` / `InitManager`，分别注册 `geolocation` / `geoLocationManager` |
| 单次定位、持续订阅 | `frameworks/js/napi/source/` | `GetCurrentLocation`、`On` / `Off` |
| 位置开关 | `frameworks/js/napi/source/` | `IsLocationEnabled`、`EnableLocation` / `DisableLocation` |
| Native 请求与服务调用 | `frameworks/native/locator_sdk/` | 从当前绑定函数追踪 SDK 实际入口 |
| ETS/Taihe 接口 | `frameworks/ets/taihe/locator_sdk/` | `idl/` 的接口定义、`src/` 的实现及回调 |
| CJ/FFI 接口 | `frameworks/cj/` | `FfiOHOSGeoLocationManagerGetCurrentLocation` |
| Native C 接口 | `frameworks/native/` 下的 Native C 接口目录 | `OH_Location_StartLocating` / `OH_Location_StopLocating`；声明见 `interfaces/c_api/` |

表中 NAPI 入口是 `OHOS::Location` 命名空间内的自由函数，不是类方法；先在模块导出表中确认 JS 名称与 C++ 入口的对应关系。

## 修改边界

- 修改导出、参数或错误返回时，分别核对旧模块与管理模块的 API 约定，不能因共用部分实现就假定两者签名及错误处理一致。
- 修改异步结果、订阅与取消时，检查回调引用、环境销毁及取消后的通知，保持 Promise/回调完成和资源释放的约定。
- 修改绑定层权限相关行为时，联读[权限约束](../expert/permission-constraints.md)及 [Locator 代码导览](../code_map/locator-service.md)；参数检查或客户端检查不能替代服务端鉴权。
- 修改围栏 API 时，继续按[围栏路由](geofence-routing.md)查找对应服务；不要只修改 JS 绑定。
- 修改 Taihe 接口时，以 `frameworks/ets/taihe/locator_sdk/BUILD.gn` 的生成规则为准，修改 IDL 或受版本管理的实现；不手改构建输出中的生成绑定。
- 修改 CJ/FFI 或 Native C 接口时，核对跨语言数据布局、回调及分配/释放的配对关系；公共 C 接口声明与实现必须同步。

## 验证入口

- NAPI 构建入口是 `frameworks/js/napi/BUILD.gn` 的 `geolocation`、`geolocationmanager`；Taihe 构建入口在其 `locator_sdk/BUILD.gn`。
- CJ/FFI 与 Native C 接口的构建入口分别为 `frameworks/cj/BUILD.gn` 和 `frameworks/native/` 下 Native C 接口目录中的 `BUILD.gn`。
- Native 定位与围栏回归分别从 `test/location_locator/`、`test/location_geofence/` 定位，不能据此宣称 JS/ETS 绑定已经验证。
- 公共 API 改动还需对应语言的集成用例，覆盖无效参数、Promise/回调结果及订阅取消等受影响场景；缺少运行环境或用例时明确记录未验证范围。
- 环境、测试目标与执行方法统一见[构建和测试](../verify/build-and-test.md)。
