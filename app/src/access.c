#include "access.h"
#include <string.h>

static void notify(access_t *a, acc_event_t ev)
{
    if (a->notify != NULL) {
        a->notify(ev, a->user);
    }
}

static bool time_reached(uint32_t now, uint32_t deadline)
{
    return (int32_t)(now - deadline) >= 0;   /* correct across uint32_t wraparound */
}

static bool is_digit(char c)
{
    return c >= '0' && c <= '9';
}

static void clear_entry(access_t *a)
{
    memset(a->entry, 0, sizeof a->entry);
    a->entry_len = 0;
    a->overflow = 0;
}

static bool valid_pin(const char *p)
{
    size_t n = strlen(p);

    if (n < PIN_MIN_LEN || n > PIN_MAX_LEN) {
        return false;
    }
    for (size_t i = 0; i < n; i++) {
        if (!is_digit(p[i])) {
            return false;
        }
    }
    return true;
}

static void store_pin(access_t *a, const char *pin)
{
    memset(a->pin, 0, sizeof a->pin);
    memcpy(a->pin, pin, strlen(pin));
}

/* Compare without early exit so timing does not leak how many digits matched. */
static bool entry_matches(const access_t *a)
{
    uint8_t diff = (uint8_t)(a->entry_len ^ (uint8_t)strlen(a->pin)) | a->overflow;

    for (size_t i = 0; i < PIN_MAX_LEN; i++) {
        diff |= (uint8_t)(a->entry[i] ^ a->pin[i]);
    }
    return diff == 0u;
}

void access_init(access_t *a, const char *pin, acc_notify_fn cb, void *user)
{
    memset(a, 0, sizeof *a);
    a->notify = cb;
    a->user = user;
    a->state = ACC_LOCKED;
    store_pin(a, (pin != NULL && valid_pin(pin)) ? pin : PIN_DEFAULT);
}

bool access_set_pin(access_t *a, const char *pin)
{
    if (pin == NULL || !valid_pin(pin)) {
        return false;
    }
    store_pin(a, pin);
    notify(a, ACC_EV_PIN_CHANGED);
    return true;
}

static void submit(access_t *a, uint32_t now)
{
    bool ok = entry_matches(a);

    clear_entry(a);

    if (ok) {
        a->bad_attempts = 0;
        a->state = ACC_UNLOCKED;
        a->deadline_ms = now + UNLOCK_HOLD_MS;
        notify(a, ACC_EV_PIN_OK);
        return;
    }

    a->bad_attempts++;
    if (a->bad_attempts >= MAX_BAD_ATTEMPTS) {
        a->state = ACC_LOCKOUT;
        a->deadline_ms = now + LOCKOUT_MS;
        notify(a, ACC_EV_PIN_BAD);
        notify(a, ACC_EV_LOCKOUT_START);
    } else {
        a->state = ACC_LOCKED;
        notify(a, ACC_EV_PIN_BAD);
    }
}

void access_on_key(access_t *a, char key, uint32_t now)
{
    if (a->state == ACC_LOCKOUT) {
        return;                              /* keys are ignored while locked out */
    }

    if (a->state == ACC_UNLOCKED) {
        if (key == '#') {                    /* relock early */
            a->state = ACC_LOCKED;
            notify(a, ACC_EV_RELOCKED);
        }
        return;
    }

    if (is_digit(key)) {
        if (a->entry_len >= PIN_MAX_LEN) {
            a->overflow = 1u;                /* over-long entry is rejected on submit, not truncated */
            a->deadline_ms = now + ENTRY_TIMEOUT_MS;
            return;
        }
        a->entry[a->entry_len++] = key;
        a->state = ACC_ENTERING;
        a->deadline_ms = now + ENTRY_TIMEOUT_MS;
        notify(a, ACC_EV_KEY);
    } else if (key == '*') {
        if (a->entry_len == 0u) {
            return;
        }
        clear_entry(a);
        a->state = ACC_LOCKED;
        notify(a, ACC_EV_CLEARED);
    } else if (key == '#') {
        if (a->entry_len == 0u) {
            return;
        }
        submit(a, now);
    }
}

void access_tick(access_t *a, uint32_t now)
{
    switch (a->state) {
    case ACC_UNLOCKED:
        if (time_reached(now, a->deadline_ms)) {
            a->state = ACC_LOCKED;
            notify(a, ACC_EV_RELOCKED);
        }
        break;
    case ACC_ENTERING:
        if (time_reached(now, a->deadline_ms)) {
            clear_entry(a);
            a->state = ACC_LOCKED;
            notify(a, ACC_EV_TIMEOUT);
        }
        break;
    case ACC_LOCKOUT:
        if (time_reached(now, a->deadline_ms)) {
            a->bad_attempts = 0;
            a->state = ACC_LOCKED;
            notify(a, ACC_EV_LOCKOUT_END);
        }
        break;
    case ACC_LOCKED:
    default:
        break;
    }
}

void access_force_lock(access_t *a)
{
    clear_entry(a);
    if (a->state == ACC_UNLOCKED) {
        a->state = ACC_LOCKED;
        notify(a, ACC_EV_RELOCKED);
    } else if (a->state == ACC_ENTERING) {
        a->state = ACC_LOCKED;
    }
}

acc_state_t access_state(const access_t *a)
{
    return a->state;
}

uint8_t access_bad_attempts(const access_t *a)
{
    return a->bad_attempts;
}

uint32_t access_lockout_remaining(const access_t *a, uint32_t now)
{
    if (a->state != ACC_LOCKOUT || time_reached(now, a->deadline_ms)) {
        return 0;
    }
    return a->deadline_ms - now;
}

const char *access_state_name(acc_state_t s)
{
    static const char *const names[] = {"LOCKED", "ENTERING", "UNLOCKED", "LOCKOUT"};

    return ((unsigned)s < 4u) ? names[s] : "?";
}

const char *access_event_name(acc_event_t e)
{
    static const char *const names[ACC_EV_COUNT] = {
        "KEY", "CLEARED", "TIMEOUT", "PIN_OK", "PIN_BAD",
        "LOCKOUT_START", "LOCKOUT_END", "RELOCKED", "PIN_CHANGED"
    };

    return ((unsigned)e < (unsigned)ACC_EV_COUNT) ? names[e] : "?";
}
