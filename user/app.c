#include "main.h"
#include "ymodem.h"
#include "log.h"

#define TAG "App"
#pragma push
#pragma O0
static struct pt uart_ir;
static struct pt uart_rs485;
static struct pt period_process;

static int Task1_uart_ir(struct pt *pt);
static int Task2_uart_rs485(struct pt *pt);
static int Task3_period_process(struct pt *pt);

static void mbs_data_process(void);
static void LedCtrl(void);
static void SysMonitor(void);
static bool IapPacketCheck(void);
static void Ymodem_timeout_process(void);
static void MbsDataSave(void);

/**
 * system_run
 * @brief ��ѭ�����������������Ե�ִ�и�����
 * @note ʹ��Э��(pt)���������ȷ��ϵͳ����Ӧ�Ժ�����ԡ�
 * @date 2024-01-31
 */
void system_run()
{
    // ��ʼ��Э��
    PT_INIT(&uart_ir);
    PT_INIT(&uart_rs485);
    PT_INIT(&period_process);

    // ��ѭ��
    while (1)
    {
        Task1_uart_ir(&uart_ir);
        Task2_uart_rs485(&uart_rs485);
        Task3_period_process(&period_process);
    }
}

/**
 * Task1_uart_ir
 * @brief ���������ͨ����ص�����
 * @param pt Э��״̬
 * @return Э��ִ��״̬
 * @date 2024-01-31
 */
static int Task1_uart_ir(struct pt *pt)
{
    static uint8_t IrCmdErrCount = 0;
    PT_BEGIN(pt);
    while (1)
    {
        // �ȴ��������ݵ���
        PT_WAIT_UNTIL(pt, uart_ir_state.has_data);
#if LOG_LEVEL >= LOG_INFO
        LOG_I(TAG, "IR data received len: %d data:", uart_ir_state.data_len);
        for (int i = 0; i < uart_ir_state.data_len; i++)
        {
            printf("%02X ", uart_ir_state.data[i]);
        }
        printf("\r\n");
#endif
        // ������յ��ĺ�������
        if (uart_ir_state.data_len == 6)
        {
            if (ir_cmd_ack_process() == IR_Success)
            {
                IrCmdErrCount = 0;
            }
            else
            {
                IrCmdErrCount++;
                if (IrCmdErrCount > 10)
                {
                    NVIC_SystemReset();
                }
            }
        }
        IR_state.waitAck = 0;
        memset(&uart_ir_state, 0, sizeof(uart_state_t));
    }
    PT_END(pt);
}

/**
 * Task2_uart_rs485
 * @brief ������RS485ͨ����ص�����
 * @param pt Э��״̬
 * @return Э��ִ��״̬
 * @date 2024-01-31
 */
