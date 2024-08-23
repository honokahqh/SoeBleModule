# 蓝牙发射模块

## 硬件相关

STM32f070f6,32KB flash, 6KB ram, 最高48MHz.  

## 分区表

0x08000000-0x08007FFF为STM32f070f6的Flash范围, 1KB为一页  
| Name        | Type | Offset | Size   |
|-------------|------|--------|--------|
| fac         | boot | 0x0000 | 0x2000 |
| app         | app  | 0x2000 | 0x4000 |
| flashdata   | data | 0x7400 | 0x0C00 |

fac固件大小为 8 KB,三页用于存储mbs寄存器信息-固件OTA信息  
编译fac时,选择option->Target->IROM1,设置FLASH(rx):ORIGIN = 0x08000000, LENGTH = 0x2000,打开main.h,#define IsApp 0  
编译app时,选择option->Target->IROM1,设置FLASH(rx):ORIGIN = 0x08000000, LENGTH = 0x4000,打开main.h,#define IsApp 1  
烧录fac时选择擦除全片,而后烧录app选择部分擦除,而后即可正常运行  

> 为了兼容老版本boot,fac的大小被固定在 8 KB,因此boot被砍了蓝牙控制相关的功能,虽然可以进行mbs通讯,但没有实际控制效果

## OTA

使用Ymodem协议使用RS485进行传输

## modbus coil reg定义

- coil  
1. 12000 音乐播放 0:暂停 1:播放
2. 12001 工作模式 0:蓝牙 1:有线
3. 12002 切换上一首 写1有效,自行清0
4. 12003 切换下一首 写1有效,自行清0
5. 12004 切指定index的歌 写1有效,自行清0 
6. 12005 播放模式 0:列表循环 1:单曲循环
7. 12006 已弃用
8. 12007 蓝牙重连 写1有效,自行清0
9. 12008 设备重启 写1有效,自行清0
10. 12009 蓝牙连接状态 0:未连接 1:已连接
11. 12010 播放状态 0:暂停 1:播放

- reg
1. 50000 音量 0~30
2. 50001-5006 蓝牙MAC
3. 50007 切歌index,搭配coil 12004使用
4. 50008 当前播放索引
5. 50009 音乐文件总数
6. 50010 当前文件时间长度
7. 50011 当前播放进度(仅蓝牙断开后可以查询到)


