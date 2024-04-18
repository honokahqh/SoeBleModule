#include "ringbuffer.h"

#define TAG "RingBuffer"

#define ringbuffer_size 20
uint32_t data[ringbuffer_size];
ring_buffer_t ring_buffer = {data, 0, 0, 0, 20}; // buffer head tail size

void write_buffer(uint32_t data)
{
    if (ring_buffer.size < ring_buffer.maxsize)
    {
        ring_buffer.data[ring_buffer.tail] = data;
        ring_buffer.tail = (ring_buffer.tail + 1) % ring_buffer.maxsize;
        ring_buffer.size++;
    }
    else
    {
        LOG_E(TAG, "buffer is full");
    }
}

uint8_t read_buffer(uint32_t *data)
{
    if (ring_buffer.size > 0)
    {
        *data = ring_buffer.data[ring_buffer.head];
        ring_buffer.head = (ring_buffer.head + 1) % ring_buffer.maxsize;
        ring_buffer.size--;
        return 1;
    }
    else
    {
        return 0;
    }
}
