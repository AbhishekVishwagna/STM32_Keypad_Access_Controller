#include "ringbuf.h"

void ringbuf_init(ringbuf_t *rb, uint8_t *storage, uint16_t size)
{
    rb->buf  = storage;
    rb->size = size;
    rb->head = 0;
    rb->tail = 0;
}

bool ringbuf_put(ringbuf_t *rb, uint8_t byte)
{
    uint16_t head = rb->head;
    uint16_t next = (uint16_t)((head + 1u) % rb->size);

    if (next == rb->tail) {
        return false;
    }
    rb->buf[head] = byte;
    rb->head = next;
    return true;
}

bool ringbuf_get(ringbuf_t *rb, uint8_t *byte)
{
    uint16_t tail = rb->tail;

    if (rb->head == tail) {
        return false;
    }
    *byte = rb->buf[tail];
    rb->tail = (uint16_t)((tail + 1u) % rb->size);
    return true;
}

uint16_t ringbuf_count(const ringbuf_t *rb)
{
    uint16_t head = rb->head;
    uint16_t tail = rb->tail;

    return (uint16_t)((head + rb->size - tail) % rb->size);
}
