#ifndef __BLE_DRIVER_H__
#define __BLE_DRIVER_H__

#include "main.h"
void parseAndHandleCommand(char *packet);

#define Ble_mac_size 6
#define ErrCountMax 4
typedef struct 
{
    /* data */
    uint8_t IsEnable;   // 0:无线 1:有线
    uint8_t IsConnected; // 0:未连接 1:已连接
    uint8_t Mac[Ble_mac_size]; 
    uint8_t errCount;       // mac报错次数
}bleState_t;
extern bleState_t bleState;

typedef struct
{
    /* data */
    uint8_t isPlay;
    uint8_t volume;
    uint8_t mode;   // 0:列表循环 1:单曲循环
    int fileIndex;
    int fileDuration;
    int fileProgress;
    int fileNum;
}musicState_t;
extern musicState_t musicState;
void writeBluetoothCommand(char *cmd);
void executeBleCommand();

#endif // !__BLE_DRIVER_H__
