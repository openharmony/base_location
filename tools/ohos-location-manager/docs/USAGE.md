# 使用说明

## 基本用法

```bash
ohos-location <command> [options]
```

## 命令列表

| 命令 | 说明 | 参数 | 权限 | 前置依赖 |
|------|------|------|------|----------|
| `help` | 显示帮助信息 | 无 | 无 | 无 |
| `is-enabled` | 查询位置开关是否开启 |   | 无 | 无 |
| `enable` | 启用位置开关 |  `--userId <userId>`（可选）,   | 无 | 无 |
| `disable` | 禁用位置开关 |  `--userId <userId>`（可选）,   | 无 | 无 |
| `get-last-approximate-location` | 获取缓存的最后已知粗略位置 |   | 无 | 无 |
| `get-last-precise-location` | 获取缓存的最后已知精确位置 |   | 无 | 无 |
| `get-current-approximate-location` | 获取当前粗略位置信息 | `--priority <accuracy\|speed>`（可选）, `--timeout <milliseconds>`（可选）,    | 无 | 无 |
| `get-current-precise-location` | 获取当前精确位置信息 | `--priority <accuracy\|speed>`（可选）, `--timeout <milliseconds>`（可选）,    | 无 | 无 |


**参数说明：**

### `--priority <accuracy\|speed>`
- **定位优先级**
- `accuracy` - 精度优先模式
- `speed` - 速度优先模式

### `--timeout <milliseconds>`
- **定位时间间隔**
- 单位：毫秒
- 默认值：3000
- 合法范围：1000 - 60000

### `--userId <userId>`
- **用户 ID**（仅 `enable`和 `disable`命令）
- 用于多用户场景
- 默认使用当前用户

**前置依赖说明：**
- **无**：该命令可直接执行，无需前置条件
- **建议**：建议先执行前置命令以确保最佳执行效果

## 输出格式

### 成功输出

```json
{
  "success": true,
  "data": {
    "isEnabled": true,
    "errCode": 0,
    "errMsg": "success"
  }
}
```

### 失败输出

```json
{
  "success": false,
  "error": {
    "code": "SERVICE_UNAVAILABLE",
    "message": "Failed to get LocatorImpl instance",
    "suggestion": "Ensure location service is available"
  }
}
```

## 示例

### 示例 1：查询位置开关状态

```bash
# 查询位置开关状态（简化输出）
ohos-location is-enabled

# 查询位置开关状态（JSON 格式输出）
ohos-location is-enabled
```

**输出示例：**
```json
{
  "success": true,
  "data": {
    "isEnabled": true,
    "errCode": 0,
    "errMsg": "success"
  }
}
```

### 示例 2：启用位置开关

```bash
# 启用位置开关
ohos-location enable

# 禁用位置开关
ohos-location disable

# 为指定用户启用位置开关
ohos-location enable --userId=100
```

**输出示例：**
```json
{
  "success": true,
  "data": {
    "enable": true,
    "errCode": 0,
    "errMsg": "success"
  }
}
```

### 示例 3：获取缓存的最后已知位置

```bash
# 获取缓存位置
ohos-location get-last-approximate-location

# JSON 格式输出
ohos-location get-last-approximate-location
```

**输出示例：**
```json
{
  "success": true,
  "data": {
    "location": {
      "latitude": 39.9042,
      "longitude": 116.4074,
      "altitude": 50.5,
      "accuracy": 10.0,
      "time": 1704067200000
    },
    "errCode": 0,
    "errMsg": "success"
  }
}
```

### 示例 4：获取当前位置

```bash
# 使用默认参数启动定位（精度优先模式）
ohos-location get-current-approximate-location

# 高精度模式定位
ohos-location get-current-approximate-location --priority=accuracy --timeout=3000

# 速度优先模式
ohos-location get-current-approximate-location --priority=speed --timeout=3000

```

**输出示例：**
```json
{
  "success": true,
  "data": {
    "taskId": "task_1704067200123",
    "status": "started",
    "priority": "high",
    "timeInterval": 500,
    "fixNumber": 5,
    "errCode": 0,
    "errMsg": "location started"
  }
}
```

**回调输出示例（定位结果）：**
```json
{"event":"location_report","task_id":"task_1704067200123","location":{"latitude":39.9042,"longitude":116.4074,"altitude":50.5,"accuracy":10.0,"time":1704067200000}}
```

### 示例 5：完整使用流程

```bash
# 1. 查询位置开关状态
ohos-location is-enabled

# 2. 如果位置开关未开启，先启用
ohos-location enable

# 3. 再次确认位置开关状态
ohos-location is-enabled

# 4. 获取缓存位置
ohos-location get-last-precise-location

# 5. 启动高精度定位
ohos-location get-current-precise-location --priority=accuracy --timeout=3000

```

## 错误码说明

| 错误码 | 说明 | 建议操作 |
|--------|------|----------|
| 0 | 成功 | - |
| 401 | 参数无效 | 检查参数格式和范围 |
| 201 | 权限错误 | 检查应用权限配置 |
| 801 | 服务不可用 | 确保位置服务正常运行 |

## 注意事项

1. **权限要求**：大多数位置查询操作无需特殊权限，但部分操作可能需要 `ohos.permission.LOCATION` 权限
2. **位置开关**：执行定位操作前建议先检查位置开关状态，确保位置开关已开启
3. **定位模式选择**：
   - 精度优先模式（accuracy）会消耗更多电量，优先返回精度更高的定位结果
   - 速度优先模式（speed）同时发起网络定位和GNSS定位，返回最先定位成功的结果
4. **LRO 模式**：`get-current-approximate-location` 命令支持异步回调，定位结果会实时输出到标准输出
5. **多用户支持**：`enable` 命令支持为不同用户设置位置开关（通过 `--userId` 参数）