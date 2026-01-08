#pragma once
#include <stdio.h>

/**
 * @brief Write to VA_ARGS to stderr and evaluates to RC
 */
#define printf_err(__rc, ...) \
    (fprintf(stderr, __VA_ARGS__), (__rc))

/**
 * @brief Print out of memory to stderr and evaluates to RC
 */
#define enomem(__rc) \
    (printf_err(__rc, "out of memory"))

