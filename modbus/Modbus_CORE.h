#ifndef MODBUS_CORE_H
#define MODBUS_CORE_H
#include "Modbus_CRC.h"
#include "Modbus_USER.h"
#include "Modbus_hook.h"

/***************************************************************/
#define MBS_FRAME_SLAVER_ADDR			      (0)
#define MBS_FRAME_FUNCTION_CODE			    (1)
#define MBS_FRAME_START_ADD				      (2)
#define MBS_FRAME_OPT_NUM				        (4)
#define MBS_FRAME_WRITE_ONE_VALUE		    (4)
#define MBS_FRAME_BYTE_NUM				      (6)
/***************************************************************/
//#define	MBS_FALUT_ACK_ENABLE			        (1)
/***************************************************************/
#define MBS_FRAME_SLAVER_ADDR			      (0)
#define MBS_FRAME_FUNCTION_CODE			    (1)
#define MBS_FRAME_START_ADD				      (2)
#define MBS_FRAME_OPT_NUM				        (4)
#define MBS_FRAME_WRITE_ONE_VALUE		    (4)
#define MBS_FRAME_BYTE_NUM				      (6)
/***************************************************************/
typedef enum{
  MBS_EX_NONE = 0x00,	               
  MBS_EX_ILLEGAL_FUNCTION = 0x01,    
  MBS_EX_ILLEGAL_DATA_ADDRESS = 0x02, 
  MBS_EX_ILLEGAL_DATA_VALUE = 0x03,   
  MBS_EX_SLAVE_DEVICE_FAILURE = 0x04, 
  MBS_EX_ACKNOWLEDGE = 0x05,         
  MBS_EX_SLAVE_BUSY = 0x06,           
  MBS_EX_MEMORY_PARITY_ERROR = 0x08, 
  MBS_EX_GATEWAY_PATH_FAILED = 0x0A,  
  MBS_EX_GATEWAY_TGT_FAILED = 0x0B,   
}MBS_EX_STATE;
/***************************************************************/
typedef struct{
  uint8 _rxBuff[MBS_PORT_RXBUFF_SIZE];
  uint8 _rxLen;
  uint8	_rxState;

  uint8	_txBuff[MBS_PORT_TXBUFF_SIZE];
  uint8	_txLen;
  uint8	_txState;

}MBS_PortFrameTypes;
extern MBS_PortFrameTypes	MBS_Buf;

typedef struct
{
	uint16				coilAddr;		
	uint16				pData;		
}MBS_CoilValueTypes;
extern MBS_CoilValueTypes       mbsCoilValue[USER_COIL_NUM];

typedef struct {
	MBS_CoilValueTypes			*const _Value;
	uint16					        _startAddr;
	uint16					        _endAddr;
	uint16					        const _num;
}MBS_CoilTypes;

typedef struct
{
    uint16                   regAddr;      
    uint16                   pData;       
}MBS_HoldRegValueTypes;
extern MBS_HoldRegValueTypes       mbsHoldRegValue[USER_HOLDREG_NUM];

typedef struct {
    MBS_HoldRegValueTypes     *const _Value;
    uint16                    _startAddr;
    uint16                    _endAddr;
    uint16                    const _num;
}MBS_HoldRegTypes;

extern volatile uint8_t mbs_has_data;
/***************************************************************/
void MBS_CorePoll(void);
void MBS_CoreAnalyze(void);
void MBS_PortSendAck(MBS_EX_STATE ackCode);
void MBS_PortSendWithCRC(uint8 *buf, uint8 len);
void MBS_PhysicalSendBuff(uint8 *buf,uint8 len);
void MBS_PortSendAck(MBS_EX_STATE ackCode);
void MBS_DataSyn(void);

#endif /* MODBUS_CORE_H */
