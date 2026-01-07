/**
 * @file pyc.h
 * @brief A python-inspired API for general C scripts.
 */
#pragma once
#include <stddef.h>
#include <stdbool.h>

/* STRING */

/**
 * @brief Buffer pointer + length
 */
typedef struct str {
    char *val;
    size_t length;
} str;

/**
 * Canonical empty string view. Safe to use everywhere.
 */
extern const struct str STR_EMPTY;

/**
 * @brief Offset string S buffer pointer by 1.
 */
struct str __str_next(struct str s);

/**
 * @brief **Read** S string length.
 * 
 * Length is defined as the "string" length, NOT the buffer size 
 * (which is +1 from the null terminator '\0').
 */
size_t __str_len(struct str s);

/**
 * @brief Get the pointer to the backing byte buffer in S.
 */
const char *__str_get(struct str s);

/**
 * @brief Cleanup *dynamically* allocated string S.
 */
bool __str_done(struct str s);

/**
 * @brief Appends char CH directly to S's buffer if IS_MUTABLE. Otherwise, return a *copy* of S 
 */
bool __str_insert(struct str s, char ch);

/* LIST */

/**
 * @brief Doubly linked list
 */
typedef struct list list;

/**
 * @brief Get the next node in the list in LS, if it exists.
 */
struct list *__list_next(struct list *ls);

/**
 * @brief Compute number of element in list LS.
 */
size_t __list_len(struct list *ls);

/**
 * @brief Read underlying of node LS.
 */
struct str __list_get(struct list *ls);

/**
 * @brief Cleanup *dynamically* allocated list LS.
 */
bool __list_done(struct list *ls);

/**
 * @brief Appends S to the *tail* end of LS *dynamically*.
 */
bool __list_insert(struct list *ls, struct str s);

/* QUEUE */

/**
 * @brief FIFO over char buffers.
 */
typedef struct queue queue;

/**
 * @brief Pops the front string of Q.
 */
struct str __queue_next(struct queue *q);

/**
 * @brief Read size of Q;
 */
size_t __queue_len(struct queue *q);

/**
 * @brief Peek the front string of Q.
 */
struct str __queue_get(struct queue *q);

/**
 * @brief Cleanup dynamically allocated Q.
 */
bool __queue_done(struct queue *q);

/**
 * @brief Enqueue S in Q.
 */
bool __queue_insert(struct queue *q, struct str s);

/**
 * @brief Get the empty value for **type** T.
 */
#define empty(T) \
    _Generic((T *)0, \
        struct str *: STR_EMPTY, \
        struct list **: NULL, \
        struct queue **: NULL \
    )

/**
 * @brief Return the next value of ITER. 
 */
#define next(iter) \
    _Generic((iter), \
        struct str: __str_next, \
        struct list *: __list_next, \
        struct queue *: __queue_next \
    )(iter)

/**
 * @brief Read length of ITER.
 */
#define len(iter) \
    _Generic((iter), \
        struct str: __str_len, \
        struct list *: __list_len, \
        struct queue *: __queue_len \
    )(iter)

/**
 * @brief Get the underyling "value" of ITER.
 */
#define get(iter) \
    _Generic((iter), \
        struct str: __str_get, \
        struct list *: __list_get, \
        struct queue *: __queue_get \
    )(iter)

/**
 * @brief Free dynamically allocated ITER.
 */
#define done(iter) \
    _Generic((iter), \
        struct str: __str_done, \
        struct list *: __list_done, \
        struct queue *: __queue_done \
    )(iter)

/**
 * @brief Return the next value of ITER. 
 */
#define insert(iter, value) \
    _Generic((iter), \
        struct str: __str_insert, \
        struct list *: __list_insert, \
        struct queue *: __queue_insert \
    )((iter), (value))
