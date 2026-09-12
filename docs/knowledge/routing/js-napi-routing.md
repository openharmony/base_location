# JS NAPI知识路由

## 模块职责

JS NAPI层提供ArkTS/JS API到Native实现的绑定。

## 相关组件

| 组件 | 代码路径 | 职责 |
|------|---------|------|
| **geolocation** | `frameworks/js/napi/` | 位置API NAPI绑定 |
| **geolocationmanager** | `frameworks/js/napi/` | 位置管理API |
| **fence_extension_ability** | `frameworks/js/napi/fence_extension_ability/` | 围栏NAPI |
| **taihe** | `frameworks/ets/taihe/` | ETS框架支持 |

## NAPI实现关键文件

| 文件 | 导出API |
|------|--------|
| `geolocation` | `getCurrentLocation()`, `onLocationChange()`, `offLocationChange()` |
| `geolocationmanager` | `isLocationEnabled()`, `enableLocation()`, `disableLocation()` |

## 路由场景

| 场景 | 先读 |
|------|------|
| **JS API实现问题** | `frameworks/js/napi/` 下的 napi 实现文件 |
| **JS回调注册** | NAPI callbacks 目录结构 |
| **ETS特有接口** | `frameworks/ets/taihe/` |
