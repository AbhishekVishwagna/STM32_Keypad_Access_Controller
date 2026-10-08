#include "event_log.h"

static log_entry_t g_buf[EVENT_LOG_SIZE];
static size_t g_head;
static size_t g_count;

void event_log_add(uint32_t ms, uint8_t code)
{
    g_buf[g_head].ms = ms;
    g_buf[g_head].code = code;
    g_head = (g_head + 1u) % EVENT_LOG_SIZE;
    if (g_count < EVENT_LOG_SIZE) {
        g_count++;
    }
}

size_t event_log_count(void)
{
    return g_count;
}

bool event_log_get(size_t index_from_oldest, log_entry_t *out)
{
    size_t start;

    if (index_from_oldest >= g_count) {
        return false;
    }
    start = (g_head + EVENT_LOG_SIZE - g_count) % EVENT_LOG_SIZE;
    *out = g_buf[(start + index_from_oldest) % EVENT_LOG_SIZE];
    return true;
}

void event_log_clear(void)
{
    g_head = 0;
    g_count = 0;
}
