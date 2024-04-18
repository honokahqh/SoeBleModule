#include "Modbus_Core.h"

MBS_CoilValueTypes mbsCoilValue[USER_COIL_NUM];
MBS_CoilTypes mbsCoil = {mbsCoilValue, 0, 0, USER_COIL_NUM};

MBS_HoldRegValueTypes mbsHoldRegValue[USER_HOLDREG_NUM];
MBS_HoldRegTypes mbsHoldReg = {mbsHoldRegValue, 0, 0, USER_HOLDREG_NUM};

void MBS_MappingCoilInit(MBS_CoilValueTypes *coil, uint16_t addr, uint8_t Value)
{
    coil->coilAddr = addr;
    coil->pData = Value;
}

void MBS_MappingHoldRegInit(MBS_HoldRegValueTypes *reg, uint16 addr, uint16 Value)
{
    reg->regAddr = addr;
    reg->pData = Value;
}
/*
 */
void MBS_MappingInit()
{
    uint8 i;

    mbsCoil._startAddr = 12000; /* ��ʼ��ַ */
    mbsCoil._endAddr = 12007;   /* ������ַ */

    for (i = 0; i < 8; i++)
    {
        MBS_MappingCoilInit(&mbsCoilValue[i], i + 12000, 0);
    }

    mbsHoldReg._startAddr = 50000; /* ��ʼ��ַ */
    mbsHoldReg._endAddr = 59001;   /* ������ַ */
    for(i = 0; i < 2;i++)
    {
        MBS_MappingHoldRegInit(&mbsHoldRegValue[i], i + 59000, 0);
    }
	for(i = 0; i < 2;i++)
    {
        MBS_MappingHoldRegInit(&mbsHoldRegValue[i + 2], i + 51000, 0);
    }
    for(i = 0; i < 7;i++)
    {
        MBS_MappingHoldRegInit(&mbsHoldRegValue[i + 4], i + 51010, 0);
    }
    mbsHoldRegValue[MBS_Addr].pData = MBS_SelfAddr;
    mbsHoldRegValue[MBS_Ver].pData = Dev_Version;
}

uint8 MBS_MemReadCoilState(uint16 coilAddr)
{
    uint16 i;
    for (i = 0; i < USER_COIL_NUM; i++)
    {
        if (mbsCoilValue[i].coilAddr == coilAddr)
            return (mbsCoilValue[i].pData) % 2;
    }
    return i;
}
uint16 MBS_MemWriteCoilsState(uint16 CoilAddr, uint16 Len, uint8 *data)
{
    uint16 i, j;

    for (i = 0; i < USER_COIL_NUM; i++) // ���ҷ��ϵ�ַ
    {
        if (mbsCoilValue[i].coilAddr == CoilAddr)
            break;
    }
    if (i >= USER_COIL_NUM)
        return i; /*��ȡʧ�ܣ���ַ����*/

    for (j = 0; j < Len; j++)
        mbsCoilValue[i + j].pData = (data[j / 8] >> (j % 8)) & 0x01;

    return 1; /* ��ȡ�ɹ� */
}
uint8 MBS_MemWriteCoilState(uint16 coilAddr, uint16 data)
{
    uint16 i;
    for (i = 0; i < USER_COIL_NUM; i++)
    {
        if (mbsCoilValue[i].coilAddr == coilAddr)
        {
            if (data)
                mbsCoilValue[i].pData = 1;
            else
                mbsCoilValue[i].pData = 0;
            return 1;
        }
    }
    return i;
}

uint8 MBS_MemReadHoldRegValue(uint16 regAddr, uint8 *Value, uint8 num)
{
    uint8 i, j;
    for (i = 0; i < USER_HOLDREG_NUM; i++)
    {
        if (mbsHoldRegValue[i].regAddr == regAddr) /* ���ּĴ����׵�ַ��ȷ */
        {
            if (mbsHoldRegValue[i + num - 1].regAddr == (regAddr + num - 1)) /* ���ּĴ���β��ַ��ȷ */
            {
                for (j = 0; j < num; j++)
                {
                    *Value++ = (mbsHoldRegValue[i + j].pData) >> 8;
                    *Value++ = (mbsHoldRegValue[i + j].pData);
                }
                return 1; /* ��ȡ�ɹ� */
            }
        }
    }
    return 0;
}

uint8 MBS_MemWriteHoldRegValue(uint16 regAddr, uint8 *Value, uint8 num)
{
    uint8 i, j;
    for (i = 0; i < USER_HOLDREG_NUM; i++)
    {
        if (mbsHoldRegValue[i].regAddr == regAddr) /* ���ּĴ����׵�ַ��ȷ */
        {
            if (mbsHoldRegValue[i + num - 1].regAddr == (regAddr + num - 1)) /* ���ּĴ���β��ַ��ȷ */
            {
                for (j = 0; j < num; j++)
                {
                    mbsHoldRegValue[i + j].pData = (*Value++) << 8;
                    mbsHoldRegValue[i + j].pData |= *Value++;
                }
                return 1; /* д��ɹ� */
            }
        }
    }
    return 0;
}
