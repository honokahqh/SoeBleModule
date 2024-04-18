# 红外发射模块

## 硬件相关

STM32f070f6,32KB flash, 6KB ram, 最高48MHz.  

## 分区表

0x08000000-0x08007FFF为STM32f070f6的Flash范围, 1KB为一页  
name            type        offset      size 
fac,            boot,       0x0000      0x2000  
app,            app,        0x2000      0x4000  
flashdata,      data,       0x7400      0x0C00
fac固件大小为 8 KB,三页用于存储mbs寄存器信息-固件OTA信息  
编译fac时,选择option->Target->IROM1,设置FLASH(rx):ORIGIN = 0x08000000, LENGTH = 0x2000,打开main.h,#define IsApp 0  
编译app时,选择option->Target->IROM1,设置FLASH(rx):ORIGIN = 0x08000000, LENGTH = 0x4000,打开main.h,#define IsApp 1  
烧录fac时选择擦除全片,而后烧录app选择部分擦除,而后即可正常运行  

> 为了兼容老版本boot,fac的大小被固定在 8 KB,目前v2.45 release 已使用 7.8 KB,若打开log后编译会报错

## OTA

使用Ymodem协议使用RS485进行传输

## modbus coil reg定义

- coil  
1. 12002 空调开关 0:关机 1:开机
2. 12007 学习模式 写一有效 开始学习

- reg
1. 51010 空调品牌
2. 51011 空调模式
3. 51012 空调温度
4. 51013 空调风速
5. 59000 modbus地址
6. 59001 modbus版本

## 软件相关

> 空调控制指令只能使用modbus 05H 06H ,使用 0FH 10H 指令只会修改参数,并不会发送实际的空调控制指令 

1. 空调ID  
设置寄存器51010后,软件会设置对应空调品牌,而后尝试发送风速5指令,若ACK有效则会保存标志位,而后发送风速0,1,2,3会被替换为0,1,3,5  
2. 学习模式
写线圈12007后,软件会开始10s的空调品牌检测,通过遥控器向模块黑色接收头发送空调命令后,会返回可能的红外编码