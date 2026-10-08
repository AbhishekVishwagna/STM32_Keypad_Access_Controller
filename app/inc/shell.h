#ifndef SHELL_H
#define SHELL_H

#include <stdint.h>
#include "access.h"

#define SHELL_LINE_MAX 40u

typedef void (*shell_write_fn)(const char *str);

typedef struct {
    char line[SHELL_LINE_MAX];
    uint8_t len;
    uint8_t last_was_cr;
    access_t *acc;
    shell_write_fn write;
} shell_t;

void shell_init(shell_t *s, access_t *acc, shell_write_fn write);
void shell_feed(shell_t *s, uint8_t byte, uint32_t now_ms);   /* one received byte */

#endif
