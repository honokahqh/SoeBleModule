#include "main.h"
#include "ymodem.h"

#define TAG "main"

static void IapCheck(void);

/**
 * main
 * @brief 主函数，负责系统初始化和程序启动。
 * @author Honokahqh
 * @date 2024-1-31
 */
int main()
{
#if IsApplication
    for (uint8_t i = 0; i < 48; i++) // 映射中断向量到 RAM区的0x20000000为始的48个字中
    {
        *((uint32_t *)(0x20000000 + (i << 2))) = *(volatile unsigned int *)(AppAddr + (i << 2));
    }
    SYSCFG_MemoryRemapConfig(SYSCFG_MemoryRemap_SRAM); // 将入口地址映射成为SRAM
    RCC_APB2PeriphResetCmd(RCC_APB2Periph_SYSCFG, ENABLE);
#endif
    // 更新系统时钟
    SystemCoreClockUpdate();
    // 配置系统滴答定时器
    SysTick_Config(48000000 / 1000);
    // 初始化看门狗
    // watchDog_init();
    // 初始化GPIO
    gpio_init();
    // 初始化UART
    uart_init();
    // 检查是否需要进行IAP（In-Application Programming）
    IapCheck();
    // 初始化ModBus映射
#if IsApplication
    MBS_MappingInit();
    // 读取Flash数据
    FlashDataRead();
#endif
    // 运行系统
    system_run();
}

/**
 * IapCheck
 * @brief 检查是否需要执行IAP（In-Application Programming），并根据条件跳转或执行相关操作。
 * @note 此函数包含对Flash的操作和判断逻辑，用于决定是否进行固件升级或跳转到应用程序。
 * @author Honokahqh
 * @date 2023-08-05
 */
static void IapCheck()
{
    uint16_t Data16;
#if !IsApplication
    // 如果在IAP地址处的数据是0xFFFF，表示是第一次启动
    if (*(uint16_t *)IapAddr == 0xFFFF)
    {
        // 在IAP地址处写入标志，准备跳转到应用程序
        Data16 = 0x55;
        flash_write_halfword(IapAddr, Data16);
        Data16 = 0x55;
        flash_write_halfword(IapAddr + 2, Data16);
        LOG_I(TAG, "First power on, jump to App\r\n");
        jumpToApplication(AppAddr);
    }
    // 如果IAP标志被设置，表示需要执行固件升级
    else if (*(uint16_t *)IapAddr == 0x55 && *(uint16_t *)(IapAddr + 2) == 0xAA && *(uint16_t *)(IapAddr + 4) == 0xFFFF)
    {
        // 设置IAP进度标志
        Data16 = 0xFA;
        flash_write_halfword(IapAddr + 4, Data16);
        LOG_I(TAG, "IAP Start\r\n");
        // 初始化ymodem协议准备固件升级
        ymodem_init();
        ymodem_session.state = YMODEM_STATE_START;
        // 擦除应用程序区域的Flash
        for (uint32_t i = 0; i < 16; i++)
        {
            flash_erase(AppAddr + i * 1024);
        }
    }
    // 如果是正常重启，跳转到应用程序
    else if (*(uint16_t *)(IapAddr + 6) == 0x55)
    {
        LOG_I(TAG, "Normal Reboot, jump to App\r\n");
        jumpToApplication(AppAddr);
    }
    // 如果不满足以上条件，表示IAP失败
    else
    {
        LOG_E(TAG, "IAP failed\r\n");
    }
#else
    // 在应用程序模式下的处理逻辑
    if (*(uint16_t *)(IapAddr + 6) != 0x55)
    {
        if (*(uint16_t *)(IapAddr + 6) != 0xFFFF)
            flash_erase(IapAddr);
        LOG_I(TAG, "first power on or IAP success, write app run flag\r\n");
        // 设置应用程序运行标志
        Data16 = 0x55;
        flash_write_halfword(IapAddr + 6, Data16);
    }
#endif
    if (ymodem_session.state == YMODEM_STATE_IDLE)
    {
        delay_ms(200);
        uint8_t StartFrame[] = {0xFF, 0x6F, 0xFE};
        UartMbsSendData(StartFrame, 3);
    }
}
