#ifndef __RING_BUFFER__H
#define __RING_BUFFER__H

#include "main.h"

#define MAX_COMMANDS 10
#define MAX_LENGTH 20
typedef struct {
    char buffer[MAX_COMMANDS][MAX_LENGTH];
    uint8_t head;
    uint8_t tail;
    uint8_t size;
} ring_buffer_t;

int write_command(char *command);
int read_command(char *command);

#endif // !__RING_BUFFER__H
