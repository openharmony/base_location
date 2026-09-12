# SA服务IPC通信约束

## 核心约束

### IPC回调禁止事项

1. **禁止在回调中执行耗时操作**
   - 禁止同步IPC调用
   - 禁止文件I/O
   - 禁止数据库操作
   - 禁止网络请求

2. **避免阻塞调用方**
   - 使用异步回调模式
   - 设置合理超时时间

### 服务通信约束

1. **服务ID和名称不可随意修改**
   - SA服务ID定义在 `sa_profile/` 目录
   - 修改可能导致服务无法启动或通信失败

2. **消息序列化约束**
   - 使用统一的序列化机制
   - 避免大对象直接传递

### 常见错误模式

❌ **错误**：在IPC回调中调用同步GetLocation
```cpp
// 错误示例：可能造成ANR
void OnLocationReport::OnResult() {
    auto location = locatorService->GetLocation(); // 同步调用，危险！
}
```

✅ **正确**：使用异步回调
```cpp
// 正确示例
void RequestLocation::OnRequest() {
    locatorService->RequestLocation(callback); // 异步
}
```

### ANR风险点

| 操作 | 风险级别 | 说明 |
|------|---------|------|
| 同步获取位置 | 高 | 可能长时间阻塞 |
| 数据库操作 | 高 | I/O时间不确定 |
| 日志打印 | 中 | 大量日志影响性能 |
| 字符串格式化 | 低 | 建议优化格式 |
