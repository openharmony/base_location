# 使用说明

## 基本用法

```bash
ohos-location-manager <command> [options]
```

## 命令列表

| 命令 | 说明 | 参数 | 权限 | 前置依赖 |
|------|------|------|------|----------|
| get-switch-state | 查询定位开关状态 | 无 | 无 | 无 |
| get-cached-location | 获取缓存位置 | 无 | ohos.permission.APPROXIMATELY_LOCATION | 定位开关已开启（可选） |
| enable-switch | 启用/禁用定位开关 | --enable/--disable | ohos.permission.CONTROL_LOCATION_SWITCH | 无 |
| start-locate | 启动定位并等待结果 | --timeout, --scenario, --interval | ohos.permission.APPROXIMATELY_LOCATION | 定位开关已开启（可选） |
| stop-locate | 停止定位 | 无 | ohos.permission.APPROXIMATELY_LOCATION | start-locate（必须） |

## 前置依赖说明

- **无**：该命令可直接执行，无需前置条件
- **可选**：建议先满足前置条件以确保操作成功
- **必须**：必须先执行前置命令才能成功执行

## 命令详解

### get-switch-state

查询定位开关状态，无需权限。

```bash
ohos-location-manager get-switch-state
```

**输出示例**：
```json
{
  "code": 0,
  "data": {
    "enabled": true
  }
}
```

### get-cached-location

获取系统缓存的位置信息。

```bash
ohos-location-manager get-cached-location
```

**输出示例**：
```json
{
  "code": 0,
  "data": {
    "location": {
      "latitude": 31.2304,
      "longitude": 121.4737,
      "altitude": 10.0,
      "accuracy": 50.0,
      "speed": 0.0,
      "direction": 0.0,
      "timestamp": 1234567890000,
      "sourceType": 1
    }
  }
}
```

### enable-switch

启用或禁用定位开关（需要系统权限）。

```bash
# 启用定位开关
ohos-location-manager enable-switch --enable

# 禁用定位开关
ohos-location-manager enable-switch --disable
```

**输出示例**：
```json
{
  "code": 0,
  "data": {
    "message": "Location switch enabled"
  }
}
```

### start-locate

启动定位并等待结果。采用 LRO 模式，启动定位后注册回调等待位置结果，超时后自动停止。

```bash
# 基本用法（默认超时30秒）
ohos-location-manager start-locate

# 指定超时时间
ohos-location-manager start-locate --timeout=60

# 指定定位场景
ohos-location-manager start-locate --scenario=0x0301 --timeout=60

# 指定上报间隔
ohos-location-manager start-locate --interval=5 --timeout=60
```

**参数说明**：
| 参数 | 说明 | 默认值 | 取值范围 |
|------|------|--------|----------|
| --timeout | 等待超时时间（秒） | 30 | 1-300 |
| --scenario | 定位场景 | 0x0304（日常服务） | 见场景枚举 |
| --interval | 位置上报间隔（秒） | 1 | 1-1800 |

**场景枚举**：
| 值 | 场景 |
|----|------|
| 0x0301 | 导航（SCENE_NAVIGATION） |
| 0x0302 | 轨迹追踪（SCENE_TRAJECTORY_TRACKING） |
| 0x0303 | 打车（SCENE_CAR_HAILING） |
| 0x0304 | 日常服务（SCENE_DAILY_LIFE_SERVICE） |
| 0x0305 | 无功耗（SCENE_NO_POWER） |

**输出示例**：
```json
{
  "code": 0,
  "data": {
    "location": {
      "latitude": 31.2304,
      "longitude": 121.4737,
      "altitude": 10.0,
      "accuracy": 10.5,
      "speed": 0.0,
      "direction": 180.0,
      "timestamp": 1234567890000,
      "sourceType": 1
    }
  }
}
```

### stop-locate

停止正在进行的定位。必须在 start-locate 执行期间或之后调用。

```bash
ohos-location-manager stop-locate
```

**输出示例**：
```json
{
  "code": 0,
  "data": {
    "message": "Locating stopped"
  }
}
```

## 错误码说明

| 错误码 | 说明 | 建议 |
|--------|------|------|
| 0 | 成功 | - |
| 201 | 权限拒绝 | 申请所需权限 |
| 401 | 参数错误 | 检查参数格式 |
| 3301000 | 服务不可用 | 检查定位服务是否运行 |
| 3301100 | 定位开关关闭 | 先启用定位开关 |
| 3301200 | 定位失败 | 检查GPS/网络信号 |
| 3301204 | 无缓存位置 | 先执行 start-locate |
| 3300207 | 请求超时 | 增加超时时间 |

## 示例

### 示例1：查询开关状态并获取缓存位置

```bash
# 查询开关状态
ohos-location-manager get-switch-state
# 输出: {"code":0,"data":{"enabled":true}}

# 获取缓存位置
ohos-location-manager get-cached-location
```

### 示例2：启动定位获取当前位置

```bash
# 启动定位（60秒超时）
ohos-location-manager start-locate --timeout=60
# 等待位置结果输出
```

### 示例3：启用定位开关后获取位置

```bash
# 启用定位开关（需要系统权限）
ohos-location-manager enable-switch --enable

# 启动定位获取位置
ohos-location-manager start-locate --timeout=30
```