static int Task2_uart_rs485(struct pt *pt)
{
    PT_BEGIN(pt);
    while (1)
    {
        // �ȴ�RS485���ݵ���
        PT_WAIT_UNTIL(pt, uart_rs485_state.has_data);
#if LOG_LEVEL >= LOG_INFO
        LOG_I(TAG, "RS485 data received len: %d data:", uart_rs485_state.data_len);
        for (int i = 0; i < uart_rs485_state.data_len; i++)
        {
            printf("%02X ", uart_rs485_state.data[i]);
        }
        printf("\r\n");
#endif
        // ����Ƿ�ΪIAP����������ǣ��������������
        if (IapPacketCheck() == false && uart_rs485_state.data_len < 64)
        {
            // ��������RS485����
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
 * @brief ����������������Modbus���ݴ���LED���ơ�ϵͳ��غ�Ymodem��ʱ����
 * @param pt Э��״̬
 * @return Э��ִ��״̬
 * @date 2024-01-31
 */
static int Task3_period_process(struct pt *pt)
{
    PT_BEGIN(pt);
    while (1)
    {
        mbs_data_process();
        LedCtrl();
        SysMonitor();
        Ymodem_timeout_process();
        MbsDataSave();
        PT_TIMER_DELAY(pt, 100);
    }
    PT_END(pt);
}

static bool IapPacketCheck()
{
    static uint8_t upFrame[8] = {0xFF, MBS_SelfAddr, 0x50, 0xA5, 0x5A, 0x38, 0x26, 0xFE};
    // ����յ��������Ƿ�ƥ��IAP�ض���֡
    if (uart_rs485_state.data_len == 8 && memcmp(uart_rs485_state.data, upFrame, 8) == 0)
    {
        flash_erase(IapAddr);
        uint16_t Data16 = 0x55;
        flash_write_halfword(IapAddr, Data16);
        Data16 = 0xAA;
        flash_write_halfword(IapAddr + 2, Data16);
        uint8_t tempdata = YMODEM_CRC;
        UartMbsSendData(&tempdata, 1);
        NVIC_SystemReset();
    }
    if (ymodem_session.state == YMODEM_STATE_IDLE)
    {
        return false;
    }
#if !IsApp
    uint8_t TxBuffer[5];
    uint16_t TxLen = 0;
    int res = ymodem_packet_analysis(uart_rs485_state.data, uart_rs485_state.data_len, TxBuffer, &TxLen);
    delay_ms(5);
    if (TxLen > 0)
    {
        UartMbsSendData(TxBuffer, TxLen);
    }
    if (res == 2)
    {
        jumpToApplication(AppAddr);
    }
    if (TxLen > 0)
        return true;
#endif
    return false;
}

/**
 * Ymodem_timeout_process
 * @brief ����Ymodemͨ�ŵĳ�ʱ�����
 * @note ��Ymodem״̬�ǿ����ҳ�ʱʱ������NAK��CANָ����ڴ����������ʱ����ϵͳ��
 * @date 2024-01-31
 */
static void Ymodem_timeout_process()
{
#if !Lora_Is_APP
    if (ymodem_session.state != YMODEM_STATE_IDLE)
    { // �ǿ���״̬�µ�Ymodem��ʱ����
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
 * @brief ����Modbus���ݺͺ���ָ��ļ��ʱ����ơ�
 * @note �ú������ƺ���ָ��ķ��ͼ��������Ƶ�����͵��µ����⡣
 * @date 2024-01-31
 */
static void mbs_data_process()
{
    uint32_t data;
    if (IR_state.Brand != mbsHoldRegValue[Reg_IR_Brand].pData)
    {
        write_buffer(IR_BrandSet << 16 | mbsHoldRegValue[Reg_IR_Brand].pData);
        IR_state.Brand = mbsHoldRegValue[Reg_IR_Brand].pData;
    }
    if (IR_state.CmdInterval > 0)
    {
        IR_state.CmdInterval--;
        return;
    }
    if (read_buffer(&data))
    {
        IR_state.waitAck = (data >> 16) & 0xFF;
        IR_state.waitAckSub = (data >> 8) & 0xFF;
        if (IR_state.waitAck == IR_AirConCtrl)
        {
            IR_state.CmdInterval = 15;
        }
        else if (IR_state.waitAck == IR_BrandStudyMode)
        {
            IR_state.CmdInterval = 100; // 10s
        }
        else
        {
            IR_state.CmdInterval = 2; // 200ms
        }
        ir_cmd_send((data >> 16) & 0xFF, (data >> 8) & 0xFF, data & 0xFF);
    }
}

/**
 * LedCtrl
 * @brief ����LED����˸����ͬģʽ����˸Ƶ�ʲ�ͬ��
 * @note ��Ӧ�ó���ģʽ�£�LEDÿ��1����˸һ�Σ�������ģʽ�£�ÿ��0.5����˸һ�Ρ�
 * @date 2024-01-31
 */
static void LedCtrl()
{
    static uint8_t count = 0;
#if IsApp
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
 * @brief ϵͳ��غ��������Modbusͨ���Ƿ�ʱ��ִ�п��Ź����ء�
 * @note ���Modbusͨ�ų�ʱ��������ϵͳ��
 * @date 2024-01-31
 */
static void SysMonitor()
{
    static uint32_t mbs_timeout = 0;
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
    IWDG_ReloadCounter();
}

/**
 * MbsDataSave
 * @brief Modbus�յ��������ڱ�����flash��
 * @note ����յ�������flash��
 * @date 2024-01-31
 */
static void MbsDataSave()
{
    static uint32_t count = 0;
    if (count++ >= 100)
    {
        if (mbsHoldRegValue[Reg_IR_Temp].pData != IR_state.aircon_state.Temp || mbsHoldRegValue[Reg_IR_Mode].pData != IR_state.aircon_state.Mode || mbsHoldRegValue[Reg_IR_WindSpeed].pData != IR_state.aircon_state.WindSpeed)
        {
            IR_state.aircon_state.Temp = mbsHoldRegValue[Reg_IR_Temp].pData;
            IR_state.aircon_state.Mode = mbsHoldRegValue[Reg_IR_Mode].pData;
            IR_state.aircon_state.WindSpeed = mbsHoldRegValue[Reg_IR_WindSpeed].pData;
            FlashDataSave(2);
        }
        count = 0;
    }
}
#pragma pop
