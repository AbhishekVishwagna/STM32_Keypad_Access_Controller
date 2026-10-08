#ifndef EVENT_LOG_H
#define EVENT_LOG_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define EVENT_LOG_SIZE 32u

typedef struct {
    uint32_t ms;
    uint8_t  code;   /* acc_event_t */
} log_entry_t;

void   event_log_add(uint32_t ms, uint8_t code);
size_t event_log_count(void);
bool   event_log_get(size_t index_from_oldest, log_entry_t *out);
void   event_log_clear(void);

#endif
