#ifndef MODBUS_USER_H
#define MODBUS_USER_H
#include "main.h"

#define MBS_SelfAddr    120
#if IsApplication
#define Dev_Version     228
#else
#define Dev_Version		227
#endif
/***************************************************************/
#define	MBS_FUNCTION_01_ENABLE			        (1)			
#define	MBS_FUNCTION_02_ENABLE			        (0)
#define	MBS_FUNCTION_03_ENABLE			        (1)
#define	MBS_FUNCTION_04_ENABLE			        (0)
#define	MBS_FUNCTION_05_ENABLE			        (1)
#define	MBS_FUNCTION_06_ENABLE			        (1)
#define	MBS_FUNCTION_0F_ENABLE			        (1)
#define	MBS_FUNCTION_10_ENABLE			        (1)

#define USER_COIL_NUM					        (11)		
#define USER_HOLDREG_NUM				        (17)		
#define USER_DISINPUT_NUM				        (0)
#define USER_INPUTREG_NUM				        (0)

#define MBS_PORT_RXBUFF_SIZE			        64
#define MBS_PORT_TXBUFF_SIZE			        64

#define MBS_Addr                                0
#define MBS_Ver                                 1

#define BleMode     0
#define WireMode    1
#define Coil_MusicPause                         0   
#define Coil_BleMode                            1
#define Coil_LastMusic                          2
#define Coil_NextMusic                          3
#define Coil_MusicChange                        4
#define Coil_MusicMode                          5
#define Coil_MusicVolume                        6
#define Coil_BleReConnect                       7
#define Coil_BleReset                           8
#define Coil_BleConnectState                    9
#define Coil_MusicIsPlay                        10

#define Reg_Volume                              5
#define Reg_BleMac                              6
#define Reg_LoopIndex                           12
#define Reg_MusicIndex                          13
#define Reg_MusicTotal                          14
#define Reg_MusicDuration                       15
#define Reg_MusicProgress                       16


void MBS_MappingInit(void);
void MBS_Data_Init(void);
uint8 MBS_MemReadCoilState(uint16 coilAddr);
uint8 MBS_MemWriteCoilState(uint16 coilAddr, uint16 data);
uint16 MBS_MemWriteCoilsState(uint16 CoilAddr, uint16 Len, uint8 *data);
uint8 MBS_MemReadHoldRegValue(uint16 regAddr, uint8 *Value, uint8 num);
uint8 MBS_MemWriteHoldRegValue(uint16 regAddr, uint8 *Value, uint8 num);
#endif /* MODBUS_USER_H */
