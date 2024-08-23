#include "main.h"
#include "ymodem.h"
#include "log.h"

#define TAG "App"
#pragma push
#pragma O0
static struct pt uart_ir;
static struct pt uart_rs485;
static struct pt period_process;

static int Task1_uart_ble(struct pt *pt);
static int Task2_uart_rs485(struct pt *pt);
static int Task3_period_process(struct pt *pt);

static void mbs_data_process(void);
static void LedCtrl(void);
static void SysMonitor(void);
static bool IapPacketCheck(void);
static void Ymodem_timeout_process(void);
static void MbsDataSave(void);
static void music_config(void);
/**
 * system_run
 * @brief 主循环函数，负责周期性地执行各任务。
 * @note 使用协程(pt)处理多任务，确保系统的响应性和灵活性。
 * @date 2024-01-31
 */
void system_run()
{
    // 初始化协程
    PT_INIT(&uart_ir);
    PT_INIT(&uart_rs485);
    PT_INIT(&period_process);

    // 主循环
    while (1)
    {
#if IsApplication
        Task1_uart_ble(&uart_ir);
#endif
        Task2_uart_rs485(&uart_rs485);
        Task3_period_process(&period_process);
    }
}

/**
 * Task1_uart_ble
 * @brief 处理与蓝牙通信相关的任务。
 * @param pt 协程状态
 * @return 协程执行状态
 * @date 2024-01-31
 */
uint8_t bleBuffer[256] = {0};
static int Task1_uart_ble(struct pt *pt)
{
    PT_BEGIN(pt);
    while (1)
    {
        // 等待蓝牙数据到达
        PT_WAIT_UNTIL(pt, uart_ble_state.has_data);
        LOG_I(TAG, "ble data received: %s \r\n", uart_ble_state.data);
        // 处理蓝牙数据
        memcpy(bleBuffer, uart_ble_state.data, uart_ble_state.data_len);
        parseAndHandleCommand(bleBuffer);
        memset(bleBuffer, 0, sizeof(bleBuffer));
        memset(&uart_ble_state, 0, sizeof(uart_state_t));
    }
    PT_END(pt);
}

/**
 * Task2_uart_rs485
 * @brief 处理与RS485通信相关的任务。
 * @param pt 协程状态
 * @return 协程执行状态
 * @date 2024-01-31
 */
static int Task2_uart_rs485(struct pt *pt)
{
    PT_BEGIN(pt);
    while (1)
    {
        // 等待RS485数据到达
        PT_WAIT_UNTIL(pt, uart_rs485_state.has_data);
        LOG_I(TAG, "ble data received: %s \r\n", uart_rs485_state.data);
#if LOG_LEVEL >= LOG_INFO
        UartBleSendData(uart_rs485_state.data, uart_rs485_state.data_len);
#endif
        // 检查是否为IAP包，如果不是，则进行正常处理
        if (IapPacketCheck() == false && uart_rs485_state.data_len < 64)
        {
            // 正常处理RS485数据
            memcpy(MBS_Buf._rxBuff, uart_rs485_state.data, uart_rs485_state.data_len);
            MBS_Buf._rxLen = uart_rs485_state.data_len;
            MBS_CorePoll();
        }
        memset(&uart_rs485_state, 0, sizeof(uart_state_t));
    }
    PT_END(pt);
}

/**
 * Task3_period_process
 * @brief 处理周期性任务，如Modbus数据处理、LED控制、系统监控和Ymodem超时处理。
 * @param pt 协程状态
 * @return 协程执行状态
 * @date 2024-01-31
 */
static int Task3_period_process(struct pt *pt)
{
    static uint32_t count_100ms = 0;
    PT_BEGIN(pt);
    while (1)
    {
#if IsApplication
        mbs_data_process();
        music_config();
        if (count_100ms % 5 == 0)
        {
            executeBleCommand();
        }
#endif
        LedCtrl();
        SysMonitor();
        Ymodem_timeout_process();
        count_100ms++;
        PT_TIMER_DELAY(pt, 100);
    }
    PT_END(pt);
}

static bool IapPacketCheck()
{
    static uint8_t upFrame[8] = {0xFF, MBS_SelfAddr, 0x50, 0xA5, 0x5A, 0x38, 0x26, 0xFE};
    // 检查收到的数据是否匹配IAP特定的帧
    if (uart_rs485_state.data_len == 8 && memcmp(uart_rs485_state.data, upFrame, 8) == 0)
    {
        flash_erase(IapAddr);
        uint16_t Data16 = 0x55;
        flash_write_halfword(IapAddr, Data16);
        Data16 = 0xAA;
        flash_write_halfword(IapAddr + 2, Data16);
        uint8_t tempdata = YMODEM_CRC;
        UartMbsSendData(&tempdata, 1); // 不要改动回复位置,老版本IAP需要
        delay_ms(1);
        NVIC_SystemReset();
    }
    if (ymodem_session.state == YMODEM_STATE_IDLE)
    {
        return false;
    }
#if !IsApplication
    uint8_t TxBuffer[5];
    uint16_t TxLen = 0;
    int res = ymodem_packet_analysis(uart_rs485_state.data, uart_rs485_state.data_len, TxBuffer, &TxLen);
    delay_ms(5);
#if !IsRelease // 不上报升级成功,无限升级测试
    if (TxLen > 0 && res != 2)
    {
        UartMbsSendData(TxBuffer, TxLen);
    }
    if (res == 2)
    {
        jumpToApplication(AppAddr);
    }
#else
    if (TxLen > 0)
    {
        UartMbsSendData(TxBuffer, TxLen);
    }
    if (res == 2)
    {
        jumpToApplication(AppAddr);
    }
#endif
    if (TxLen > 0)
        return true;
#endif
    return false;
}

