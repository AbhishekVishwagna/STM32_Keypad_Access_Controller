#ifndef KEYPAD_H
#define KEYPAD_H

#include <stdbool.h>
#include <stdint.h>
#include "ringbuf.h"

#define KEYPAD_ROWS 4u
#define KEYPAD_COLS 3u

/*
 * Hardware hooks, so the scan/debounce logic has no HAL dependency
 * and can be unit-tested on a PC.
 *   drive_col(col, true)  -> pull that column LOW (active)
 *   drive_col(col, false) -> release that column (high-Z / high)
 *   read_rows()           -> bit r set when row r reads LOW (key down)
 */
typedef struct {
    void    (*drive_col)(uint8_t col, bool active);
    uint8_t (*read_rows)(void);
} keypad_hw_t;

typedef struct {
    const keypad_hw_t *hw;
    uint8_t col;                                   /* column sampled on this tick */
    uint8_t count[KEYPAD_ROWS][KEYPAD_COLS];       /* per-key debounce integrator */
    uint8_t pressed[KEYPAD_ROWS][KEYPAD_COLS];     /* debounced state */
    ringbuf_t events;
    uint8_t ev_storage[8];
} keypad_t;

void keypad_init(keypad_t *k, const keypad_hw_t *hw);
void keypad_tick(keypad_t *k);                     /* call every 1 ms (ISR context is fine) */
bool keypad_get_key(keypad_t *k, char *key);       /* pops one debounced key press */

#endif
