#include "ble_driver.h"
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#define TAG "Ble Cmd Process"

bleState_t bleState; // ʵ�ʲ�����Ŀ�����
musicState_t musicState;
// ������ǰ������
void handleCommand(char *cmd);
void handleOK(char *str, uint16_t len);
void handleIsConnected(char *str, uint16_t len);
void handleIsBleMode(char *str, uint16_t len);
void handleSearchMode(char *str, uint16_t len);
void handleTQ(char *str, uint16_t len);
void handleM1(char *str, uint16_t len);
void handleM2(char *str, uint16_t len);
void handleMP(char *str, uint16_t len);
void handleMT(char *str, uint16_t len);
void handleMK(char *str, uint16_t len);

typedef void (*BleCmd_Handler)(char *str, uint16_t len);

typedef struct
{
    const char *command;
    BleCmd_Handler handler;
} BleCommand;

// �����������ӳ��
BleCommand Commands[] = {
    {"OK", handleOK},
    {"RS+", handleIsConnected}, // ��������״̬ δ����-RS+00 ��������-RS+01	������-RS+03
    {"RN+", handleIsBleMode},   // �Ƿ�Ϊ����  ����ģʽ-RN+00 ����ģʽ-RN+01
    {"RT+", handleSearchMode},  // ������ʽ  δ��������ģʽ-RT+00 �������-RT+01 MAC����-RT+02
    {"TQ+", handleTQ},          // ����MAC��ַ TQ+775651ACAB48
    {"M1+", handleM1},          // ��ǰ�ļ����� M1+00000001
    {"M2+", handleM2},          // ���ļ���  M2+00000025
    {"MP+", handleMP},          // ����״̬  ����-MP+01 ��ͣ-MP+02
    {"MT+", handleMT},          // �����ļ�ʱ��  MT+000000AD
    {"MT+", handleMK},          // ��ǰ�ļ����Ž���  MK+000000AD
    {NULL, NULL}                // �б������־
};

void parseAndHandleCommand(char *packet)
{
    char *token = strtok(packet, "\r\n");
    while (token != NULL)
    {
        LOG_I(TAG, "cmd:%s\n", token);
        handleCommand(token);
        token = strtok(NULL, "\r\n");
    }
}

char ProcessBuffer[40], ProcessBufferLen = 0;
void handleCommand(char *cmd)
{
    for (int i = 0; Commands[i].command != NULL; i++)
    {
        if (strncmp(cmd, Commands[i].command, strlen(Commands[i].command)) == 0)
        {
            ProcessBufferLen = strlen(cmd) - strlen(Commands[i].command);
            for (int j = 0; j < ProcessBufferLen; j++)
            {
                ProcessBuffer[j] = cmd[strlen(Commands[i].command) + j];
            }
            Commands[i].handler(ProcessBuffer, ProcessBufferLen);
            return;
        }
    }
    LOG_I(TAG, "Unknown command\r\n");
}

int ascii2int(char *str, uint8_t len)
{
    int data = 0;
    for (int i = 0; i < len; i++)
    {
        if (str[i] >= '0' && str[i] <= '9')
            data = data * 16 + (str[i] - '0');
        else if (str[i] >= 'A' && str[i] <= 'F')
            data = data * 16 + (str[i] - 'A' + 10);
        else if (str[i] >= 'a' && str[i] <= 'f')
            data = data * 16 + (str[i] - 'a' + 10);
    }
    return data;
}

// ʵ��ʣ��Ĵ�����...
void handleOK(char *str, uint16_t len)
{
    LOG_I(TAG, "handleOK\r\n");
}

void handleIsConnected(char *str, uint16_t len)
{
    int data = ascii2int(str, len);
    if (data == 3)
    {
        LOG_I(TAG, "Bluetooth connect\r\n");
    }
    else
    {
        if (data == 0)
            LOG_I(TAG, "Bluetooth connection down\r\n");
        else if (data == 1)
            LOG_I(TAG, "Bluetooth connecting\r\n");
        else
            LOG_I(TAG, "Unknown Bluetooth connection status:%d\r\n", data);
        bleState.IsConnected = false; // �����߳�����������
    }
}

void handleSearchMode(char *str, uint16_t len)
{
    int data = ascii2int(str, len);
    if (data == 2)
        LOG_I(TAG, "Bluetooth search mode: mac\r\n");
    else
    {
        if (data == 0)
            LOG_I(TAG, "Bluetooth search mode: undefined\r\n");
        else if (data == 1)
            LOG_I(TAG, "Bluetooth search mode: random\r\n");
        else
            LOG_I(TAG, "Unknown Bluetooth search mode:%d\r\n", data);
    }
}

