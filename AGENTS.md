# AGENTS.md - 位置服务开发与知识入口

## 适用范围与使用方式

本文件适用于整个仓库，面向修改代码、测试或文档的 Agent。本仓库在完整 OpenHarmony 源码树中对应 `base/location/location`，提供定位、地理编码和围栏服务；独立克隆不包含完整系统构建环境。

1. 修改前根据下表中的任务、路径或术语读取相关知识；命中多类时合并阅读。
2. 简要说明任务类别、已读文档、需要保持的约束和拟执行的验证，不必重复整篇文档。
3. 进入目标目录时检查是否存在更局部的 `AGENTS.md`，并读取其目录范围内适用的指引。
4. 知识文档提供导航与约束，具体实现以当前源码为准。入口失效或文档与源码冲突时，先核实并说明差异；不能据此直接删除权限、兼容性或诊断逻辑。

## 按任务、路径与术语查找

下表路径均相对仓库根目录。入口函数放在对应知识页，用于继续检索源码，不在此重复完整调用链。

| 任务 / 路径 / 术语触发条件 | 代码目录与职责 | 必读知识 |
| --- | --- | --- |
| 定位请求、缓存、请求分发、Locator；修改 `services/location_locator/` | `services/location_locator/locator/`：定位服务与请求管理 | [Locator](docs/knowledge/code_map/locator-service.md)；涉及请求或结果访问时加读权限约束 |
| GNSS、卫星、NMEA、HDI；修改 `services/location_gnss/` | `services/location_gnss/gnss/`：GNSS 驱动接口接入与状态管理 | [GNSS](docs/knowledge/code_map/gnss-service.md)；改回调或驱动通信时加读 IPC 约束 |
| 网络定位、NLP；修改 `services/location_network/` | `services/location_network/network/`：网络定位服务接入 | [网络定位](docs/knowledge/code_map/network-location.md) |
| 地理编码、GeoConvert、地址坐标转换；修改 `services/location_geocode/` | `services/location_geocode/geocode/`：地理编码服务接入 | [地理编码](docs/knowledge/code_map/geocode-service.md) |
| 围栏、Geofence、Fence、Beacon；修改 `frameworks/native/geofence_sdk/` 或围栏扩展 | `frameworks/native/geofence_sdk/`、`services/location_locator/`、`services/location_gnss/`：按围栏类型选择入口 | [围栏路由](docs/knowledge/routing/geofence-routing.md)及权限约束 |
| JS/NAPI、ArkTS、ETS/Taihe、CJ/FFI、Native API；修改 `frameworks/js/`、`frameworks/ets/`、`frameworks/cj/` 或 `frameworks/native/` 下 Native C 接口目录 | 各语言绑定与 C 接口适配 | [API 绑定路由](docs/knowledge/routing/js-napi-routing.md)；跨进程接口变化加读 IPC 约束 |
| 权限、Token、精确/模糊定位、后台定位；修改权限判断或位置返回路径 | `frameworks/location_common/common/`、`services/location_locator/locator/`：权限工具与服务端判断 | [权限约束](docs/knowledge/expert/permission-constraints.md) |
| SA、IPC、IDL、序列化、注册/回调生命周期；修改 `sa_profile/`、`frameworks/native/locator_sdk/`、`frameworks/native/locator_agent/` 或 IPC 接口 | SA 配置、Native SDK 与跨进程契约 | [SA / IPC 约束](docs/knowledge/expert/sa-ipc-constraints.md) |
| 被动定位、Passive；修改 `services/location_passive/` | 被动定位服务，结合 Locator 的分发入口检索 | [Locator](docs/knowledge/code_map/locator-service.md)及 IPC 约束 |
| 公共数据类型、接口契约；修改 `interfaces/` 或 `frameworks/base_module/` | `interfaces/c_api/`、`interfaces/inner_api/`：公开/内部接口；`frameworks/base_module/`：基础数据类型。Inner API 不等于仅进程内调用 | IPC 约束；涉及位置数据访问时加读权限约束 |
| SysCap、功能开关、组件依赖、编译测试；修改 `bundle.json`、`config.gni`、`BUILD.gn` 或 `test/` | 组件描述、产品裁剪与测试目标 | [构建与验证](docs/knowledge/verify/build-and-test.md) |
| 定位弹窗、资源、工具；修改 `services/location_ui/`、`services/utils/`、`tools/` | UI、公共工具/资源及命令行工具 | 对应目录实现；定位授权变化加读权限约束，工具操作先读其使用文档 |
| HiLog、HiSysEvent、DFX、卡顿、回调耗时；修改日志或事件记录 | 相关模块及 `services/location_locator/hisysevent.yaml` | IPC 约束中的回调与诊断规则 |

