#include "bsp_driver.h"

uart_state_t uart_ir_state, uart_rs485_state;

#pragma push
#pragma O0
/**
 * uart_init
 * @brief 初始化UART通信，配置USART1和USART2。
 * @note 设置USART的波特率、字长、停止位等参数，并使能接收中断。
 * @date 2024-01-31
 */
void uart_init()
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

	NVIC_InitTypeDef NVIC_InitStructure;

	/* USART1中断配置 */
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPriority = 0x01;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	/* USART2中断配置 */
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_Init(&NVIC_InitStructure);

	/* 初始化并启动USART1和USART2 */
	USART1->BRR = 0x1388; // baud 9600   115200:0x1A1
	USART1->CR1 = 0x2D;	  // Tx Rx RxIE Enable

	USART2->BRR = 0x1A1; // baud 9600
	USART2->CR1 = 0x2D;	  // Tx Rx RxIE Enable
}

/**
 * UartBleSendData
 * @brief 通过USART1发送数据。
 * @param data 要发送的数据指针。
 * @param len 要发送的数据长度。
 * @date 2024-01-31
 */
void UartBleSendData(uint8_t *data, uint8_t len)
{
	for (uint8_t i = 0; i < len; i++)
	{
		USART2->TDR = data[i] & 0xFF;
		while ((USART2->ISR & USART_FLAG_TXE) == RESET)
			;
	}
}

/**
 * UartMbsSendData
 * @brief 通过USART2发送数据，并控制RS485传输方向。
 * @param data 要发送的数据指针。
 * @param len 要发送的数据长度。
 * @date 2024-01-31
 */
void UartMbsSendData(uint8_t *data, uint8_t len)
{
	RS485_TX(); // 切换为发送模式
	for (uint8_t i = 0; i < len; i++)
	{
		USART1->TDR = data[i] & 0xFF;
		while ((USART1->ISR & USART_FLAG_TXE) == RESET)
			;
	}
	while ((USART1->ISR & USART_FLAG_TC) == RESET)
		;
	RS485_RX(); // 切换回接收模式
}

/**
 * gpio_init
 * @brief 初始化GPIO，配置LED和RS485控制引脚。
 * @note 设置GPIO模式、速度、输出类型等参数，并初始化系统LED和红外功率。
 * @date 2024-01-31
 */
void gpio_init()
{
	GPIO_InitTypeDef GPIO_InitStructure;

	/* Enable GPIO clock */
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA | RCC_AHBPeriph_GPIOB, ENABLE);

	/* 两路串口io初始化 */
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_1);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_1);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_1);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_1);
	
	/* Configure pins as AF pushpull */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_2 | GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* Configure PC10 and PC11 in output pushpull mode */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	SysLedOn(); // 打开系统LED
	RS485_RX(); // 初始设置为RS485接收模式
}

/**
 * watchDog_init
 * @brief 初始化独立看门狗（IWDG），用于系统监控。
 * @note 配置看门狗的预分频器、重载值，并启用看门狗。
 * @date 2024-01-31
 */
void watchDog_init()
{
	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
	IWDG_SetPrescaler(IWDG_Prescaler_128);
	IWDG_SetReload(10000);
	IWDG_ReloadCounter();
	IWDG_Enable();
}

// flash 操作相关函数,写(Halfword) 读(byte) 擦(page 1KB)需要对齐
void flash_write_halfword(uint32_t addr, uint16_t data)
{
	FLASH_Unlock();
	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPERR);
	if (FLASH_ProgramHalfWord(addr, data) != FLASH_COMPLETE)
		LOG_E("flash", "write error\r\n");
	FLASH_Lock();
}
void flash_write(uint32_t addr, uint16_t *data, uint8_t len)
{
	FLASH_Unlock();
	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPERR);
	for (uint8_t i = 0; i < len; i++)
	{
		if (FLASH_ProgramHalfWord(addr + i * 2, data[i]) != FLASH_COMPLETE)
			LOG_E("flash", "write error\r\n");
	}

	FLASH_Lock();
}
void flash_write_word(uint32_t addr, uint32_t data)
{
	FLASH_Unlock();
	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPERR);
	if (FLASH_ProgramWord(addr, data) != FLASH_COMPLETE)
		LOG_E("flash", "write error\r\n");
	FLASH_Lock();
}

void flash_program_bytes(uint32_t addr, uint8_t *data, uint32_t len)
{
	// 1.data cpy 32位对其 2.按字写入 len / 4
	uint32_t data32[32];
	if (len > 128)
	{
		LOG_E("flash", "write error\r\n");
		return;
	}
	memcpy(data32, data, len);
	FLASH_Unlock();
	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPERR);
	for (int i = 0; i < len / 4; i++)
	{
		if (FLASH_ProgramWord(addr + i * 4, data32[i]) != FLASH_COMPLETE)
		{
			LOG_E("flash", "write error\r\n");
			break;
		}
	}
	FLASH_Lock();
}

