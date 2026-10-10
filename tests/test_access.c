#include "access.h"
#include "test_util.h"
#include <string.h>

static acc_event_t g_events[64];
static int g_n;

static void on_event(acc_event_t ev, void *user)
{
    (void)user;
    if (g_n < 64) {
        g_events[g_n++] = ev;
    }
}

static int count_ev(acc_event_t ev)
{
    int n = 0;
    for (int i = 0; i < g_n; i++) {
        if (g_events[i] == ev) {
            n++;
        }
    }
    return n;
}

static void type(access_t *a, const char *keys, uint32_t now)
{
    for (; *keys; keys++) {
        access_on_key(a, *keys, now);
    }
}

int main(void)
{
    access_t a;
    uint32_t t = 1000;

    access_init(&a, "1234", on_event, NULL);
    CHECK(access_state(&a) == ACC_LOCKED);

    /* correct PIN unlocks, then auto-relocks */
    type(&a, "1234#", t);
    CHECK(access_state(&a) == ACC_UNLOCKED);
    CHECK(count_ev(ACC_EV_PIN_OK) == 1);
    access_tick(&a, t + UNLOCK_HOLD_MS - 1);
    CHECK(access_state(&a) == ACC_UNLOCKED);
    access_tick(&a, t + UNLOCK_HOLD_MS);
    CHECK(access_state(&a) == ACC_LOCKED);
    CHECK(count_ev(ACC_EV_RELOCKED) == 1);

    /* '#' relocks early */
    type(&a, "1234#", t);
    type(&a, "#", t + 10);
    CHECK(access_state(&a) == ACC_LOCKED);

    /* wrong PIN, prefix of the PIN and longer PIN are all rejected */
    g_n = 0;
    type(&a, "1235#", t);
    CHECK(access_state(&a) == ACC_LOCKED && access_bad_attempts(&a) == 1);
    type(&a, "123#", t);
    CHECK(access_bad_attempts(&a) == 2);
    CHECK(count_ev(ACC_EV_PIN_BAD) == 2);

    /* third failure -> lockout; keys ignored; correct PIN does not help */
    type(&a, "12345#", t);
    CHECK(access_state(&a) == ACC_LOCKOUT);
    CHECK(count_ev(ACC_EV_LOCKOUT_START) == 1);
    type(&a, "1234#", t + 1);
    CHECK(access_state(&a) == ACC_LOCKOUT);
    CHECK(access_lockout_remaining(&a, t + 1000) == LOCKOUT_MS - 1000);

    /* lockout ends, counter resets */
    access_tick(&a, t + LOCKOUT_MS);
    CHECK(access_state(&a) == ACC_LOCKED && access_bad_attempts(&a) == 0);
    CHECK(count_ev(ACC_EV_LOCKOUT_END) == 1);
    CHECK(access_lockout_remaining(&a, t + LOCKOUT_MS) == 0);

    /* '*' clears the entry */
    g_n = 0;
    type(&a, "12*34#", t);              /* after '*', only "34" is entered -> wrong */
    CHECK(count_ev(ACC_EV_CLEARED) == 1);
    CHECK(count_ev(ACC_EV_PIN_BAD) == 1);

    /* entry timeout */
    g_n = 0;
    type(&a, "12", t);
    CHECK(access_state(&a) == ACC_ENTERING);
    access_tick(&a, t + ENTRY_TIMEOUT_MS);
    CHECK(access_state(&a) == ACC_LOCKED && count_ev(ACC_EV_TIMEOUT) == 1);

    /* entry longer than PIN_MAX_LEN is rejected (not silently truncated) and never overflows */
    access_init(&a, "123456", on_event, NULL);
    type(&a, "12345678#", t);
    CHECK(access_state(&a) == ACC_LOCKED && access_bad_attempts(&a) == 1);
    type(&a, "123456#", t);
    CHECK(access_state(&a) == ACC_UNLOCKED);

    /* set_pin validation */
    access_init(&a, "1234", on_event, NULL);
    CHECK(!access_set_pin(&a, "12"));
    CHECK(!access_set_pin(&a, "1234567"));
    CHECK(!access_set_pin(&a, "12a4"));
    CHECK(!access_set_pin(&a, NULL));
    CHECK(access_set_pin(&a, "9876"));
    type(&a, "1234#", t);
    CHECK(access_state(&a) == ACC_LOCKED);
    type(&a, "9876#", t);
    CHECK(access_state(&a) == ACC_UNLOCKED);

    /* invalid initial PIN falls back to the default */
    access_init(&a, "abc", on_event, NULL);
    type(&a, PIN_DEFAULT "#", t);
    CHECK(access_state(&a) == ACC_UNLOCKED);

    /* timers survive uint32_t wraparound */
    access_init(&a, "1234", on_event, NULL);
    t = 0xFFFFFFFFu - 1000u;
    type(&a, "1234#", t);
    CHECK(access_state(&a) == ACC_UNLOCKED);
    access_tick(&a, t + 1000u);            /* still before the deadline */
    CHECK(access_state(&a) == ACC_UNLOCKED);
    access_tick(&a, t + UNLOCK_HOLD_MS);   /* wrapped past zero */
    CHECK(access_state(&a) == ACC_LOCKED);

    /* force lock */
    type(&a, "1234#", 10);
    access_force_lock(&a);
    CHECK(access_state(&a) == ACC_LOCKED);

    TEST_DONE("test_access");
}
