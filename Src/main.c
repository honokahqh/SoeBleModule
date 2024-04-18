#include "main.h"
#include "ymodem.h"

#define TAG "main"

static void IapCheck(void);

/**
 * main
 * @brief ������������ϵͳ��ʼ���ͳ���������
 * @author Honokahqh
 * @date 2024-1-31
 */
int main()
{
#if IsApp
    for (uint8_t i = 0; i < 48; i++) // ӳ���ж������� RAM����0x20000000Ϊʼ��48������
    {
        *((uint32_t *)(0x20000000 + (i << 2))) = *(volatile unsigned int *)(AppAddr + (i << 2));
    }
    SYSCFG_MemoryRemapConfig(SYSCFG_MemoryRemap_SRAM); // ����ڵ�ַӳ���ΪSRAM
    RCC_APB2PeriphResetCmd(RCC_APB2Periph_SYSCFG, ENABLE);
#endif
    // ����ϵͳʱ��
    SystemCoreClockUpdate();
    // ����ϵͳ�δ�ʱ��
    SysTick_Config(48000000 / 1000);
    // ��ʼ�����Ź�
//    watchDog_init();
	// ��ʼ��GPIO
    gpio_init();
    // ��ʼ��UART
    uart_init();
    // ����Ƿ���Ҫ����IAP��In-Application Programming��
    IapCheck();
    // ��ʼ��ModBusӳ��
    MBS_MappingInit();
    // ��ȡFlash����
    FlashDataRead();
    // ����ϵͳ
    system_run();
}	

/**
 * IapCheck
 * @brief ����Ƿ���Ҫִ��IAP��In-Application Programming����������������ת��ִ����ز�����
 * @note �˺���������Flash�Ĳ������ж��߼������ھ����Ƿ���й̼���������ת��Ӧ�ó���
 * @author Honokahqh
 * @date 2023-08-05
 */
static void IapCheck()
{
    uint16_t Data16;
#if !IsApp
    // �����IAP��ַ����������0xFFFF����ʾ�ǵ�һ������
    if (*(uint16_t *)IapAddr == 0xFFFF)
    {
        // ��IAP��ַ��д���־��׼����ת��Ӧ�ó���
        Data16 = 0x55;
        flash_write_halfword(IapAddr, Data16);
        Data16 = 0x55;
        flash_write_halfword(IapAddr + 2, Data16);
        LOG_I(TAG, "First power on, jump to App\r\n");
        jumpToApplication(AppAddr);
    }
    // ���IAP��־�����ã���ʾ��Ҫִ�й̼�����
    else if (*(uint16_t *)IapAddr == 0x55 && *(uint16_t *)(IapAddr + 2) == 0xAA && *(uint16_t *)(IapAddr + 4) == 0xFFFF)
    {
        // ����IAP���ȱ�־
        Data16 = 0xFA;
        flash_write_halfword(IapAddr + 4, Data16);
        LOG_I(TAG, "IAP Start\r\n");
        // ��ʼ��ymodemЭ��׼���̼�����
        ymodem_init();
        ymodem_session.state = YMODEM_STATE_START;
        // ����Ӧ�ó��������Flash
        for (uint32_t i = 0; i < 16; i++)
        {
            flash_erase(AppAddr + i * 1024);
        }
    }
    // �����������������ת��Ӧ�ó���
    else if (*(uint16_t *)(IapAddr + 6) == 0x55)
    {
        LOG_I(TAG, "Normal Reboot, jump to App\r\n");
        jumpToApplication(AppAddr);
    }
    // ���������������������ʾIAPʧ��
    else
    {
        LOG_E(TAG, "IAP failed\r\n");
    }
#else
    // ��Ӧ�ó���ģʽ�µĴ����߼�
    if (*(uint16_t *)(IapAddr + 6) != 0x55)
    {
        if(*(uint16_t *)(IapAddr + 6) != 0xFFFF)
            flash_erase(IapAddr);
        LOG_I(TAG, "first power on or IAP success, write app run flag\r\n");
        // ����Ӧ�ó������б�־
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
