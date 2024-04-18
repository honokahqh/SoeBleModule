#include "ir_ctrl.h"

#define TAG "IR Ctrl"

IR_state_t IR_state;

/**
 * ir_state_init
 * @brief 初始化红外控制状态。
 * @note 设置空调的初始状态，并将状态写入缓冲区。
 * @date 2024-01-31
 */
void ir_state_init()
{
    LOG_I(TAG, "ir_state_init");
    IR_state.aircon_state.Switch = 0; // 初始化空调开关状态为关闭
    mbsCoilValue[Coil_IR_Switch].pData = 0; // 设置Modbus线圈的值为0
    // 将空调状态的相关参数写入缓冲区
    write_buffer(IR_AirConStateSet << 16 | IR_Mode << 8 | IR_state.aircon_state.Mode << 0);
    write_buffer(IR_AirConStateSet << 16 | IR_TempSet << 8 | (IR_state.aircon_state.Temp - 16) << 0);
    write_buffer(IR_AirConStateSet << 16 | IR_WindSpeed << 8 | IR_state.aircon_state.WindSpeed << 0);
}

static void uart_ir_send(uint8_t *data, uint8_t len)
{
    UartBleSendData(data, len); // 通过UART发送数据
}

/**
 * ir_cmd_send
 * @brief 发送红外命令。
 * @param cmd 主命令码。
 * @param subcmd 子命令码。
 * @param data 命令数据。
 * @date 2024-01-31
 */
void ir_cmd_send(uint8_t cmd, uint8_t subcmd, uint8_t data)
{
    uint8_t buf[6], xor;
    buf[0] = cmd;
    buf[1] = 0x01; // 子命令长度
    buf[2] = subcmd;
    buf[3] = data;
    buf[4] = 0x00; // 保留位
    xor = buf[0] ^ buf[1] ^ buf[2] ^ buf[3] ^ buf[4]; // 计算校验和
    buf[5] = xor; // 设置校验和
    uart_ir_send(buf, 6); // 发送命令
}

/**
 * ir_cmd_ack_process
 * @brief 处理红外命令的响应。
 * @return 响应处理结果。
 * @note 验证响应格式、计算校验和，并根据收到的命令更新状态。
 * @date 2024-01-31
 */
uint8_t ir_cmd_ack_process()
{
	static uint8_t IsSupportSix = 0;
    uint8_t xor_res = 0;
    if (uart_ir_state.data_len == 6 && uart_ir_state.data[0] == 0x06 && uart_ir_state.data[1] == 0xE0)
    {
        if(IR_state.waitAck == IR_AirConCtrl && IR_state.waitAckSub == IR_WindSpeed)
        {
            IsSupportSix = 0;
            IR_state.isSupportSix = 0; // 不支持6档风
            FlashDataSave(1);
        }
        if(IR_state.waitAck == IR_BrandStudyMode)
        {
            IR_state.CmdInterval = 0;
        }
        LOG_E(TAG, "IR cmd invalid");
        return Cmd_Error;
    }
    if (uart_ir_state.data_len != 6 || uart_ir_state.data[0] != 0x06 || uart_ir_state.data[1] != 0x89)
    {
        LOG_E(TAG, "IR cmd ack format error");
        return Format_Error;
    }
    for (uint8_t i = 0; i < uart_ir_state.data_len; i++)
    {
        xor_res ^= uart_ir_state.data[i];
    }
    if (xor_res != 0)
    {
        LOG_E(TAG, "IR cmd ack xor error");
        return XOR_Error;
    }
    switch (IR_state.waitAck)
    {
	case IR_BrandSet:
		IsSupportSix = 1;
        write_buffer(IR_AirConCtrl << 16 | IR_WindSpeed << 8 | 5);
		break;
    case IR_BrandStudyMode:
		IsSupportSix = 1;
        IR_state.Brand = uart_ir_state.data[2] << 8 | uart_ir_state.data[3];
        write_buffer(IR_BrandSet << 16 | IR_state.Brand);
        write_buffer(IR_AirConCtrl << 16 | IR_WindSpeed << 8 | 5);
        break;
    case IR_AirConCtrl:
        if(IR_state.waitAckSub == IR_WindSpeed && IsSupportSix == 1)
        {
            IsSupportSix = 0;
            IR_state.isSupportSix = 1; // 支持6档风
            FlashDataSave(1);
        }
        break;
    default:
        break;
    }
    return IR_Success;
}
