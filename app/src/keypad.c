#include "keypad.h"
#include "app_config.h"
#include <string.h>

static const char KEYMAP[KEYPAD_ROWS][KEYPAD_COLS] = {
    {'1', '2', '3'},
    {'4', '5', '6'},
    {'7', '8', '9'},
    {'*', '0', '#'},
};

void keypad_init(keypad_t *k, const keypad_hw_t *hw)
{
    memset(k, 0, sizeof *k);
    k->hw = hw;
    ringbuf_init(&k->events, k->ev_storage, (uint16_t)sizeof k->ev_storage);

    for (uint8_t c = 0; c < KEYPAD_COLS; c++) {
        hw->drive_col(c, false);
    }
    hw->drive_col(0, true);
}

void keypad_tick(keypad_t *k)
{
    /* The active column was driven at the end of the previous tick,
     * so it has had a full millisecond to settle before we read. */
    uint8_t rows = k->hw->read_rows();
    uint8_t c = k->col;

    for (uint8_t r = 0; r < KEYPAD_ROWS; r++) {
        bool down = ((rows >> r) & 1u) != 0u;
        uint8_t *cnt = &k->count[r][c];

        if (down) {
            if (*cnt < KEY_DEBOUNCE_SAMPLES) {
                (*cnt)++;
            }
            if (*cnt == KEY_DEBOUNCE_SAMPLES && !k->pressed[r][c]) {
                k->pressed[r][c] = 1u;
                (void)ringbuf_put(&k->events, (uint8_t)KEYMAP[r][c]);
            }
        } else {
            if (*cnt > 0u) {
                (*cnt)--;
            }
            if (*cnt == 0u) {
                k->pressed[r][c] = 0u;
            }
        }
    }

    k->hw->drive_col(c, false);
    c = (uint8_t)((c + 1u) % KEYPAD_COLS);
    k->col = c;
    k->hw->drive_col(c, true);
}

bool keypad_get_key(keypad_t *k, char *key)
{
    uint8_t b;

    if (!ringbuf_get(&k->events, &b)) {
        return false;
    }
    *key = (char)b;
    return true;
}
