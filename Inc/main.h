#ifndef __MAIN_H__
#define __MAIN_H__

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "stm32f0xx.h"
#include "stm32f0xx_conf.h"

#include "log.h"
#include "pt.h"
#include "Modbus_CORE.h"
#include "bsp_driver.h"
#include "ringbuffer.h"
#include "ble_driver.h"

#define IsApplication	 0
#define IsRelease        1 	// 测试中不要打开,防止死机等问题被开门狗复位
extern uint32_t sys_ms;

void delay_ms(__IO uint32_t nTime);
void system_run(void);
uint32_t millis(void);

#endif 
