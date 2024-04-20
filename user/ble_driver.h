#ifndef __BLE_DRIVER_H__
#define __BLE_DRIVER_H__

#include "main.h"
void parseAndHandleCommand(char *packet);

#define Ble_mac_size 6
#define ErrCountMax 4
typedef struct 
{
    /* data */
    uint8_t IsEnable;   // 0:���� 1:����
    uint8_t IsConnected; // 0:δ���� 1:������
    uint8_t Mac[Ble_mac_size]; 
    uint8_t errCount;       // mac�������
}bleState_t;
extern bleState_t bleState;

typedef struct
{
    /* data */
    uint8_t isPlay;
    uint8_t volume;
    uint8_t mode;   // 0:�б�ѭ�� 1:����ѭ��
    int fileIndex;
    int fileDuration;
    int fileProgress;
    int fileNum;
}musicState_t;
extern musicState_t musicState;
void BleCmdSend(char *cmd);

#endif // !__BLE_DRIVER_H__
