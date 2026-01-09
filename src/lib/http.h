#pragma once
#include <stddef.h>
#include <stdbool.h>

/**
 * "Unavailable For Legal Reasons"
 */
#define HTTP_MAX_STATUS_TEXT_LENGTH 29
#define HTTP_MAX_HEADERS_SIZE 4096

/**
 * Create HTTP Request for METHOD request at URL and write out the buffer string length to REQUEST_LEN
 */
char *http_request(const char *method,
                   const char *url,
                   const char *headers,
                   size_t *request_len);

/**
 * Write hostname and port *positions in URL* HOSTNAME and PORT.
 */
int http_get_host(
    const char *url,
    const char **hostname,
    size_t *hostname_len,
    const char **port,
    size_t *port_len);

bool http_parse_headers(int *crlf_state, char *chunk, size_t chunk_size,
                       char *headers, size_t *headers_size,
                       char *body, size_t *body_size,
                       size_t *response_size);

int http_parse_status(const char *headers, size_t headers_len, 
                             char *status_text, size_t status_text_len);
