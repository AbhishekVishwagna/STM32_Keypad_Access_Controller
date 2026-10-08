#ifndef APP_CONFIG_H
#define APP_CONFIG_H

/* ---- PIN policy ---- */
#define PIN_MIN_LEN           4u
#define PIN_MAX_LEN           6u
#define PIN_DEFAULT           "1234"   /* change via the UART shell: pin <digits> */

/* ---- Access policy (milliseconds) ---- */
#define MAX_BAD_ATTEMPTS      3u
#define LOCKOUT_MS            30000u
#define UNLOCK_HOLD_MS        5000u
#define ENTRY_TIMEOUT_MS      10000u

/* ---- Keypad ---- */
/* One sample per key every full scan (3 columns x 1 ms = 3 ms). 5 => ~15 ms debounce. */
#define KEY_DEBOUNCE_SAMPLES  5u

/* ---- Servo pulse widths in microseconds (timer tick must be 1 MHz) ---- */
#define SERVO_LOCKED_US       1000u
#define SERVO_UNLOCKED_US     2000u

/* ---- Vibration feedback (ms) ---- */
#define VIB_KEY_MS            25u
#define VIB_OK_MS             120u
#define VIB_BAD_MS            400u
#define VIB_LOCKOUT_MS        900u

/* Print access events (never PIN digits) on the UART as they happen. */
#define LOG_EVENTS_TO_UART    1

#endif
