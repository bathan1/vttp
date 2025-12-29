/**
 * @file stream.h
 * @brief In memory streams
 *
 * In memory stream that implements FIFO over a #deque
 */
#pragma once
#include <stdio.h>
#include "deque.h"

/**
 * @brief Returns a readable FILE handle bound to DEQUE.
 */
FILE *stream_readable(struct deque8 *deque);

/**
 * @brief Returns a writable FILE handle bound to DEQUE.
 */
FILE *stream_writable(struct deque8 *deque);
