#ifndef ACCESS_H
#define ACCESS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "app_config.h"

typedef enum {
    ACC_LOCKED = 0,
    ACC_ENTERING,
    ACC_UNLOCKED,
    ACC_LOCKOUT
} acc_state_t;

/* Order matters: access_event_name() indexes a table with this enum. */
typedef enum {
    ACC_EV_KEY = 0,       /* a digit was accepted (the digit itself is never reported) */
    ACC_EV_CLEARED,       /* '*' cleared the entry */
    ACC_EV_TIMEOUT,       /* entry timed out */
    ACC_EV_PIN_OK,
    ACC_EV_PIN_BAD,
    ACC_EV_LOCKOUT_START,
    ACC_EV_LOCKOUT_END,
    ACC_EV_RELOCKED,
    ACC_EV_PIN_CHANGED,
    ACC_EV_COUNT
} acc_event_t;

typedef void (*acc_notify_fn)(acc_event_t ev, void *user);

typedef struct {
    acc_state_t state;
    char pin[PIN_MAX_LEN + 1u];
    char entry[PIN_MAX_LEN + 1u];
    uint8_t entry_len;
    uint8_t overflow;      /* more digits than PIN_MAX_LEN were typed: entry can never match */
    uint8_t bad_attempts;
    uint32_t deadline_ms;
    acc_notify_fn notify;
    void *user;
} access_t;

/* Pure logic: no HAL, no globals, time is passed in. Wrap-safe for uint32_t ms. */
void        access_init(access_t *a, const char *pin, acc_notify_fn notify, void *user);
void        access_on_key(access_t *a, char key, uint32_t now_ms);
void        access_tick(access_t *a, uint32_t now_ms);
bool        access_set_pin(access_t *a, const char *pin);   /* 4..6 digits */
void        access_force_lock(access_t *a);

acc_state_t access_state(const access_t *a);
uint8_t     access_bad_attempts(const access_t *a);
uint32_t    access_lockout_remaining(const access_t *a, uint32_t now_ms);
const char *access_state_name(acc_state_t s);
const char *access_event_name(acc_event_t e);

#endif
