#ifndef TEST_UTIL_H
#define TEST_UTIL_H
#include <stdio.h>

static int g_failures;

#define CHECK(cond)                                                          \
    do {                                                                     \
        if (!(cond)) {                                                       \
            printf("  FAIL %s:%d  %s\n", __FILE__, __LINE__, #cond);         \
            g_failures++;                                                    \
        }                                                                    \
    } while (0)

#define TEST_DONE(name)                                                      \
    do {                                                                     \
        printf("%s: %s\n", (name), g_failures ? "FAILED" : "ok");            \
        return g_failures ? 1 : 0;                                           \
    } while (0)

#endif
