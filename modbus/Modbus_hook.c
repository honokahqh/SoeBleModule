#include "Modbus_CORE.h"
extern MBS_CoilTypes mbsCoil;
extern MBS_CoilValueTypes mbsCoilValue[USER_COIL_NUM];

extern MBS_HoldRegTypes mbsHoldReg;
extern MBS_HoldRegValueTypes mbsHoldRegValue[USER_HOLDREG_NUM];

#if MBS_FUNCTION_01_ENABLE
/*********************************************************************
 * @fn          MBS_Function01H
 *
 * @brief       Modbus01H
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

	num = (MBS_Buf._rxBuff[MBS_FRAME_OPT_NUM] << 8) + MBS_Buf._rxBuff[MBS_FRAME_OPT_NUM + 1];
	if ((num < 1) || (num > USER_COIL_NUM))
	{
		MBS_PortSendAck(MBS_EX_ILLEGAL_DATA_VALUE);
		return;
	}
	coil = (MBS_Buf._rxBuff[MBS_FRAME_START_ADD] << 8) + MBS_Buf._rxBuff[MBS_FRAME_START_ADD + 1];
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
			MBS_PortSendAck(MBS_EX_SLAVE_DEVICE_FAILURE); 
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
 * @brief       Modbus05H
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
		if (MBS_MemWriteCoilState(coil, value) > 1) 
		{
			MBS_PortSendAck(MBS_EX_SLAVE_DEVICE_FAILURE); 
			return;
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
 * @brief       Modbus0F
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
	num = (MBS_Buf._rxBuff[MBS_FRAME_OPT_NUM] << 8) + MBS_Buf._rxBuff[MBS_FRAME_OPT_NUM + 1]; 
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

	coil = (MBS_Buf._rxBuff[MBS_FRAME_START_ADD] << 8) + MBS_Buf._rxBuff[MBS_FRAME_START_ADD + 1];
	if ((coil < mbsCoil._startAddr) || ((coil + num - 1) > mbsCoil._endAddr))
	{
		MBS_PortSendAck(MBS_EX_ILLEGAL_DATA_ADDRESS);
		return;
	}

	if (MBS_MemWriteCoilsState(coil, num, &MBS_Buf._rxBuff[MBS_FRAME_BYTE_NUM + 1]) > 1)
	{
		MBS_PortSendAck(MBS_EX_SLAVE_DEVICE_FAILURE);
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
 * @brief       Modbus03H
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

	num = (MBS_Buf._rxBuff[MBS_FRAME_OPT_NUM] << 8) | MBS_Buf._rxBuff[MBS_FRAME_OPT_NUM + 1];	
	reg = (MBS_Buf._rxBuff[MBS_FRAME_START_ADD] << 8) | MBS_Buf._rxBuff[MBS_FRAME_START_ADD + 1];
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
	i = MBS_MemReadHoldRegValue(reg, &MBS_Buf._txBuff[MBS_Buf._txLen], num);
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
 * @brief       Modbus06H
 *
 * @param       none
 *
 * @return      none
 */
void MBS_Function06H(void)
{
	uint16 reg;
	uint8 i;

	reg = MBS_Buf._rxBuff[MBS_FRAME_START_ADD] << 8 | MBS_Buf._rxBuff[MBS_FRAME_START_ADD + 1]; 

	if (reg < mbsHoldReg._startAddr || reg > mbsHoldReg._endAddr)
	{
		MBS_PortSendAck(MBS_EX_ILLEGAL_DATA_ADDRESS);
		return;
	}
	i = MBS_MemWriteHoldRegValue(reg, &MBS_Buf._rxBuff[4], 1); 
	if (i == 0)
	{
		MBS_PortSendAck(MBS_EX_SLAVE_DEVICE_FAILURE);
		return;
	}
	MBS_PortSendAck(MBS_EX_NONE);
}
#endif

#if MBS_FUNCTION_10_ENABLE
/*********************************************************************
 * @fn          MBS_Function10H
 *
 * @brief       Modbus10H
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

	num = MBS_Buf._rxBuff[MBS_FRAME_OPT_NUM] << 8 | MBS_Buf._rxBuff[MBS_FRAME_OPT_NUM + 1];		
	reg = MBS_Buf._rxBuff[MBS_FRAME_START_ADD] << 8 | MBS_Buf._rxBuff[MBS_FRAME_START_ADD + 1]; 
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
	i = MBS_MemWriteHoldRegValue(reg, &MBS_Buf._rxBuff[7], num); 
	if (i == 0)
	{
		MBS_PortSendAck(MBS_EX_SLAVE_DEVICE_FAILURE);
		return;
	}
	MBS_PortSendAck(MBS_EX_NONE);
}
#endif
