# AGENTS.md - 位置服务代码仓知识入口

## 项目定位

本仓库对应 OpenHarmony `base/location/location`，提供设备定位、地理编码、围栏服务能力。

**核心能力：**
- GNSS全球导航卫星系统定位
- 网络定位（基站、WLAN、蓝牙）
- 地理编码/反编码
- 地理围栏
- 被动定位

---

## 代码地图

### 一级目录职责

| 目录 | 职责 | 关键路径 |
|------|------|---------|
| `services/` | SA系统能力服务实现 | location服务核心 |
| `frameworks/` | SDK框架和API适配层 | 对外接口实现 |
| `interfaces/` | 公共API定义 | C API / Inner API |
| `test/` | 单元测试和模糊测试 | 验证覆盖 |
| `sa_profile/` | SA配置描述文件 | 服务启动配置 |
| `tools/` | 命令行工具 | ohos-location |

### services/ 服务模块

| 模块 | 职责 | 代码路径 |
|------|------|---------|
| **location_locator** | 主定位服务，位置请求分发、缓存管理 | `services/location_locator/locator/` |
| **location_gnss** | GNSS定位实现，卫星信号处理 | `services/location_gnss/gnss/` |
| **location_network** | 网络定位，基站/WLAN定位 | `services/location_network/` |
| **location_geocode** | 地理编码/反编码服务 | `services/location_geocode/` |
| **location_passive** | 被动定位，消息订阅机制 | `services/location_passive/` |
| **location_ui** | 定位权限弹窗HAP | `services/location_ui/` |
| **utils** | 公共服务：配置解析、日志、IPC | `services/utils/` |

### frameworks/ 框架模块

| 模块 | 职责 | 代码路径 |
|------|------|---------|
| **locator_sdk** | Native定位SDK，服务端代理 | `frameworks/native/locator_sdk/` |
| **locator_agent** | 定位代理，跨进程通信 | `frameworks/native/locator_agent/` |
| **geofence_sdk** | 围栏SDK | `frameworks/native/geofence_sdk/` |
| **location_ndk** | NDK接口 | `frameworks/native/location_ndk/` |
| **js/napi** | JS NAPI绑定（geolocation/geolocationmanager） | `frameworks/js/napi/` |
| **cj** | CJ FFI绑定 | `frameworks/cj/` |
| **ets/taihe** | ETS框架绑定 | `frameworks/ets/taihe/` |
| **location_common** | 公共基础能力 | `frameworks/location_common/` |

### interfaces/ API模块

| 模块 | 职责 | 代码路径 |
|------|------|---------|
| **c_api** | C语言公共接口 | `interfaces/c_api/` |
| **inner_api** | 进程内Inner API | `interfaces/inner_api/include/` |

---

## 知识索引

稳定背景知识放在 `docs/knowledge/`。改动前按场景读取对应文件：

### 任务到路径映射

| 任务类型 | 先读 | 原因 |
|----------|------|------|
| **定位请求处理** | `docs/knowledge/code_map/locator-service.md` | 理解位置请求分发链路 |
| **GNSS定位实现** | `docs/knowledge/code_map/gnss-service.md` | 卫星定位核心逻辑 |
| **网络定位实现** | `docs/knowledge/code_map/network-location.md` | 基站/WLAN定位 |
| **地理编码服务** | `docs/knowledge/code_map/geocode-service.md` | 地址与坐标转换 |
| **围栏功能** | `docs/knowledge/routing/geofence-routing.md` | 围栏触发和回调 |
| **JS API绑定** | `docs/knowledge/routing/js-napi-routing.md` | NAPI接口实现 |
| **权限校验** | `docs/knowledge/expert/permission-constraints.md` | 位置权限检查约束 |
| **SA服务通信** | `docs/knowledge/expert/sa-ipc-constraints.md` | IPC通信禁止事项 |
| **编译验证** | `docs/knowledge/verify/build-and-test.md` | 构建和测试命令 |

### 术语触发路由

| 术语/缩写 | 含义 | 指向 |
|-----------|------|------|
| **Locator** | 主定位服务 | `services/location_locator/` |
| **GNSS** | 全球导航卫星系统 | `services/location_gnss/` |
| **Geofence** | 地理围栏 | `frameworks/native/geofence_sdk/` |
| **NAPI** | Node API绑定 | `frameworks/js/napi/` |
| **SA** | System Ability | `services/*/` |
| **SysCap** | 系统能力 | `bundle.json` |

---

## 项目约束

### 禁止事项

- **禁止** 修改SA服务ID或service name，可能导致服务无法启动
- **禁止** 在IPC回调中执行耗时操作，可能导致ANR
- **禁止** 跳过位置权限检查，位置信息属于敏感数据
- **禁止** 在GNSS定位回调中进行字符串格式化或日志打印（高频路径）
- **禁止** 修改系统能力定义（SystemCapability）而不同步更新依赖
- **禁止** 对公共接口做不兼容变更

### 架构约束

- 位置请求必须经过Locator服务分发，不得绕过
- 所有定位结果通过IPC回调返回，调用方不应阻塞
- 配置变更需通过配置解析服务，不得硬编码
- 日志输出使用HiLog，禁止使用printf

---

## 构建和验证

### 构建命令

```sh
# 全量编译
./build.sh --product-name {product_name} --build-target location

# 单独编译服务
./build.sh --product-name {product_name} --build-target lbsservice_locator
./build.sh --product-name {product_name} --build-target lbsservice_gnss

# 单独编译SDK
./build.sh --product-name {product_name} --build-target locator_sdk
./build.sh --product-name {product_name} --build-target geolocation
```

### 测试命令

```sh
# 运行单元测试
./build.sh --product-name {product_name} --build-target location_unittest
# 或通过以下测试目录
# test/location_locator/unittest
# test/location_gnss/unittest
# test/location_geocode/unittest
# test/location_network/unittest
# test/location_passive/unittest
```

### 验证通过标准

任务完成必须满足：

1. ✅ 所有修改的文件 LSP 诊断无新增 error
2. ✅ 编译成功
3. ✅ 相关测试用例通过
4. ✅ 未引入公共 API 不兼容变更
5. ✅ 未违反任何约束和边界中列出的禁止事项
6. ✅ 敏感API权限检查路径未被绕过

---

## 知识文档目录

```
docs/knowledge/
├── code_map/           # 代码地图
│   ├── locator-service.md
│   ├── gnss-service.md
│   ├── network-location.md
│   └── geocode-service.md
├── routing/            # 知识路由
│   ├── geofence-routing.md
│   └── js-napi-routing.md
├── expert/             # 专家经验
│   ├── permission-constraints.md
│   └── sa-ipc-constraints.md
└── verify/             # 验证方法
    └── build-and-test.md
```
