# 网络定位服务代码导览

## 何时读取

修改网络定位请求、NLP 服务连接、网络位置回传，或 `services/location_network/` 时读取。NLP 指网络定位服务提供方，具体能力取决于产品配置与提供方实现。

## 职责与目录

本模块接收 Locator 的请求，对接配置的 NLP 服务并回传结果。不能据此推断基站、WLAN、蓝牙定位算法都在本目录实现。

| 目录 | 查找内容 |
|------|----------|
| `services/location_network/network/` | 网络定位服务、提供方连接与结果回调 |
| `services/location_locator/locator/` | 请求调度及 NLP 服务配置读取 |
| `test/location_network/` | 网络定位服务与 IPC 入口用例 |

## 关键入口

以下入口均在 `services/location_network/network/`，按请求发送或结果接收方向选择入口。

| 入口 | 适用问题 |
|------|----------|
| `NetworkAbility::SendLocationRequest` | 接收 Locator 下发的请求 |
| `NetworkAbility::RequestNetworkLocation` | 向 NLP 提供方发送请求 |
| `NetworkCallbackHost::OnLocationReport` | 接收提供方位置并回传 |

## 修改边界与验证

- 修改提供方连接或请求时，核对产品配置、服务是否安装、连接失败及远端死亡处理；不能把提供方缺失误判为定位算法错误。
- 修改请求参数或回调数据时，同时检查发送方与接收方协议；接口兼容约束见 [SA/IPC 文档](../expert/sa-ipc-constraints.md)。
- 修改位置结果和 mock 路径时，联读 [Locator 代码导览](locator-service.md)及[权限约束](../expert/permission-constraints.md)，验证模拟状态与正常结果处理不混用。
- 测试入口为 `test/location_network/`；真实 NLP 提供方的连接、结果与断连恢复还需集成环境验证。
- 构建入口：`services/location_network/network/BUILD.gn` 的 `lbsservice_network`；执行步骤统一见[构建和测试](../verify/build-and-test.md)。
