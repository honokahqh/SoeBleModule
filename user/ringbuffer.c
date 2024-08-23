#include "ringbuffer.h"

#define TAG "RingBuffer"

ring_buffer_t ring_buffer;

int write_command(char *command)
{
    if (ring_buffer.size == MAX_COMMANDS)
    {
        LOG_E(TAG, "Buffer is full");
        return -1;
    }
    if (strlen(command) >= MAX_LENGTH)
    {
        LOG_E(TAG, "Command is too long");
        return -1;
    }
    strcpy(ring_buffer.buffer[ring_buffer.tail], command);
    ring_buffer.tail = (ring_buffer.tail + 1) % MAX_COMMANDS;    
    ring_buffer.size++;
}

int read_command(char *command)
{
    if (ring_buffer.size == 0)
    {
        // LOG_E(TAG, "Buffer is empty");
        return -1;
    }
    strcpy(command, ring_buffer.buffer[ring_buffer.head]);
    ring_buffer.head = (ring_buffer.head + 1) % MAX_COMMANDS;
    ring_buffer.size--;
    return 0;
}