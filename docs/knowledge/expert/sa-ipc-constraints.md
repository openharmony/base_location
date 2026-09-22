# SA、IPC 与回调边界

适用：修改 SA 注册或发现、IPC 协议、序列化、回调和生成接口。涉及调用身份时同时阅读 [权限边界](permission-constraints.md)。以下路径均相对仓库根目录。

## 从哪里查

| 目录或契约源文件 | 关键入口 | 用途 |
| --- | --- | --- |
| `services/location_locator/locator/` | `LocatorAbility::OnStart`、`Init` | Locator 生命周期与服务发布 |
| `frameworks/native/locator_sdk/` | `LocatorImpl::GetProxy` | 服务发现与代理获取 |
| `sa_profile/`、`services/etc/init/locationsa.cfg` | SA 配置与 `locationhub` 进程配置 | 核对 SA 标识、库名、进程名及加载方式 |
| `frameworks/native/locator_sdk/ILocatorService.idl`、`frameworks/native/locator_sdk/type/IDataTypes.idl` | 接口与类型定义 | Locator IPC 方法、事务码、参数方向和类型的构建输入 |
| `services/location_locator/callback/` | `LocatorCallbackProxy::OnLocationReport` | 位置回调发送及同步/异步选项 |
| `frameworks/native/locator_sdk/` | `LocatorCallbackHost::OnRemoteRequest` | 回调接收、接口标识校验与分发 |
| `frameworks/base_module/` | `Location::Marshalling`、`Unmarshalling`；`RequestConfig::Marshalling`、`Unmarshalling` | 位置与请求对象的序列化契约 |

SA 常量的声明由平台 `system_ability_definition.h` 提供；`sa_profile/` 是本仓加载配置，不能当作常量定义的唯一来源。

## 必须保持的契约

| 修改什么 | 必须保持什么 | 核查与验证 |
| --- | --- | --- |
| SA 标识、服务/进程名或库名 | 注册、发现、加载配置及构建产物一致 | 联查上表入口及对应 `BUILD.gn`；验证服务可加载、发现和重新连接 |
| IPC 方法、事务码或参数 | 客户端与服务端的协议、类型、参数顺序和错误语义兼容 | 对照 IDL；手写接口还要检查两端的 proxy/stub 与接口声明；验证正常请求和非法输入 |
| 序列化字段或列表 | 写入与读取对应，保留类型、顺序、空值、数量边界和失败处理 | 检查 `Marshalling` / `Unmarshalling` 两侧；补正常值、边界值和畸形数据测试 |
| 回调注册、分发或线程切换 | 回调对象生命周期、注销、顺序与错误处理保持正确 | 验证注册/注销、远端死亡、重复请求；不能直接把已有同步回调改成异步 |

这些是默认兼容约束，不表示相关标识完全不能修改。任务明确包含契约变更时，需说明受影响调用方和迁移方案，并同步更新依赖与验证。

## 同步、异步与性能的适用范围

- 当前接口同时存在同步输出（如 `ILocatorService::GetCacheLocation`）和回调结果；不能统一写成“所有定位结果都异步返回”。
- `LocatorCallbackProxy::OnLocationReport` 通常使用异步消息，但后台代理分支使用同步消息唤醒应用。修改时保留该行为，或在明确调整该机制的任务中验证替代方案。
- 工程要求：新增同步 IPC、文件/数据库/网络 I/O 或等待操作前，检查所在 IPC/回调线程、持有锁及是否存在重入等待；把可能长期阻塞的工作移交合适执行上下文，并验证取消、顺序和对象生命周期。
- 性能建议：高频回调新增逐次日志、格式化或事件上报时，说明频率与开销，必要时限频或采样。现有 GNSS 回调包含诊断日志与事件记录，不能仅因“回调禁止日志”而删除；涉及优化时保留故障可诊断性并对比耗时。

## 生成代码边界

- Locator IPC：`frameworks/native/locator_sdk/BUILD.gn` 的 `idl_gen_interface` 使用该目录的 IDL 生成接口代码。修改 IDL 和相关手写实现，再构建核验；不要手改构建目录中的生成 proxy/stub。
- Taihe：`frameworks/ets/taihe/locator_sdk/idl/ohos.geoLocationManager.taihe` 是本仓生成输入，对应 `BUILD.gn` 使用 `run_taihe` 生成绑定；`src/` 中的手写实现与生成产物需区分处理。
- 生成目录和产物名从当前 `BUILD.gn` 获取，不在知识库复制一份易过期的产物清单。

## 验证入口

按改动模块选择 `test/location_locator/`、`test/location_gnss/` 等目录下的入口、stub、回调测试及 `test/fuzztest/` 中对应 IPC/序列化模糊测试。执行方法见 [构建与验证](../verify/build-and-test.md)；编译通过不能替代设备上的加载、通信和回调验证。
