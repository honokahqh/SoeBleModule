#ifndef __IR_CTRL_H__
#define __IR_CTRL_H__

#include "main.h"
#include "bsp_driver.h"

#define IR_BrandSet         0x80    // ����Ʒ��
#define IR_BrandStudyMode   0x81    // ѧϰģʽ
#define IR_AirConCtrl       0x86    // ��������-���ƿյ�
#define IR_AirConStateSet   0x8F    // ���ú���оƬ�յ�״̬

#define IR_Switch           0   // ����
#define IR_Mode             1   // ģʽ
#define IR_TempUp           2   // �¶�
#define IR_TempDown         3   // �¶�
#define IR_TempSet          2   // �¶�
#define IR_WindSpeed        4   // ����
#define IR_WindStrength     9   // ǿ��

#define IR_CmdInterval      2000    // ��������ͼ��
typedef enum
{
    IR_Success = 0,
    XOR_Error,
    Cmd_Error,
    Format_Error,
    TimeOut_Error,
    Unknow_Error,
}ir_ack_t;

typedef struct 
{
    uint8_t  Switch;
    uint8_t  Mode;
    uint8_t  Temp;
    uint8_t  WindSpeed;
}aircon_state_t;

typedef struct 
{
    uint16_t Brand;
    uint8_t  isSupportSix;
    aircon_state_t aircon_state;
    uint8_t waitAck;
    uint8_t waitAckSub;
    uint8_t CmdInterval;
}IR_state_t;
extern IR_state_t IR_state;

typedef uint8_t (*UartIrRecvCb)(void);

void ir_state_init(void);
void ir_cmd_send(uint8_t cmd, uint8_t subcmd, uint8_t data);
uint8_t ir_cmd_ack_process(void);
#endif // !__IR_CTRL_H__
