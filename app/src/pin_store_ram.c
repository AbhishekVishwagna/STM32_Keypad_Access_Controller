#include "pin_store.h"
#include "app_config.h"
#include <string.h>

static char g_pin[PIN_MAX_LEN + 1u];
static bool g_valid;

bool pin_store_load(char *out, size_t out_size)
{
    size_t n = strlen(g_pin);

    if (!g_valid || out_size < n + 1u) {
        return false;
    }
    memcpy(out, g_pin, n + 1u);
    return true;
}

bool pin_store_save(const char *pin)
{
    size_t n = strlen(pin);

    if (n > PIN_MAX_LEN) {
        return false;
    }
    memset(g_pin, 0, sizeof g_pin);
    memcpy(g_pin, pin, n);
    g_valid = true;
    return true;
}
