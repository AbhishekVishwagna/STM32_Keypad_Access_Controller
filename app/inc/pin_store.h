#ifndef PIN_STORE_H
#define PIN_STORE_H

#include <stdbool.h>
#include <stddef.h>

/*
 * Persistence interface for the PIN.
 * Default implementation (pin_store_ram.c) keeps it in RAM only, so it resets on power cycle.
 * A flash-backed implementation can replace that file without touching anything else.
 */
bool pin_store_load(char *out, size_t out_size);   /* false if nothing stored */
bool pin_store_save(const char *pin);

#endif
