/**
 * @file iter8.h
 * @brief Iterable byte-sized buffer data structures
 */
#pragma once
#include <stddef.h>

/**
 * doubly linked list.
 */
typedef struct list8 {
    struct list8 *next;
    struct list8 *prev;
    char *buffer;
    size_t length;
} list8;

/**
 * Go to the next node in the list in NODE, if it exists.
 */
struct list8 *__list8_next(struct list8 *node);
struct list8 *__list8_new(struct list8 init);

/**
 * @brief Get the next value from ITER.
 *
 * You can also call the `__` prefixed implementation
 * functions directly, so this is really just for typing convenience.
 */
#define next8(iter) \
    _Generic((iter), \
        struct list8 *:  __list8_next \
    )(iter)

#define List8(...) __list8_new((struct list8){ __VA_ARGS__ })
