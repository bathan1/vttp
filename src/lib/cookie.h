/**
 * @file cookie.h
 * @brief In memory streams
 *
 * In memory stream that implements FIFO over a #deque
 */
#pragma once
#include <stdio.h>
#include "cfns.h"

struct cookie;

/**
 * Initialize a custom io stream with the provided callbacks in IO with nullable CTX.
 *
 * ...
 *
 * 내가 만든 쿠키
 */
FILE *cookie(const struct cookie *backend, void *ctx);

/**
 * The trivial cookie implementation that just forwards written bytes into a readable end.
 */
extern const struct cookie COOKIE_PASSTHROUGH;

/**
 * JSON object list stream. It separates elements by newline '\n', so it's basically NDJSON.
 */
extern const struct cookie COOKIE_JSON;

/**
 * `fwrite()` on N bytes of data from SRC buffer to DST stream.
 */
size_t fwrite8(const char *src, size_t n, FILE *dst);
