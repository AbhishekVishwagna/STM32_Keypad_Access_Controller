# STM32-Keypad-Access-Controller

A PIN code door controller for STM32, written without an RTOS: a timer scanned 3x4 keypad, a PIN state machine with lockout, a servo latch, status LED, vibration feedback and a UART admin shell.

The decision logic (keypad debounce, PIN state machine, ring buffer, shell) is plain C with no HAL dependency, so it is unit tested on a PC and in CI. Only a thin layer (`app.c`, `actuators.c`) touches the STM32 HAL.

## Project Objective

| Input | Behaviour |
|-------|-----------|
| `0-9` | Enter PIN digits (4 to 6 digits). Entry times out after 10 s. |
| `#`   | Submit the PIN. When unlocked, relocks immediately. |
| `*`   | Clear the current entry. |

- Correct PIN: servo moves to the unlocked position, LED turns green, auto-relocks after 5 s.
- Wrong PIN: long vibration pulse. After 3 failures the controller locks out for 30 s (LED blinks red, keys ignored).
- Everything tunable is in [`app/inc/app_config.h`](app/inc/app_config.h).
- Keystrokes are never logged, and the PIN is compared without an early exit.

In UART shell (115200): `help`, `status`, `log`, `uptime`, `lock`, `pin <digits>`. Access events (`PIN_OK`, `PIN_BAD`, `LOCKOUT_START`, ...) are also printed as they happen.
