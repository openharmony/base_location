# 构建和测试验证

## 构建目标

### 服务组件

```sh
# 编译所有location服务
./build.sh --product-name {product_name} --build-target location

# 单独编译各服务
./build.sh --product-name {product_name} --build-target lbsservice_locator
./build.sh --product-name {product_name} --build-target lbsservice_gnss
./build.sh --product-name {product_name} --build-target lbsservice_network
./build.sh --product-name {product_name} --build-target lbsservice_geocode
./build.sh --product-name {product_name} --build-target lbsservice_passive
```

### SDK组件

```sh
./build.sh --product-name {product_name} --build-target locator_sdk
./build.sh --product-name {product_name} --build-target geolocation
./build.sh --product-name {product_name} --build-target geofence_sdk
```

### 测试组件

```sh
./build.sh --product-name {product_name} --build-target location_unittest
```

## 单元测试

### 测试目录

| 测试 | 路径 |
|------|------|
| locator测试 | `test/location_locator/` |
| gnss测试 | `test/location_gnss/` |
| geocode测试 | `test/location_geocode/` |
| network测试 | `test/location_network/` |
| passive测试 | `test/location_passive/` |
| common测试 | `test/location_common/` |
| manager测试 | `test/location_manager/` |
| fuzztest | `test/fuzztest/` |

### 运行测试

```sh
# 通过hdc执行测试
hdc shell
# 进入设备后执行测试二进制
```

## 验证通过标准

任务完成必须满足：

1. ✅ 所有修改的文件 LSP 诊断无新增 error
2. ✅ 编译成功（无链接错误）
3. ✅ 相关测试用例通过
4. ✅ 未引入公共 API 不兼容变更
5. ✅ 未违反任何约束和边界中列出的禁止事项
6. ✅ 敏感API权限检查路径未被绕过

## Done 定义

当以下全部满足时，任务视为完成：

```
[ ] LSP 诊断无新增 error
[ ] 编译通过
[ ] 单元测试通过
[ ] API兼容性检查通过
[ ] 约束检查通过
[ ] 代码格式化已执行
```