所有改动均按[构建与验证](docs/knowledge/verify/build-and-test.md)选择最低必要检查；纯文档修改不要求整机编译。

## 修改边界

以下是修改规则，不是对所有现有实现的穷尽描述。性能建议不能替代权限与契约检查。

| 触发条件 | 必须保持的边界 / 行动 |
| --- | --- |
| 修改定位、缓存、围栏或后台访问 | 保留服务端调用者身份和权限校验；SDK 检查不能替代服务端鉴权。按权限文档区分不同接口及现有特权路径，不推广例外。 |
| 修改公共 API 或 IPC | 核对签名、错误码、回调生命周期、IPC 编号与序列化读写双方；不得把不兼容变更混入普通修复。具体检查见 IPC 文档。 |
| 修改 SA ID/名称、SysCap、功能开关或依赖 | 核查注册、发现、配置及消费者；依赖变化同时核对 `bundle.json` 和相关 `BUILD.gn`，不得只改单个引用。 |
| 修改生成接口 | 从 `frameworks/native/locator_sdk/ILocatorService.idl`、其 `type/` 数据定义或 `frameworks/ets/taihe/locator_sdk/idl/` 及对应 `BUILD.gn` 确认来源；修改源定义并重新生成，不手改构建输出目录中的代理、桩或绑定产物。 |
| 修改 IPC/GNSS 回调或日志 | 检查新增操作是否阻塞回调、延长持锁时间或造成同步重入；按执行频率评估逐次日志和格式化开销。沿用模块现有日志设施，不为满足笼统性能建议删除必要诊断，不扩大位置等敏感数据的日志可见范围。 |
| 新增第三方依赖或导入代码 | 核对来源、许可和组件依赖；保留已有版权与许可声明。 |
| 涉及公共契约不兼容、权限/信任边界变化、数据格式迁移或新外部依赖 | 先说明影响、兼容方案及验证范围；若超出本次授权范围，取得维护者确认后再实施。已有明确授权不重复确认。 |
| 设备测试、定位开关、Mock 或权限状态操作 | 仅在已授权测试设备上进行，确认设备身份和恢复方案；刷机、清数据等破坏性操作需要明确授权，不视为普通测试的默认步骤。 |

## 验证与交付

- 先说明环境：独立仓库可做静态核查；系统构建和设备测试需要完整源码树、匹配产品及测试设备。命令与目标集中维护在[构建与验证](docs/knowledge/verify/build-and-test.md)。
- 文档改动检查路径、入口符号和链接；代码改动按模块选择构建与测试；接口、权限与生成代码改动增加对应专项检查。
- 交付时报告修改范围、实际执行的命令与结果、未执行项目及原因。只有对应检查有证据通过才能标为通过；“未执行”“构建成功”“测试通过”分别陈述。

## 知识维护约定

- 维护粒度为“目录＋少量关键入口函数＋修改约束＋验证入口”，不复制实现步骤、源码行号、内部辅助函数清单或完整调用链。
- 模块入口和职责只在对应知识页维护；根文件维护路由与共性规则，验证命令只在验证页维护。
- 改动模块边界、入口名称、契约或测试目标时同步检查受影响知识页；内部重构且上述信息不变时无需更新文档。
- 新知识必须能在源码、接口定义、构建配置或实际验证记录中确认；未证实的推测明确标为待核实，不写成既定能力或强制规则。
- 一次性审查报告、运行日志与历史评分不作为长期知识正文，避免与当前源码事实混淆。
