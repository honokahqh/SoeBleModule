#include "Modbus_CORE.h"
extern MBS_CoilTypes mbsCoil;
extern MBS_CoilValueTypes mbsCoilValue[USER_COIL_NUM];

extern MBS_HoldRegTypes mbsHoldReg;
extern MBS_HoldRegValueTypes mbsHoldRegValue[USER_HOLDREG_NUM];

#if MBS_FUNCTION_01_ENABLE
/*********************************************************************
 * @fn          MBS_Function01H
 *
 * @brief       Modbus01H£¬¶ÁÏßÈ¦
 *
 * @param       none
 *
 * @return      none
 */
void MBS_Function01H(void)
{
	uint16 coil;
	uint16 num;
	uint16 i;
	uint16 m;
	uint8 status[MBS_PORT_TXBUFF_SIZE] = {0};
	uint8 temp;

	num = (MBS_Buf._rxBuff[MBS_FRAME_OPT_NUM] << 8) + MBS_Buf._rxBuff[MBS_FRAME_OPT_NUM + 1]; /* ¼Ä´æÆ÷¸öÊý */
	if ((num < 1) || (num > USER_COIL_NUM))
	{
		MBS_PortSendAck(MBS_EX_ILLEGAL_DATA_VALUE);
		return;
	}
	coil = (MBS_Buf._rxBuff[MBS_FRAME_START_ADD] << 8) + MBS_Buf._rxBuff[MBS_FRAME_START_ADD + 1]; /* ¼Ä´æÆ÷ºÅ */
	if ((coil < mbsCoil._startAddr) || ((coil + num - 1) > mbsCoil._endAddr))
	{
		MBS_PortSendAck(MBS_EX_ILLEGAL_DATA_ADDRESS);
		return;
	}
	m = (num + 7) / 8;
	for (i = 0; i < m; i++)
	{
		status[i] = 0;
	}
	for (i = 0; i < num; i++)
	{
		temp = MBS_MemReadCoilState(coil + i);
		switch (temp)
		{
		case 0:
			// status[i / 8] |= (0 << (i % 8));
			break;
		case 1:
			status[i / 8] |= (1 << (i % 8));
			break;
		default:
			MBS_PortSendAck(MBS_EX_SLAVE_DEVICE_FAILURE); /*·µ»Ø´íÎó²¢ÇÒÍË³ö*/
			return;
		}
	}

	MBS_Buf._txLen = 0;
	MBS_Buf._txBuff[MBS_Buf._txLen++] = MBS_Buf._rxBuff[0];
	MBS_Buf._txBuff[MBS_Buf._txLen++] = MBS_Buf._rxBuff[1];
	MBS_Buf._txBuff[MBS_Buf._txLen++] = m;
	for (i = 0; i < m; i++)
	{
		MBS_Buf._txBuff[MBS_Buf._txLen++] = status[i];
	}
	MBS_PortSendWithCRC(MBS_Buf._txBuff, MBS_Buf._txLen);
}
#endif

#if MBS_FUNCTION_05_ENABLE
/*********************************************************************
 * @fn          MBS_Function05H
 *
 * @brief       Modbus05H£¬Ð´µ¥¸öÏßÈ¦
 *
 * @param       none
 *
 * @return      none
 */
void MBS_Function05H(void)
{
	uint16 coil;
	uint16 value;

	value = (MBS_Buf._rxBuff[MBS_FRAME_WRITE_ONE_VALUE] << 8) + MBS_Buf._rxBuff[MBS_FRAME_WRITE_ONE_VALUE + 1];
	if ((value != 0x0000) && (value != 0xff00))
	{
		MBS_PortSendAck(MBS_EX_ILLEGAL_DATA_VALUE);
		return;
	}

	coil = (MBS_Buf._rxBuff[MBS_FRAME_START_ADD] << 8) + MBS_Buf._rxBuff[MBS_FRAME_START_ADD + 1];
	if ((coil >= mbsCoil._startAddr) && (coil <= mbsCoil._endAddr))
	{
		if (MBS_MemWriteCoilState(coil, value) > 1) // Ê§°Ü
		{
			MBS_PortSendAck(MBS_EX_SLAVE_DEVICE_FAILURE); /*·µ»Ø´íÎó²¢ÇÒÍË³ö*/
			return;
		}
		else
		{
			if(coil == mbsCoilValue[Coil_IR_Switch].coilAddr)
			{
				write_buffer(IR_AirConCtrl << 16 | IR_Switch << 8 | (!value));
			}
			else if(coil == mbsCoilValue[Coil_IR_Brand_Set].coilAddr)
			{
				write_buffer(IR_BrandSet << 16 | mbsHoldRegValue[Reg_IR_Brand].pData);
			}
			else if(coil == mbsCoilValue[Coil_IR_Brand_Study].coilAddr)
			{
				write_buffer(IR_BrandStudyMode << 16 | 0);
			}
			MBS_PortSendAck(MBS_EX_NONE);
		}
	}
	else
	{
		MBS_PortSendAck(MBS_EX_ILLEGAL_DATA_ADDRESS);
	}
}
#endif

