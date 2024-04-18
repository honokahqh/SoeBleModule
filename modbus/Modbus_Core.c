#include "main.h"
#include "Modbus_CORE.h"
MBS_PortFrameTypes MBS_Buf;
volatile uint8_t mbs_has_data;
/**
 * Cmd_MBS_Cmd_Request
 * @brief 收到Lora主机0x60命令时调用-判断是否为本机地址
 * @brief 如果是本机地址直接处理回复,否则进MBS主机队列
 * @brief v101新增作为普通中控使用,不需要转发从机数据
 * @author Honokahqh
 * @date 2023-08-05
 */
void MBS_CorePoll()
{
    if (MBS_Buf._rxLen >= 4 && mb_crc16(MBS_Buf._rxBuff, MBS_Buf._rxLen) == 0 && MBS_Buf._rxBuff[MBS_FRAME_SLAVER_ADDR] == MBS_SelfAddr)
    {
        MBS_CoreAnalyze();
        mbs_has_data = 1;
    }
}

/**
 * MBS_CoreAnalyze
 * @brief mbs从机协议分析
 * @author Honokahqh
 * @date 2023-08-05
 */
void MBS_CoreAnalyze() // 只实现两个功能---03H读保持寄存器---10H写多个保持寄存器
{
    switch (MBS_Buf._rxBuff[1]) /* 第2个字节 功能码 */
    {
#if MBS_FUNCTION_01_ENABLE
    case 0x01:
        MBS_Function01H();
        break;
#endif
#if MBS_FUNCTION_02_ENABLE
    case 0x02:
        MBS_Function02H();
        break;
#endif
#if MBS_FUNCTION_03_ENABLE
    case 0x03:
        MBS_Function03H();
        break;
#endif
#if MBS_FUNCTION_04_ENABLE
    case 0x04:
        MBS_Function04H();
        break;
#endif
#if MBS_FUNCTION_05_ENABLE
    case 0x05:
        MBS_Function05H();
        break;
#endif
#if MBS_FUNCTION_06_ENABLE
    case 0x06:
        MBS_Function06H();
        break;
#endif
#if MBS_FUNCTION_0F_ENABLE
    case 0x0F:
        MBS_Function0FH();
        break;
#endif
#if MBS_FUNCTION_10_ENABLE
    case 0x10:
        MBS_Function10H();
        break;
#endif
    default:
        MBS_PortSendAck(MBS_EX_ILLEGAL_FUNCTION);
        break;
    }
}
void MBS_PortSendWithCRC(uint8 *buf, uint8 len)
{
    uint16 crc;
    crc = mb_crc16(buf, len);
    buf[len++] = crc;
    buf[len++] = crc >> 8;
    MBS_PhysicalSendBuff(buf, len);
}
/*
*****************************************************
* Method:       MBS_PortSendAck
* Description:	发送应答消息
* Author:       @Draven
* Date:  		2017/05/08
* Returns:      void
* Parameter:   	MBS_EX_STATE ackCode
* History:
*****************************************************
*/
void MBS_PortSendAck(MBS_EX_STATE ackCode)
{
    if (ackCode)
    {
        MBS_Buf._txBuff[MBS_FRAME_SLAVER_ADDR] = MBS_SelfAddr;
        MBS_Buf._txBuff[MBS_FRAME_FUNCTION_CODE] = MBS_Buf._rxBuff[MBS_FRAME_FUNCTION_CODE] | 0x80;
        MBS_Buf._txBuff[MBS_FRAME_FUNCTION_CODE + 1] = (uint8)ackCode; // 发送错误码
        MBS_Buf._txLen = 3;
    }
    else
    {
        uint8 i;
        for (i = 0; i < 6; i++)
        {
            MBS_Buf._txBuff[i] = MBS_Buf._rxBuff[i];
        }
        MBS_Buf._txLen = 6;
    }
    MBS_PortSendWithCRC(MBS_Buf._txBuff, MBS_Buf._txLen);
}
void MBS_PhysicalSendBuff(uint8 *buf, uint8 len)
{
    UartMbsSendData(buf, len);
}