/**
 * Ymodem_timeout_process
 * @brief 处理Ymodem通信的超时情况。
 * @note 当Ymodem状态非空闲且超时时，发送NAK或CAN指令，并在错误次数过多时重置系统。
 * @date 2024-01-31
 */
static void Ymodem_timeout_process()
{
#if !Lora_Is_APP
    if (ymodem_session.state != YMODEM_STATE_IDLE)
    { // 非空闲状态下的Ymodem超时处理
        uint8_t temp[2];
        ymodem_session.timeout++;
        if (ymodem_session.timeout > 30)
        {
            ymodem_session.error_count++;
            ymodem_session.timeout = 0;
            temp[0] = YMODEM_NAK;
            UartMbsSendData(temp, 1);
        }
        if (ymodem_session.error_count > 5)
        {
            memset(&ymodem_session, 0, sizeof(ymodem_session_t));
            temp[0] = YMODEM_CAN;
            temp[1] = YMODEM_CAN;
            UartMbsSendData(temp, 2);
            NVIC_SystemReset();
        }
    }
#endif
}

/**
 * mbs_data_process
 * @brief 处理Modbus数据和蓝牙指令的间隔时间控制。
 * @note 该函数控制蓝牙指令的发送间隔，避免频繁发送导致的问题。
 * @date 2024-01-31
 */
static void mbs_data_process()
{
    static uint8_t Init_flag = 1;
    static uint32_t count_100ms = 0, errCount = 0;
    if (Init_flag == 1)
    {
        for (uint8_t i = 0; i < 6; i++)
        {
            bleState.Mac[i] = mbsHoldRegValue[Reg_BleMac + i].pData;
        }
        if (mbsCoilValue[Coil_BleMode].pData == BleMode)
        {
            char temp[32];
            sprintf(temp, "AT+SP%02X%02X%02X%02X%02X%02X\r\n", bleState.Mac[0], bleState.Mac[1], bleState.Mac[2],
                    bleState.Mac[3], bleState.Mac[4], bleState.Mac[5]);
            writeBluetoothCommand(temp);
            PowerOff();
        }
        else
            PowerOn();
        Init_flag = 0;
    }
    if (count_100ms % 100 == 10)
    {
        writeBluetoothCommand("AT+RN\r\n"); // 查蓝牙使能
    }
    if (mbsCoilValue[Coil_BleMode].pData == BleMode)
    {
        if (count_100ms % 100 == 20)
        {
            writeBluetoothCommand("AT+RS\r\n"); // 查蓝牙连接状态
        }
        if (count_100ms % 100 == 30)
        {
            writeBluetoothCommand("AT+TQ01\r\n"); // 查蓝牙MAC
        }
    }
    if (mbsCoilValue[Coil_BleReset].pData)
    {
        writeBluetoothCommand("AT+CZ\r\n");
        delay_ms(50);
        NVIC_SystemReset();
    }
    else if (mbsCoilValue[Coil_BleMode].pData == bleState.IsEnable && count_100ms % 10 == 5)
    {
        if (mbsCoilValue[Coil_BleMode].pData == BleMode)
        {
            writeBluetoothCommand("AT+SF01\r\n");
            bleState.IsEnable = 1;
            PowerOff();
        }
        else
        {
            writeBluetoothCommand("AT+SF02\r\n");
            bleState.IsEnable = 0;
            PowerOn();
        }
        MbsFlashDataSave();
    }
    else if (mbsCoilValue[Coil_BleReConnect].pData)
    {
        bleState.IsConnected = false;
        mbsCoilValue[Coil_BleReConnect].pData = 0;
        char temp[32];
        sprintf(temp, "AT+SP%02X%02X%02X%02X%02X%02X\r\n", bleState.Mac[0], bleState.Mac[1], bleState.Mac[2],
                bleState.Mac[3], bleState.Mac[4], bleState.Mac[5]);
        writeBluetoothCommand(temp);
    }
    else
    {
        for (uint8_t i = 0; i < 6; i++)
        {
            if (bleState.Mac[i] != mbsHoldRegValue[Reg_BleMac + i].pData)
            {
                for (uint8_t j = 0; j < 6; j++)
                {
                    bleState.Mac[j] = mbsHoldRegValue[Reg_BleMac + j].pData;
                }
                char temp[32];
                sprintf(temp, "AT+SP%02X%02X%02X%02X%02X%02X\r\n", bleState.Mac[0], bleState.Mac[1], bleState.Mac[2],
                        bleState.Mac[3], bleState.Mac[4], bleState.Mac[5]);
                writeBluetoothCommand(temp);
                bleState.IsConnected = false;
                mbsCoilValue[Coil_BleConnectState].pData = false;
                MbsFlashDataSave();
                break;
            }
        }
    }
    if (bleState.IsEnable && (!bleState.IsConnected))
    {
        errCount++;
        if (errCount > 600) // 一分钟没连上 重启
        {
            LOG_E(TAG, "Bluetooth connect error\r\n");
            UartBleSendData("AT+CZ\r\n", 7);
            delay_ms(10);
            NVIC_SystemReset();
        }
    }
    else
    {
        errCount = 0;
    }
    count_100ms++;
}