void handleIsBleMode(char *str, uint16_t len)
{
    int data = ascii2int(str, len);
    if (data == 0)
    {
        LOG_I(TAG, "Bluetooth down\r\n");
        if (bleState.IsEnable != (!mbsCoilValue[Coil_BleMode].pData))
            BleCmdSend("AT+SF02\r\n"); // ����Ϊ����
    }
    else if (data == 1)
    {
        LOG_I(TAG, "Bluetooth enable\r\n");
        if (bleState.IsEnable != (!mbsCoilValue[Coil_BleMode].pData))
            BleCmdSend("AT+SF01\r\n"); // ����Ϊ����
    }
    else
    {
        LOG_I(TAG, "Unknown Bluetooth search mode:%d\r\n", data);
    }
}

void handleTQ(char *str, uint16_t len)
{
    if (len != 12)
    {
        LOG_I(TAG, "Invalid MAC address length\r\n");
        return;
    }
    LOG_I(TAG, "%s, Mac:", str);
    char mac[6];
    for (int i = 0; i < 6; i++)
    {
        char hex[3] = {str[2 * i], str[2 * i + 1], 0};
        mac[i] = (char)strtol(hex, NULL, 16);
        LOG_D("%02X ", mac[i]);
    }
    LOG_D("\r\n");
    if (memcmp(mac, bleState.Mac, 6) != 0)
    {
        if (bleState.IsConnected)
        {
            bleState.errCount++;
            if (bleState.errCount > ErrCountMax)
            {   // ��ֹ��ȷ����,���Ǵ���ͨѶ�����µ�����
                bleState.errCount = 0;
                bleState.IsConnected = false;
                LOG_I(TAG, "Ble mac error\r\n");
                char temp[32];
                sprintf(temp, "AT+SP%02X%02X%02X%02X%02X%02X\r\n", bleState.Mac[0], bleState.Mac[1], bleState.Mac[2],
                        bleState.Mac[3], bleState.Mac[4], bleState.Mac[5]);
                BleCmdSend(temp);
            }
        }
        else
        {
            LOG_I(TAG, "Ble mac error\r\n");
            char temp[32];
            sprintf(temp, "AT+SP%02X%02X%02X%02X%02X%02X\r\n", bleState.Mac[0], bleState.Mac[1], bleState.Mac[2],
                    bleState.Mac[3], bleState.Mac[4], bleState.Mac[5]);
            BleCmdSend(temp);
        }
    }
    else
    {
        bleState.IsConnected = true;
        bleState.errCount = 0;
        LOG_I(TAG, "Ble mac true");
    }
}

void handleM1(char *str, uint16_t len)
{
    int data = ascii2int(str, len);
    mbsHoldRegValue[Reg_MusicIndex].pData = data;
    LOG_I(TAG, "Current file index:%d\r\n", data);
}

void handleM2(char *str, uint16_t len)
{
    int data = ascii2int(str, len);
    mbsHoldRegValue[Reg_MusicTotal].pData = data;
    LOG_I(TAG, "Total file number:%d\r\n", data);
}

void handleMP(char *str, uint16_t len)
{
    int data = ascii2int(str, len);
    if (data == 2)
    {
        musicState.isPlay = 0;
        LOG_I(TAG, "Pause\r\n");
    }
    else if (data == 1)
    {
        musicState.isPlay = 1;
        LOG_I(TAG, "Play\r\n");
    }
    else
        LOG_I(TAG, "Unknown play status:%d\r\n", data);
}

void handleMT(char *str, uint16_t len)
{
    int data = ascii2int(str, len);
    mbsHoldRegValue[Reg_MusicDuration].pData = data;
    LOG_I(TAG, "File duration:%d\r\n", data);
}

void handleMK(char *str, uint16_t len)
{
    int data = ascii2int(str, len);
    mbsHoldRegValue[Reg_MusicProgress].pData = data;
}

void BleCmdSend(char *cmd)
{
    static uint32_t lastOperationTime;
    if (sys_ms - lastOperationTime < 100)
    {
        LOG_I(TAG, "Cmd too fast\r\n");
        return;
    }
    LOG_I(TAG, "Send cmd:%s\r\n", cmd);
    if (strlen(cmd) > 30)
    {
        LOG_E(TAG, "Cmd too long\r\n");
        return;
    }
    UartBleSendData(cmd, strlen(cmd));
    lastOperationTime = sys_ms;
}
