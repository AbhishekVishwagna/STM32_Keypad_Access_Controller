#include "keypad.h"
#include "app_config.h"
#include "test_util.h"

static int g_active_col = -1;
static int g_press_r = -1;
static int g_press_c = -1;

static void fake_drive(uint8_t col, bool active)
{
    if (active) {
        g_active_col = col;
    } else if (g_active_col == col) {
        g_active_col = -1;
    }
}

static uint8_t fake_rows(void)
{
    if (g_press_r >= 0 && g_press_c == g_active_col) {
        return (uint8_t)(1u << g_press_r);
    }
    return 0;
}

static const keypad_hw_t HW = {fake_drive, fake_rows};

static void run(keypad_t *k, int ticks)
{
    for (int i = 0; i < ticks; i++) {
        keypad_tick(k);
    }
}

static int drain(keypad_t *k, char *out, int max)
{
    int n = 0;
    char c;

    while (keypad_get_key(k, &c) && n < max) {
        out[n++] = c;
    }
    return n;
}

int main(void)
{
    keypad_t k;
    char keys[8];

    keypad_init(&k, &HW);

    /* press '0' (row 3, col 1): exactly one event, even while held */
    g_press_r = 3; g_press_c = 1;
    run(&k, 100);
    CHECK(drain(&k, keys, 8) == 1 && keys[0] == '0');
    run(&k, 300);
    CHECK(drain(&k, keys, 8) == 0);

    /* release, then press again: a second event */
    g_press_r = -1;
    run(&k, 100);
    g_press_r = 3; g_press_c = 1;
    run(&k, 100);
    CHECK(drain(&k, keys, 8) == 1 && keys[0] == '0');
    g_press_r = -1;
    run(&k, 100);

    /* every key maps correctly */
    static const char EXPECT[4][3] = {{'1','2','3'},{'4','5','6'},{'7','8','9'},{'*','0','#'}};
    for (int r = 0; r < 4; r++) {
        for (int c = 0; c < 3; c++) {
            g_press_r = r; g_press_c = c;
            run(&k, 100);
            CHECK(drain(&k, keys, 8) == 1 && keys[0] == EXPECT[r][c]);
            g_press_r = -1;
            run(&k, 100);
        }
    }

    /* a glitch shorter than the debounce window produces nothing */
    g_press_r = 0; g_press_c = 0;
    run(&k, 3 * 2);          /* only 2 scans */
    g_press_r = -1;
    run(&k, 100);
    CHECK(drain(&k, keys, 8) == 0);

    TEST_DONE("test_keypad");
}
