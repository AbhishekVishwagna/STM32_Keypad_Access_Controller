#ifndef RINGBUF_H
#define RINGBUF_H

#include <stdbool.h>
#include <stdint.h>

/*
 * Single-producer / single-consumer byte ring buffer.
 * Safe for one ISR writing (put) and the main loop reading (get) on a single core.
 * One slot is kept empty, so usable capacity is size - 1.
 */
typedef struct {
    uint8_t *buf;
    uint16_t size;
    volatile uint16_t head; /* written by producer */
    volatile uint16_t tail; /* written by consumer */
} ringbuf_t;

void     ringbuf_init(ringbuf_t *rb, uint8_t *storage, uint16_t size);
bool     ringbuf_put(ringbuf_t *rb, uint8_t byte);   /* false if full */
bool     ringbuf_get(ringbuf_t *rb, uint8_t *byte);  /* false if empty */
uint16_t ringbuf_count(const ringbuf_t *rb);

#endif
