# 构建和测试验证

## 环境与依据

本仓库是 OpenHarmony 组件，不是独立构建工程：仓内没有 `build.sh`，且 GN 文件依赖源码树中的 `build/`、驱动、IPC 等组件。单独克隆到 Windows 后可以编辑、检索和检查文档，不能据此宣称已完成系统编译或设备测试。

编译需使用与目标分支、产品匹配的完整 OpenHarmony 源码树及其支持的 Linux 构建环境，将本仓放在 `base/location/location`。产品名、工具链、构建参数以该源码树的构建说明为准。`bundle.json` 是组件目标索引，`config.gni` 定义组件路径与特性开关，目标的最终依赖以各目录 `BUILD.gn` 为准。

以下为执行模板；GN 目标已对照本仓定义核验，完整树的构建入口、输出路径和设备环境仍需现场确认。模板不构成已经执行的验证记录。

## 编译目标与命令

以下标签相对于 `//base/location/location/`；传给构建工具时补齐该前缀。

| 修改范围 | 组件目标 |
|---|---|
| Locator | `services/location_locator/locator:lbsservice_locator` |
| GNSS | `services/location_gnss/gnss:lbsservice_gnss` |
| 网络定位 | `services/location_network/network:lbsservice_network` |
| 地理编码 | `services/location_geocode/geocode:lbsservice_geocode` |
| 被动定位 | `services/location_passive/passive:lbsservice_passive` |
| Native SDK / 围栏 | `frameworks/native/locator_sdk:locator_sdk` / `frameworks/native/geofence_sdk:geofence_sdk` |
| JS NAPI | `frameworks/js/napi:geolocation` / `frameworks/js/napi:geolocationmanager` |
| C API / Taihe | Native C 接口模块目标（`frameworks/native/` 下，目标名与目录同名）/ `frameworks/ets/taihe/locator_sdk:locationframework_taihe` |

在完整源码树根目录执行 Bash 模板，先替换占位值，并确认该分支的 `build.sh --help` 支持完整 GN 标签：

```sh
PRODUCT='<实际产品名>'
TARGET='//base/location/location/services/location_locator/locator:lbsservice_locator'
./build.sh --product-name "$PRODUCT" --build-target "$TARGET"
# 编译 Locator 测试；这一步不会执行测试。
./build.sh --product-name "$PRODUCT" --build-target //base/location/location/test/location_locator:unittest
```

涉及多个模块时分别覆盖相关目标；修改特性开关或条件编译时核查受影响的开启、关闭配置。公共接口变化还需构建相关调用方，单个服务目标成功不足以证明兼容。

## 测试入口与设备执行

下表目录中均有 `BUILD.gn`，去掉目录末尾 `/` 后加 `:unittest` 为测试组；具体二进制以 `ohos_unittest` 定义及实际产物为准。

| 测试目录 | 二进制目标 |
|---|---|
| `test/location_locator/` | `LocatorServiceAbilityTest` |
| `test/location_gnss/` | `GnssAbilityTest` |
| `test/location_geocode/` | `GeocodeServiceAbilityTest` |
| `test/location_network/` | `NetworkAbilityTest` |
| `test/location_passive/` | `PassiveAbilityTest` |
| `test/location_common/` | `LocationCommonTest` |
| `test/location_geofence/` | `GeofenceSdkTest` |
| `test/location_manager/` | `LocatorManagerAbilityTest` |
| `test/location_mock_ipc/` | `LocationMockIpcTest` |

这些测试将 `module_out_path` 设置为 `location/location`；它不是完整产物路径，需在实际产品输出目录定位二进制。GNSS、Geocode、Network、Passive 的关闭特性分支没有测试源文件，不能把空目标构建成功记为用例通过。模糊测试另见 `test/fuzztest/BUILD.gn` 的 `:fuzztest` 组，按变更选择解析或 IPC 入口及已有语料，不把编译 fuzz 目标等同于完成 fuzz 运行。

设备测试需要与构建架构、系统版本及依赖库匹配的开发设备、`hdc` 连接和测试所需权限。用例可能修改定位开关、访问令牌等状态；在允许执行这些测试的设备上运行，记录并恢复受影响状态。若项目提供测试框架配置，优先遵循其部署和运行流程。

