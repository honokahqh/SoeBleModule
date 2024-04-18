#include "ymodem.h"

#define TAG "ymodem"
/**
 * ymodem.c
 * @brief ��Ҫʵ��flash_program_bytes���������ڽ�����д��flash
 * @brief init�����һ��ymodem_packet_analysis(timing����),֮��ÿ���յ����ݵ���һ��ymodem_packet_analysis
 * @brief ������Ƴ�ʱ����-�ο�lora_core.c�е�slaver_1s_period����
 * @author Honokahqh
 * @date 2023-08-05
 */

ymodem_session_t ymodem_session;

/**
 * UpdateCRC16
 * @brief ymodem CRC16У���㷨
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
 * @brief ymodem CRC16У���㷨
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
 * @brief ��ʼ��ymodem
 * @author Honokahqh
 * @date 2023-08-05
 */
void ymodem_init()
{
    // ��ʼ��ymodem_session
    memset(&ymodem_session, 0, sizeof(ymodem_session_t));
    ymodem_session.state = YMODEM_STATE_INIT;
    // �ظ�'C'
}

/**
 * ymodem_packet_analysis
 * @brief �յ����ݺ���ô˺������н��� ����1�������� 0����Ymodemֹͣ -1������� 2���������ɹ�
 * @param rxbuffer ���յ�������
 * @param rxlen ���յ������ݳ���
 * @param txbuffer ���͵�����
 * @param txlen ���͵����ݳ���
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
        case YMODEM_STATE_START://��ʼ��:�ļ���+��С
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
                if(ymodem_session.packet_total > 64*1024 / 128)//����64k
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
            if (rxbuffer[0] != 0x01 || rxbuffer[1] != (ymodem_session.packet_number + 1) % 256)//������
            {
				if(rxbuffer[1] < ymodem_session.packet_number + 1)//�ѽ��չ��İ�
				{
					txbuffer[0] = YMODEM_ACK;
					*txlen = 1;
				}
				else//������
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
            
            // ����д��
            flash_program_bytes(AppAddr + ymodem_session.packet_number * 128, &rxbuffer[3], 128);
            // д�����
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
            return -1;  // ���ش���
    }
    return 0;  // ���سɹ�
}
