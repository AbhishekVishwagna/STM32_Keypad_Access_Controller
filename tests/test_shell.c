#include "shell.h"
#include "event_log.h"
#include "test_util.h"
#include <string.h>

static char g_out[2048];

static void capture(const char *s)
{
    strncat(g_out, s, sizeof g_out - strlen(g_out) - 1u);
}

static void send(shell_t *sh, const char *text, uint32_t now)
{
    for (; *text; text++) {
        shell_feed(sh, (uint8_t)*text, now);
    }
}

static int has(const char *needle)
{
    return strstr(g_out, needle) != NULL;
}

int main(void)
{
    access_t a;
    shell_t sh;

    access_init(&a, "1234", NULL, NULL);
    shell_init(&sh, &a, capture);

    send(&sh, "help\r", 0);
    CHECK(has("pin <digits>"));

    g_out[0] = '\0';
    send(&sh, "status\r\n", 42);               /* CR/LF pair must run the command once */
    CHECK(has("state=LOCKED"));

    g_out[0] = '\0';
    send(&sh, "pin 55\r", 0);
    CHECK(has("error"));
    g_out[0] = '\0';
    send(&sh, "pin 5678\r", 0);
    CHECK(has("ok: PIN updated"));
    access_on_key(&a, '5', 0); access_on_key(&a, '6', 0);
    access_on_key(&a, '7', 0); access_on_key(&a, '8', 0);
    access_on_key(&a, '#', 0);
    CHECK(access_state(&a) == ACC_UNLOCKED);

    g_out[0] = '\0';
    send(&sh, "lock\r", 0);
    CHECK(has("ok: locked") && access_state(&a) == ACC_LOCKED);

    g_out[0] = '\0';
    event_log_clear();
    send(&sh, "log\r", 0);
    CHECK(has("(log empty)"));
    event_log_add(1234, ACC_EV_PIN_OK);
    g_out[0] = '\0';
    send(&sh, "log\r", 0);
    CHECK(has("PIN_OK") && has("1234 ms"));

    g_out[0] = '\0';
    send(&sh, "bogus\r", 0);
    CHECK(has("unknown command"));

    /* backspace edits the line */
    g_out[0] = '\0';
    send(&sh, "uptimX\x7f" "e\r", 99);
    CHECK(has("uptime_ms=99"));

    /* over-long input never overflows */
    g_out[0] = '\0';
    for (int i = 0; i < 200; i++) {
        shell_feed(&sh, 'a', 0);
    }
    shell_feed(&sh, '\r', 0);
    CHECK(has("unknown command"));

    TEST_DONE("test_shell");
}