void flash_erase(uint32_t addr)
{
	if (addr % 1024 != 0 || addr < 0x08000000 || addr > 0x08007FFF)
	{
		LOG_E("flash", "erase addr error\r\n");
		return;
	}
	FLASH_Unlock();
	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPERR);
	if (FLASH_ErasePage(addr) != FLASH_COMPLETE)
		LOG_E("flash", "erase error\r\n");
	FLASH_Lock();
}

uint32_t offset1 = 0, offset2 = 0;
void FlashDataSave(uint8_t type)
{
	uint16_t data[2];
	memset(data, 0, sizeof(data));
	if (offset1 > 1000)
	{
		flash_erase(Data1Addr);
		offset1 = 0;
	}
	if (offset2 > 1000)
	{
		flash_erase(Data2Addr);
		offset2 = 0;
	}
	if (type == 1)
	{
		data[0] = IR_state.Brand & 0x0FFF;
		data[0] |= IR_state.isSupportSix << 12;
		flash_write(Data1Addr + offset1, data, 2);
		offset1 += 4;
	}
	if (type == 2)
	{
		data[0] = IR_state.aircon_state.WindSpeed & 0x03;
		data[0] |= (IR_state.aircon_state.Mode << 4) & 0x70;
		data[0] |= (IR_state.aircon_state.Temp << 8) & 0x1F00;
		flash_write(Data2Addr + offset2, data, 1);
		offset2 += 2;
	}
}

void FlashDataRead()
{
	uint32_t data = 0, i;
	for (i = 0; i < 1000; i += 4)
	{
		data = *(uint32_t *)(Data1Addr + i);
		if (data == 0xFFFFFFFF)
			break;
	}
	offset1 = i;
	if (offset1 != 0)
	{
		IR_state.Brand = *(uint16_t *)(Data1Addr + offset1 - 4) & 0x3FF;
		IR_state.isSupportSix = (*(uint16_t *)(Data1Addr + offset1 - 4) >> 12) & 0x01;
	}

	for (i = 0; i < 1000; i += 2)
	{
		data = *(uint16_t *)(Data2Addr + i);
		if (data == 0xFFFF)
			break;
	}
	offset2 = i;
	if (offset2 == 0)
	{
		IR_state.aircon_state.Mode = 0;
		IR_state.aircon_state.WindSpeed = 0;
		IR_state.aircon_state.Temp = 25;
	}
	else
	{
		data = *(uint16_t *)(Data2Addr + offset2 - 2);
		IR_state.aircon_state.WindSpeed = data & 0x03;
		IR_state.aircon_state.Mode = (data >> 4) & 0x07;
		IR_state.aircon_state.Temp = (data >> 8) & 0x1F;
	}
	if (IR_state.aircon_state.Temp < 16 || IR_state.aircon_state.Temp > 30)
		IR_state.aircon_state.Temp = 25;
	if (IR_state.aircon_state.Mode > 0x08)
		IR_state.aircon_state.Mode = 0;
	if (IR_state.aircon_state.WindSpeed > 0x03)
		IR_state.aircon_state.WindSpeed = 0;
	if (IR_state.isSupportSix > 1)
		IR_state.isSupportSix = 0;

	mbsCoilValue[Coil_IR_Switch].pData = IR_state.aircon_state.Switch;
	mbsHoldRegValue[Reg_IR_Brand].pData = IR_state.Brand;
	mbsHoldRegValue[Reg_IR_Temp].pData = IR_state.aircon_state.Temp;
	mbsHoldRegValue[Reg_IR_Mode].pData = IR_state.aircon_state.Mode;
	mbsHoldRegValue[Reg_IR_WindSpeed].pData = IR_state.aircon_state.WindSpeed;
}

typedef void (*appFunction)(void);

unsigned char jumpToApplication(unsigned int appAddress)
{
	appFunction jumpFunction;
	unsigned int jumpAddress;
	NVIC->ICER[0] = 0XFFFFFFFF;								  // 失能M0芯片中所有的外设中断
	jumpAddress = *(volatile unsigned int *)(appAddress + 4); // 获取跳转程序的入口地址
	jumpFunction = (appFunction)jumpAddress;

	__set_MSP(*(volatile unsigned int *)appAddress); // 设置SP指针 复位指针
	jumpFunction();									 // 开始跳转
	return 1;										 // 跳转成功
}
#pragma pop
