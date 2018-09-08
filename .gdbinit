# 连接远程调试端口
target remote :2331
# 在main函数开头设置断点
b main
# 复位
monitor reset
# 运行
continue