#if MBS_FUNCTION_05_ENABLE
/*********************************************************************
 * @fn          MBS_Function0FH
 *
 * @brief       Modbus0FH£¬Ð´ÏßÈ¦
 *
 * @param       none
 *
 * @return      none
 */
void MBS_Function0FH(void)
{
	uint16 coil;
	uint16 num;
	uint8 byteNum;
	num = (MBS_Buf._rxBuff[MBS_FRAME_OPT_NUM] << 8) + MBS_Buf._rxBuff[MBS_FRAME_OPT_NUM + 1]; /* ¼Ä´æÆ÷¸öÊý */
	if ((num < 1) || (num > USER_COIL_NUM))
	{
		MBS_PortSendAck(MBS_EX_ILLEGAL_DATA_VALUE);
		return;
	}

	byteNum = num / 8;
	if ((num % 8) != 0)
	{
		byteNum++;
	}
	if (byteNum != MBS_Buf._rxBuff[MBS_FRAME_BYTE_NUM])
	{
		MBS_PortSendAck(MBS_EX_ILLEGAL_DATA_VALUE);
		return;
	}

	coil = (MBS_Buf._rxBuff[MBS_FRAME_START_ADD] << 8) + MBS_Buf._rxBuff[MBS_FRAME_START_ADD + 1]; /* ¼Ä´æÆ÷ºÅ */
	if ((coil < mbsCoil._startAddr) || ((coil + num - 1) > mbsCoil._endAddr))
	{
		MBS_PortSendAck(MBS_EX_ILLEGAL_DATA_ADDRESS);
		return;
	}

	if (MBS_MemWriteCoilsState(coil, num, &MBS_Buf._rxBuff[MBS_FRAME_BYTE_NUM + 1]) > 1)
	{
		MBS_PortSendAck(MBS_EX_SLAVE_DEVICE_FAILURE); /*·µ»Ø´íÎó²¢ÇÒÍË³ö*/
		return;
	}
	else
	{
		MBS_PortSendAck(MBS_EX_NONE);
	}
}
#endif

#if MBS_FUNCTION_03_ENABLE
/*********************************************************************
 * @fn          MBS_Function03H
 *
 * @brief       Modbus03H£¬¶Á±£³Ö¼Ä´æÆ÷
 *
 * @param       none
 *
 * @return      none
 */
void MBS_Function03H(void)
{
	uint16 reg;
	uint16 num;
	uint8 i;

	num = (MBS_Buf._rxBuff[MBS_FRAME_OPT_NUM] << 8) | MBS_Buf._rxBuff[MBS_FRAME_OPT_NUM + 1];	  /* ¼Ä´æÆ÷¸öÊý */
	reg = (MBS_Buf._rxBuff[MBS_FRAME_START_ADD] << 8) | MBS_Buf._rxBuff[MBS_FRAME_START_ADD + 1]; /* ¼Ä´æÆ÷ºÅ */
	if (num < 1 || num > USER_HOLDREG_NUM * 2)
	{
		MBS_PortSendAck(MBS_EX_ILLEGAL_DATA_VALUE);
		return;
	}
	if (reg < mbsHoldReg._startAddr || (reg + num - 1) > mbsHoldReg._endAddr)
	{
		MBS_PortSendAck(MBS_EX_ILLEGAL_DATA_ADDRESS);
		return;
	}
	MBS_Buf._txLen = 0;
	MBS_Buf._txBuff[MBS_Buf._txLen++] = MBS_Buf._rxBuff[0];
	MBS_Buf._txBuff[MBS_Buf._txLen++] = MBS_Buf._rxBuff[1];
	MBS_Buf._txBuff[MBS_Buf._txLen++] = num * 2;
	i = MBS_MemReadHoldRegValue(reg, &MBS_Buf._txBuff[MBS_Buf._txLen], num); /* ÀàËÆmemcpy ¶Á */
	if (i == 0)
	{
		MBS_PortSendAck(MBS_EX_SLAVE_DEVICE_FAILURE);
		return;
	}
	MBS_Buf._txLen += num * 2;
	MBS_PortSendWithCRC(MBS_Buf._txBuff, MBS_Buf._txLen);
}
#endif
#if MBS_FUNCTION_06_ENABLE
/*********************************************************************
 * @fn          MBS_Function06H
 *
 * @brief       Modbus06H£¬Ð´1¸ö±£³Ö¼Ä´æÆ÷
 *
 * @param       none
 *
 * @return      none
 */