手动运行示例（主机 Bash；占位值全部替换后使用）：

```sh
OUT_DIR='<本次产品构建输出目录>'
find "$OUT_DIR" -type f -name LocatorServiceAbilityTest
TEST_BINARY='<上一步确认的本次测试二进制完整路径>'
DEVICE_ID='<hdc list targets 中选定的设备标识>'
RUN_ID='<本次运行的唯一名称，仅字母数字和短横线>'
DEVICE_DIR="/data/local/tmp/location-test-$RUN_ID"
hdc list targets
hdc -t "$DEVICE_ID" shell "mkdir -p $DEVICE_DIR"
hdc -t "$DEVICE_ID" file send "$TEST_BINARY" "$DEVICE_DIR/LocatorServiceAbilityTest"
hdc -t "$DEVICE_ID" shell "chmod 700 $DEVICE_DIR/LocatorServiceAbilityTest"
hdc -t "$DEVICE_ID" shell "$DEVICE_DIR/LocatorServiceAbilityTest --gtest_list_tests"
hdc -t "$DEVICE_ID" shell "$DEVICE_DIR/LocatorServiceAbilityTest --gtest_output=xml:$DEVICE_DIR/result.xml; result=\$?; echo REMOTE_TEST_EXIT=\$result; exit \$result"
hdc -t "$DEVICE_ID" file recv "$DEVICE_DIR/result.xml" "./location-test-$RUN_ID.xml"
```

若存在多个同名产物，先按本次产品、架构和构建记录确认选择。缺库、无权限、设备服务不可用属于环境失败，需解决或报告。通过需设备端 `REMOTE_TEST_EXIT=0`、XML 中失败/错误数为零，且预期范围内实际执行用例数大于零；检查并说明跳过项，不能仅凭 `hdc` 返回成功或输出文件存在判定。需筛选时先列举用例再传 `--gtest_filter`，记录筛选条件并确认匹配数量大于零。

## 按修改类型确定最小验证

| 修改类型 | 最小验证与交付证据 |
|---|---|
| 仅 `AGENTS.md` / 知识文档 | 在仓库根运行 `git diff --check`；检查 Markdown 链接、引用目录及关键符号真实存在，核对描述与当前入口职责一致；不要求系统编译 |
| 服务、SDK 或绑定实现 | 编译受影响目标，运行相关测试组或明确列出的用例；说明测试未覆盖的行为 |
| 权限、位置结果、后台访问 | 增加或选取授权成功、无权限拒绝、适用的模糊定位及后台条件用例；核对服务端入口与结果返回路径 |
| 公共 API、IDL、IPC 数据 | 检查签名、枚举/错误码、数据字段及序列化兼容性；构建调用方与服务方，涉及生成接口时重新生成并构建，记录该分支可用的兼容性检查结果 |
| IPC 输入解析 | 相关单元测试及异常/边界输入；存在对应 fuzz 入口时按项目流程执行并记录运行范围 |
| GN / 特性 / SA 配置 | 相关配置的编译；SA 变更还需设备上验证服务注册、发现和受影响请求 |

文档核查可用 `rg --files <目录>` 定位文件、`rg -n -F '<符号名>' <目录>` 查找入口；文本命中后还需阅读定义，不能以同名字符串替代语义核验。源码修改按现有工程规则处理格式和诊断；未配置 LSP 或静态分析工具时，不得把“未运行”写成“无错误”。

## 结果记录与维护

交付时分别记录已通过、失败、未执行和不适用项，并附执行目录、产品/特性配置、命令、关键结果及日志或产物位置。缺少完整源码树、工具链、设备或用例时，明确缺项及后续验证动作；不能仅凭阅读代码、清单勾选或编译成功声明测试通过。

本页集中维护验证命令，不在各模块导览重复复制。改变目标名、测试目录或生成方式时，依据 `bundle.json` 和相关 `BUILD.gn` 同步核对本页；知识页只保留验证入口，不维护易变的逐用例清单。
