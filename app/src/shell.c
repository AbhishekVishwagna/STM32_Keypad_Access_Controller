#include "shell.h"
#include "event_log.h"
#include "pin_store.h"
#include <stdio.h>
#include <string.h>

static void say(shell_t *s, const char *str)
{
    s->write(str);
}

static void prompt(shell_t *s)
{
    say(s, "> ");
}

static void cmd_help(shell_t *s)
{
    say(s, "commands:\r\n"
           "  help           this list\r\n"
           "  status         state, failed attempts, lockout time left\r\n"
           "  log            recent access events\r\n"
           "  uptime         time since boot\r\n"
           "  lock           force the door back to locked\r\n"
           "  pin <digits>   set a new 4-6 digit PIN\r\n");
}

static void cmd_status(shell_t *s, uint32_t now)
{
    char out[96];

    snprintf(out, sizeof out, "state=%s bad_attempts=%u lockout_left_ms=%lu\r\n",
             access_state_name(access_state(s->acc)),
             (unsigned)access_bad_attempts(s->acc),
             (unsigned long)access_lockout_remaining(s->acc, now));
    say(s, out);
}

static void cmd_uptime(shell_t *s, uint32_t now)
{
    char out[48];

    snprintf(out, sizeof out, "uptime_ms=%lu\r\n", (unsigned long)now);
    say(s, out);
}

static void cmd_log(shell_t *s)
{
    char out[64];
    log_entry_t e;
    size_t n = event_log_count();

    if (n == 0u) {
        say(s, "(log empty)\r\n");
        return;
    }
    for (size_t i = 0; i < n; i++) {
        if (event_log_get(i, &e)) {
            snprintf(out, sizeof out, "%10lu ms  %s\r\n", (unsigned long)e.ms,
                     access_event_name((acc_event_t)e.code));
            say(s, out);
        }
    }
}

static void cmd_pin(shell_t *s, const char *arg)
{
    if (arg == NULL || *arg == '\0') {
        say(s, "usage: pin <4-6 digits>\r\n");
        return;
    }
    if (!access_set_pin(s->acc, arg)) {
        say(s, "error: PIN must be 4-6 digits\r\n");
        return;
    }
    if (pin_store_save(arg)) {
        say(s, "ok: PIN updated\r\n");
    } else {
        say(s, "warning: PIN updated for this session only (storage failed)\r\n");
    }
}

static void execute(shell_t *s, char *line, uint32_t now)
{
    char *arg = strchr(line, ' ');

    if (arg != NULL) {
        *arg++ = '\0';
        while (*arg == ' ') {
            arg++;
        }
    }

    if (strcmp(line, "help") == 0) {
        cmd_help(s);
    } else if (strcmp(line, "status") == 0) {
        cmd_status(s, now);
    } else if (strcmp(line, "log") == 0) {
        cmd_log(s);
    } else if (strcmp(line, "uptime") == 0) {
        cmd_uptime(s, now);
    } else if (strcmp(line, "lock") == 0) {
        access_force_lock(s->acc);
        say(s, "ok: locked\r\n");
    } else if (strcmp(line, "pin") == 0) {
        cmd_pin(s, arg);
    } else {
        say(s, "unknown command (try 'help')\r\n");
    }
}

void shell_init(shell_t *s, access_t *acc, shell_write_fn write)
{
    memset(s, 0, sizeof *s);
    s->acc = acc;
    s->write = write;
}

void shell_feed(shell_t *s, uint8_t b, uint32_t now)
{
    if (b == '\n' && s->last_was_cr != 0u) {   /* swallow the LF of a CR/LF pair */
        s->last_was_cr = 0;
        return;
    }
    s->last_was_cr = (b == '\r') ? 1u : 0u;

    if (b == '\r' || b == '\n') {
        say(s, "\r\n");
        if (s->len > 0u) {
            s->line[s->len] = '\0';
            execute(s, s->line, now);
            s->len = 0;
        }
        prompt(s);
    } else if (b == 0x08u || b == 0x7Fu) {      /* backspace / delete */
        if (s->len > 0u) {
            s->len--;
            say(s, "\b \b");
        }
    } else if (b >= 32u && b < 127u) {
        if (s->len < SHELL_LINE_MAX - 1u) {
            char echo[2];

            s->line[s->len++] = (char)b;
            echo[0] = (char)b;
            echo[1] = '\0';
            say(s, echo);
        }
    }
}
