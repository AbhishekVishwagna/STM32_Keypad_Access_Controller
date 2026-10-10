#include "ringbuf.h"
#include "test_util.h"

int main(void)
{
    uint8_t storage[4];
    ringbuf_t rb;
    uint8_t v;

    ringbuf_init(&rb, storage, sizeof storage);
    CHECK(ringbuf_count(&rb) == 0);
    CHECK(!ringbuf_get(&rb, &v));

    /* capacity is size - 1 */
    CHECK(ringbuf_put(&rb, 1));
    CHECK(ringbuf_put(&rb, 2));
    CHECK(ringbuf_put(&rb, 3));
    CHECK(!ringbuf_put(&rb, 4));
    CHECK(ringbuf_count(&rb) == 3);

    CHECK(ringbuf_get(&rb, &v) && v == 1);
    CHECK(ringbuf_get(&rb, &v) && v == 2);

    /* wrap around */
    CHECK(ringbuf_put(&rb, 5));
    CHECK(ringbuf_put(&rb, 6));
    CHECK(ringbuf_get(&rb, &v) && v == 3);
    CHECK(ringbuf_get(&rb, &v) && v == 5);
    CHECK(ringbuf_get(&rb, &v) && v == 6);
    CHECK(!ringbuf_get(&rb, &v));
    CHECK(ringbuf_count(&rb) == 0);

    TEST_DONE("test_ringbuf");
}
