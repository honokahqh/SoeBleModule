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

    mbsCoil._startAddr = 12000; /* 起始地址 */
    mbsCoil._endAddr = 12010;   /* 结束地址 */

    for (i = 0; i < 11; i++)
    {
        MBS_MappingCoilInit(&mbsCoilValue[i], i + 12000, 0);
    }

    mbsHoldReg._startAddr = 50000; /* 起始地址 */
    mbsHoldReg._endAddr = 59001;   /* 结束地址 */
    for(i = 0; i < 5;i++)
    {
        MBS_MappingHoldRegInit(&mbsHoldRegValue[i], i + 59000, 0);
    }
	for(i = 0; i < 12;i++)
    {
        MBS_MappingHoldRegInit(&mbsHoldRegValue[i + 5], i + 51000, 0);
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

    for (i = 0; i < USER_COIL_NUM; i++) // 查找符合地址
    {
        if (mbsCoilValue[i].coilAddr == CoilAddr)
            break;
    }
    if (i >= USER_COIL_NUM)
        return i; /*读取失败，地址错误*/

    for (j = 0; j < Len; j++)
        mbsCoilValue[i + j].pData = (data[j / 8] >> (j % 8)) & 0x01;

    return 1; /* 读取成功 */
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
        if (mbsHoldRegValue[i].regAddr == regAddr) /* 保持寄存器首地址正确 */
        {
            if (mbsHoldRegValue[i + num - 1].regAddr == (regAddr + num - 1)) /* 保持寄存器尾地址正确 */
            {
                for (j = 0; j < num; j++)
                {
                    *Value++ = (mbsHoldRegValue[i + j].pData) >> 8;
                    *Value++ = (mbsHoldRegValue[i + j].pData);
                }
                return 1; /* 读取成功 */
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
        if (mbsHoldRegValue[i].regAddr == regAddr) /* 保持寄存器首地址正确 */
        {
            if (mbsHoldRegValue[i + num - 1].regAddr == (regAddr + num - 1)) /* 保持寄存器尾地址正确 */
            {
                for (j = 0; j < num; j++)
                {
                    mbsHoldRegValue[i + j].pData = (*Value++) << 8;
                    mbsHoldRegValue[i + j].pData |= *Value++;
                }
                return 1; /* 写入成功 */
            }
        }
    }
    return 0;
}
