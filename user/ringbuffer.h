#ifndef __RING_BUFFER__H
#define __RING_BUFFER__H

#include "main.h"

typedef struct {
    uint32_t *data;
    uint8_t head;
    uint8_t tail;
    uint8_t size;
    uint8_t maxsize;
} ring_buffer_t;
extern ring_buffer_t ring_buffer;

void write_buffer(uint32_t data);
uint8_t read_buffer(uint32_t *data); // 1: success 0: fail

#endif // !__RING_BUFFER__H
