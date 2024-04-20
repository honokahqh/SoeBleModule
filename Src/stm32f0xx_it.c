#include "main.h"
#include "bsp_driver.h"

uint32_t sys_ms;
__IO uint32_t TimingDelay;
#pragma push
#pragma O0
void SysTick_Handler(void)
{
    sys_ms++;
    if (uart_ble_state.idle)
    {
        uart_ble_state.idle++;
        if (uart_ble_state.idle > UART_IDLE * 4)
        {
            uart_ble_state.has_data = 1;
            uart_ble_state.idle = 0;
        }
    }
    if (uart_rs485_state.idle)
    {
        uart_rs485_state.idle++;
        if (uart_rs485_state.idle > UART_IDLE)
        {
            uart_rs485_state.has_data = 1;
            uart_rs485_state.idle = 0;
        }
    }
}


void delay_ms(__IO uint32_t nTime)
{
    TimingDelay = sys_ms + nTime;
    while (TimingDelay > sys_ms);
}

uint32_t millis(void)
{
    return sys_ms;
}

#if LOG_LEVEL >= LOG_I
int fputc(int ch, FILE *f)
{
    USART_SendData(USART2, (uint8_t)ch);
    while (USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET)
        ;
    return ch;
}
#endif

void USART1_IRQHandler(void)
{
    if (USART1->ISR & (0x01 << 5)) // Rx IE
    {
        /* Read one byte from the receive data register */
        uart_ble_state.data[uart_ble_state.data_len++] = USART1->RDR;
        uart_ble_state.idle = 1;
    }
    else if (USART1->ISR & (0x01 << 1)) // frame err
    {
        USART1->ICR = (0x01 << 1);
    }
    else if (USART1->ISR & (0x01 << 2)) // noise err
    {
        USART1->ICR = (0x01 << 2);
    }
    else if (USART1->ISR & (0x01 << 3)) // overrun err
    {
        USART1->ICR = (0x01 << 3);
    }
}

void USART2_IRQHandler(void)
{
    if (USART2->ISR & (0x01 << 5)) // Rx IE
    {
        /* Read one byte from the receive data register */
        uart_rs485_state.data[uart_rs485_state.data_len++] = USART2->RDR;
        uart_rs485_state.idle = 1;
    }
    // overrun等其他问题简单处理
    else if (USART2->ISR & (0x01 << 1)) // frame err
    {
        USART2->ICR = (0x01 << 1);
    }
    else if (USART2->ISR & (0x01 << 2)) // noise err
    {
        USART2->ICR = (0x01 << 2);
    }
    else if (USART2->ISR & (0x01 << 3)) // overrun err
    {
        USART2->ICR = (0x01 << 3);
    }
}

#pragma pop
