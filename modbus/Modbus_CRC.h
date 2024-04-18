#ifndef MODBUS_CRC_H
#define MODBUS_CRC_H
#ifndef uint8
  #define uint8 unsigned char
  #define uint16 unsigned short
#endif
uint16 mb_crc16(uint8 * _pBuf, uint16 _usLen );
#endif /* MODBUS_CRC_H */
  