void music_config()
{
    static uint32_t count_100ms = 0;
    if ((!bleState.IsEnable) || (bleState.IsEnable && bleState.IsConnected))
    {
        if (mbsCoilValue[Coil_MusicPause].pData != musicState.isPlay) // 播放暂停
        {
            musicState.isPlay = mbsCoilValue[Coil_MusicPause].pData;
            writeBluetoothCommand("AT+CB\r\n");
        }
        else if (mbsHoldRegValue[Reg_Volume].pData != musicState.volume) // 音量
        {
            musicState.volume = mbsHoldRegValue[Reg_Volume].pData;
            char temp[32];
            if (musicState.volume > 30)
                musicState.volume = 30;
            if (musicState.volume >= 10)
                sprintf(temp, "AT+CA%d\r\n", musicState.volume);
            else
                sprintf(temp, "AT+CA0%d\r\n", musicState.volume);
            writeBluetoothCommand(temp);
        }
        else if (mbsCoilValue[Coil_MusicMode].pData != musicState.mode) // 播放模式
        {
            musicState.mode = mbsCoilValue[Coil_MusicMode].pData;
            if (mbsCoilValue[Coil_MusicMode].pData)
            {
                writeBluetoothCommand("AT+AC02\r\n");
            }
            else
            {
                writeBluetoothCommand("AT+AC00\r\n");
            }
            MbsFlashDataSave();
        }
        else if (mbsCoilValue[Coil_NextMusic].pData) // 下一首
        {
            mbsCoilValue[Coil_NextMusic].pData = 0;
            writeBluetoothCommand("AT+CC\r\n");
        }
        else if (mbsCoilValue[Coil_LastMusic].pData) // 上一首
        {
            mbsCoilValue[Coil_LastMusic].pData = 0;
            writeBluetoothCommand("AT+CD\r\n");
        }
        else if (mbsCoilValue[Coil_MusicChange].pData) // 指定切换某一首
        {
            mbsCoilValue[Coil_MusicChange].pData = 0;
            char temp[32];
            sprintf(temp, "AT+AB%d\r\n", mbsHoldRegValue[Reg_LoopIndex].pData);
            writeBluetoothCommand(temp);
            if (mbsCoilValue[Coil_MusicMode].pData == 0)
            {
                writeBluetoothCommand("AT+AC00\r\n");
            }
            MbsFlashDataSave();
        }
        else if (count_100ms % 100 == 40)
        {
            writeBluetoothCommand("AT+MP\r\n");
        }
        else if (count_100ms % 100 == 50)
        {
            if (musicState.mode)
                writeBluetoothCommand("AT+AC02\r\n");
            else
                writeBluetoothCommand("AT+AC00\r\n");
        }
        count_100ms++;
    }
}
/**
 * LedCtrl
 * @brief 控制LED的闪烁，不同模式下闪烁频率不同。
 * @note 在应用程序模式下，LED每隔1秒闪烁一次；在其他模式下，每隔0.5秒闪烁一次。
 * @date 2024-01-31
 */
static void LedCtrl()
{
    static uint8_t count = 0;
#if IsApplication
    if (count % 10 == 0)
    {
        SysLedOn();
    }
#else
    if (count % 3 == 0)
    {
        SysLedOn();
    }
#endif
    else
    {
        SysLedOff();
    }
    count++;
}

/**
 * SysMonitor
 * @brief 系统监控函数，检测Modbus通信是否超时并执行看门狗重载。
 * @note 如果Modbus通信超时，则重置系统。
 * @date 2024-01-31
 */
static void SysMonitor()
{
    static uint32_t mbs_timeout = 0;
#if IsRelease
    if (mbs_has_data)
    {
        mbs_timeout = 0;
        mbs_has_data = 0;
    }
    else
    {
        mbs_timeout++;
        if (mbs_timeout > 10000)
        {
            mbs_timeout = 0;
            NVIC_SystemReset();
        }
    }
#endif
    IWDG_ReloadCounter();
}

#pragma pop
