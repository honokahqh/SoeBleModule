#ifndef __BSP_DRIVER_H__
#define __BSP_DRIVER_H__

#include "main.h"

// gpio
#define SysLedOn() GPIO_ResetBits(GPIOA, GPIO_Pin_0)
#define SysLedOff() GPIO_SetBits(GPIOA, GPIO_Pin_0)
#define RS485_TX() GPIO_SetBits(GPIOA, GPIO_Pin_1)
#define RS485_RX() GPIO_ResetBits(GPIOA, GPIO_Pin_1)
#define PowerOn() GPIO_ResetBits(GPIOB, GPIO_Pin_1)
#define PowerOff() GPIO_SetBits(GPIOB, GPIO_Pin_1)

void gpio_init(void);

// uart
void uart_init(void);
void UartBleSendData(uint8_t *data, uint8_t len);
void UartMbsSendData(uint8_t *data, uint8_t len);

#define UART_IDLE 5
typedef struct
{
    uint8_t idle;
    uint8_t has_data;
    uint8_t data[256];
    uint8_t data_len;
} uart_state_t;
extern uart_state_t uart_ble_state, uart_rs485_state;

// iwdog
void watchDog_init(void);

// flash
#define BootAddr 0x08000000
#define AppAddr 0x08002000
#define MbsDataAddr 0x08007800
#define IapAddr 0x08007C00

void flash_write_halfword(uint32_t addr, uint16_t data);
void flash_write_word(uint32_t addr, uint32_t data);
void flash_program_bytes(uint32_t addr, uint8_t *data, uint32_t len); // align 4
void flash_erase(uint32_t addr);
void MbsFlashDataSave(void);
void MbsFlashDataSyn(void);
unsigned char jumpToApplication(unsigned int appAddress);

#endif // !__BSP_DRIVER_H__
