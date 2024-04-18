#include "ymodem.h"

#define TAG "ymodem"
/**
 * ymodem.c
 * @brief 需要实现flash_program_bytes函数，用于将数据写入flash
 * @brief init后调用一次ymodem_packet_analysis(timing合适),之后每次收到数据调用一次ymodem_packet_analysis
 * @brief 自行设计超时函数-参考lora_core.c中的slaver_1s_period函数
 * @author Honokahqh
 * @date 2023-08-05
 */

ymodem_session_t ymodem_session;

/**
 * UpdateCRC16
 * @brief ymodem CRC16校验算法
 * @author Honokahqh
 * @date 2023-08-05
 */
static uint16_t UpdateCRC16(uint16_t crc_in, uint8_t byte)
{
    uint32_t crc = crc_in;
    uint32_t in = byte | 0x100;
    do
    {
        crc <<= 1;
        in <<= 1;
        if (in & 0x100)
            ++crc;
        if (crc & 0x10000)
            crc ^= 0x1021;
    } while (!(in & 0x10000));

    return crc & 0xffffu;
}

/**
 * YmodemCRC
 * @brief ymodem CRC16校验算法
 * @author Honokahqh
 * @date 2023-08-05
 */
static uint16_t YmodemCRC(uint8_t *data, uint32_t len)
{
    uint32_t crc = 0;
    uint8_t *dataEnd;
    dataEnd = data + len;

    while (data < dataEnd)
        crc = UpdateCRC16(crc, *data++);

    crc = UpdateCRC16(crc, 0);
    crc = UpdateCRC16(crc, 0);

    return crc & 0xffff;
}

/**
 * ymodem_init
 * @brief 初始化ymodem
 * @author Honokahqh
 * @date 2023-08-05
 */
void ymodem_init()
{
    // 初始化ymodem_session
    memset(&ymodem_session, 0, sizeof(ymodem_session_t));
    ymodem_session.state = YMODEM_STATE_INIT;
    // 回复'C'
}

/**
 * ymodem_packet_analysis
 * @brief 收到数据后调用此函数进行解析 返回1代表正常 0代表Ymodem停止 -1代表错误 2代表升级成功
 * @param rxbuffer 接收到的数据
 * @param rxlen 接收到的数据长度
 * @param txbuffer 发送的数据
 * @param txlen 发送的数据长度
 * @author Honokahqh
 * @date 2023-08-05
 */
int ymodem_packet_analysis(uint8_t *rxbuffer, uint16_t rxlen, uint8_t *txbuffer, uint16_t *txlen) {
	uint16_t i,Data_offset;

    ymodem_session.timeout = 0;

    if(rxbuffer[0] == YMODEM_CAN && rxbuffer[1] == YMODEM_CAN)
    {
        memset(&ymodem_session, 0, sizeof(ymodem_session_t));
        ymodem_session.state = YMODEM_STATE_IDLE;
        return 0;
    }
    switch (ymodem_session.state) {
        case YMODEM_STATE_INIT:
            txbuffer[0] = YMODEM_CRC;
            *txlen = 1;
            ymodem_session.state = YMODEM_STATE_START;
            return 1;
        case YMODEM_STATE_START://起始包:文件名+大小
            if (rxbuffer[0] != 0x01 || rxbuffer[1] != 0x00 || rxbuffer[2] != 0xFF)
            {
                // txbuffer[0] = NAK;
                *txlen = 0;
                // ymodem_session.error_count++;
                return -1;
            }   
            else
            {
                i = 0;
                while (rxbuffer[3 + i] != 0x00 && i < 128)
                {
                    ymodem_session.filename[i] = rxbuffer[3 + i];
                    i++;
                }
                if (i >= 128)
                    return -1;
                ymodem_session.filename[i] = 0;

                Data_offset = i + 4;
                i = 0;
                while (rxbuffer[Data_offset + i] >= 0x30 && rxbuffer[Data_offset + i] < 0x3A && i < 10)
                {
                    ymodem_session.filesize = (ymodem_session.filesize * 10) + (rxbuffer[Data_offset + i] - 0x30);
                    i++;
                }
                ymodem_session.packet_total = (ymodem_session.filesize + 128 - 1) / 128;
                if(ymodem_session.packet_total > 64*1024 / 128)//大于64k
                {
                    txbuffer[0] = YMODEM_CAN;
                    txbuffer[1] = YMODEM_CAN;
                    *txlen = 2;
                    memset(&ymodem_session, 0, sizeof(ymodem_session_t));
                    return -1;
                }
                txbuffer[0] = YMODEM_ACK;
                txbuffer[1] = YMODEM_CRC;
                *txlen = 2;
                ymodem_session.state = YMODEM_STATE_DATA;
                return 1;
            }
        case YMODEM_STATE_DATA:
			if(YmodemCRC(&rxbuffer[3], 130))
            {
                txbuffer[0] = YMODEM_NAK;
                *txlen = 1;
                return -1;
            }
            if (rxbuffer[0] != 0x01 || rxbuffer[1] != (ymodem_session.packet_number + 1) % 256)//包错误
            {
				if(rxbuffer[1] < ymodem_session.packet_number + 1)//已接收过的包
				{
					txbuffer[0] = YMODEM_ACK;
					*txlen = 1;
				}
				else//跳包了
				{
					txbuffer[0] = YMODEM_CAN;
					txbuffer[1] = YMODEM_CAN;
					*txlen = 2;
					memset(&ymodem_session, 0, sizeof(ymodem_session_t));
				}
                
                ymodem_session.error_count++;
                return -1;
            } 
            ymodem_session.error_count ++;
            
            // 数据写入
            flash_program_bytes(AppAddr + ymodem_session.packet_number * 128, &rxbuffer[3], 128);
            // 写入完毕
            ymodem_session.packet_number++;
            if (ymodem_session.packet_number >= ymodem_session.packet_total)
            {
                txbuffer[0] = YMODEM_ACK;
                *txlen = 1;
                ymodem_session.state = YMODEM_STATE_EOT;
            }
            else
            {
                txbuffer[0] = YMODEM_ACK;
                *txlen = 1;
            }
            ymodem_session.error_count = 0;
            return 1;
        case YMODEM_STATE_EOT:
			if(rxbuffer[0] != YMODEM_EOT)
            {
                ymodem_session.error_count++;
                return -1;
            }
            txbuffer[0] = YMODEM_ACK;
            txbuffer[1] = YMODEM_CRC;
            *txlen = 2;
            ymodem_session.state = YMODEM_STATE_END;
            break;
        case YMODEM_STATE_END:  
            if (rxbuffer[0] != 0x01 || rxbuffer[1] != 0x00 || rxbuffer[2] != 0xFF)
            {
                txbuffer[0] = YMODEM_NAK;
                *txlen = 1;
                ymodem_session.error_count++;
                return -1;
            }   
            else
            {   
                memset(&ymodem_session, 0, sizeof(ymodem_session_t));
                txbuffer[0] = YMODEM_ACK;
                *txlen = 1;
                ymodem_session.state = YMODEM_STATE_IDLE;
                return 2;
            }
        default:
            return -1;  // 返回错误
    }
    return 0;  // 返回成功
}
