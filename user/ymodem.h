#ifndef _YMODEM_H_
#define _YMODEM_H_

#include "main.h"

#define YMODEM_SOH 0x01        // Start of Header
#define YMODEM_STX 0x02        // Start of Text
#define YMODEM_EOT 0x04        // End of Transmission
#define YMODEM_ACK 0x06        // Acknowledge
#define YMODEM_NAK 0x15        // Negative Acknowledge
#define YMODEM_CAN 0x18        // Cancel
#define YMODEM_CRC 0x43 // ASCII "C", used to request CRC mode
#define YMODEM_SUB 0x1A        // Substitute (used for padding)

#define YMODEM_STATE_IDLE 0  // ����״̬��û�д������ڽ���
#define YMODEM_STATE_INIT 1  // ��ʼ��״̬��׼����ʼ�µĴ���
#define YMODEM_STATE_START 2 // ��ʼ״̬�����Ϳ�ʼ����
#define YMODEM_STATE_DATA 3  // �ļ�����״̬�������ļ�����
#define YMODEM_STATE_EOT 4   // EOT״̬�������ļ��������
#define YMODEM_STATE_EOT2 5  // EOT״̬�������ļ��������
#define YMODEM_STATE_END 6   // ����״̬�����ʹ���������

#define YMODEM_TIMEOUT 30 // ��ʱʱ�䣬��λS
typedef struct
{

    // ��ǰ״̬�����ܵ�ֵ�����ȴ���ʼ�����ڴ��䡢�ȴ�Ӧ�𡢴�����ɵ�
    int state;

    // ��ǰ���ڴ�����ļ���
    char filename[20];

    // ��ǰ���ڴ�����ļ�����
    uint8_t *filedata;
    // ��ǰ���ڴ�����ļ���С
    unsigned long filesize;

    // ��ǰ���
    unsigned int packet_number;

    // �ܹ���Ҫ����/���յĿ���
    unsigned int packet_total;

    // ���ͻ���յ����ݻ�����
    char buffer[10];

    // ��������������ڼ�¼�ط��Ĵ���
    int error_count;

    // ��ʱʱ��
    unsigned int timeout;
} ymodem_session_t;
extern ymodem_session_t ymodem_session;

void ymodem_init(void);
int ymodem_packet_analysis(uint8_t *rxbuffer, uint16_t rxlen, uint8_t *txbuffer, uint16_t *txlen);

#endif