void MBS_Function06H(void)
{
	uint16 reg;
	uint8 i;

	reg = MBS_Buf._rxBuff[MBS_FRAME_START_ADD] << 8 | MBS_Buf._rxBuff[MBS_FRAME_START_ADD + 1]; /* ¼Ä´æÆ÷ºÅ */

	if (reg < mbsHoldReg._startAddr || reg > mbsHoldReg._endAddr)
	{
		MBS_PortSendAck(MBS_EX_ILLEGAL_DATA_ADDRESS);
		return;
	}
	i = MBS_MemWriteHoldRegValue(reg, &MBS_Buf._rxBuff[4], 1); /* ÀàËÆmemcpy Ð´ */
	if (i == 0)
	{
		MBS_PortSendAck(MBS_EX_SLAVE_DEVICE_FAILURE);
		return;
	}
	if (reg == mbsHoldRegValue[Reg_IR_WindSpeed].regAddr && mbsHoldRegValue[Reg_IR_WindSpeed].pData < 3)
    {
		if(IR_state.isSupportSix && mbsHoldRegValue[Reg_IR_WindSpeed].pData != 0)
			write_buffer(IR_AirConCtrl << 16 | (IR_WindSpeed << 8) | ((mbsHoldRegValue[Reg_IR_WindSpeed].pData * 2) - 1));
		else
			write_buffer(IR_AirConCtrl << 16 | (IR_WindSpeed << 8) | mbsHoldRegValue[Reg_IR_WindSpeed].pData);
    }
    else if (reg == mbsHoldRegValue[Reg_IR_Mode].regAddr)
    {
        write_buffer(IR_AirConCtrl << 16 | (IR_Mode << 8) | mbsHoldRegValue[Reg_IR_Mode].pData);
    }
    else if (reg == mbsHoldRegValue[Reg_IR_Temp].regAddr)
    {
        write_buffer(IR_AirConCtrl << 16 | (IR_TempSet << 8) | mbsHoldRegValue[Reg_IR_Temp].pData - 16);
    }
	MBS_PortSendAck(MBS_EX_NONE);
}
#endif

#if MBS_FUNCTION_10_ENABLE
/*********************************************************************
 * @fn          MBS_Function10H
 *
 * @brief       Modbus10H£¬Ð´¶à¸ö±£³Ö¼Ä´æÆ÷
 *
 * @param       none
 *
 * @return      none
 */
void MBS_Function10H(void)
{
	uint16 reg;
	uint16 num;
	uint8 i;

	num = MBS_Buf._rxBuff[MBS_FRAME_OPT_NUM] << 8 | MBS_Buf._rxBuff[MBS_FRAME_OPT_NUM + 1];		/* ¼Ä´æÆ÷¸öÊý */
	reg = MBS_Buf._rxBuff[MBS_FRAME_START_ADD] << 8 | MBS_Buf._rxBuff[MBS_FRAME_START_ADD + 1]; /* ¼Ä´æÆ÷ºÅ */
	if (num < 1 || num > USER_HOLDREG_NUM * 2)
	{
		MBS_PortSendAck(MBS_EX_ILLEGAL_DATA_VALUE);
		return;
	}
	if (reg < mbsHoldReg._startAddr || (reg + num - 1) > mbsHoldReg._endAddr)
	{
		MBS_PortSendAck(MBS_EX_ILLEGAL_DATA_ADDRESS);
		return;
	}
	i = MBS_MemWriteHoldRegValue(reg, &MBS_Buf._rxBuff[7], num); /* ÀàËÆmemcpy Ð´ */
	if (i == 0)
	{
		MBS_PortSendAck(MBS_EX_SLAVE_DEVICE_FAILURE);
		return;
	}
	MBS_PortSendAck(MBS_EX_NONE);
}
#endif
