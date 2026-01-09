#include "http.h"
#include "str.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

struct url_parts {
    const char *host;
    size_t host_len;
    const char *path;
    size_t path_len;
    int is_https;
};

static int parse_url(const char *url, struct url_parts *out) {
    const char *p = url;

    memset(out, 0, sizeof *out);

    /* scheme */
    if (strncmp(p, "http://", 7) == 0) {
        out->is_https = 0;
        p += 7;
    } else if (strncmp(p, "https://", 8) == 0) {
        out->is_https = 1;
        p += 8;
    } else {
        return -1; /* unsupported / invalid */
    }

    /* host */
    const char *host_start = p;
    while (*p && *p != '/' && *p != '?')
        p++;

    out->host = host_start;
    out->host_len = p - host_start;

    /* path */
    if (*p) {
        out->path = p;
        out->path_len = strlen(p);
    } else {
        out->path = "/";
        out->path_len = 1;
    }

    return 0;
}

#define SCHEME_HTTP "http://"
#define SCHEME_HTTPS "https://"
#define PORT_HTTP "80"
#define PORT_HTTPS "443"
int http_get_host(
    const char *url,
    const char **hostname,
    size_t *hostname_len,
    const char **port,
    size_t *port_len)
{
    const char *p = url;
    int is_https = 0;

    /* scheme */
    if (strncmp(p, SCHEME_HTTP, STRLEN(SCHEME_HTTP)) == 0) {
        p += STRLEN(SCHEME_HTTP);
        is_https = 0;
    } else if (strncmp(p, SCHEME_HTTPS, STRLEN(SCHEME_HTTPS)) == 0) {
        p += STRLEN(SCHEME_HTTPS);
        is_https = 1;
    } else {
        return -1; /* unsupported scheme */
    }

    /* host */
    const char *host_start = p;
    while (*p && *p != '/' && *p != '?' && *p != ':')
        p++;

    if (p == host_start)
        return -1; /* empty host */

    if (hostname)
        *hostname = host_start;
    if (hostname_len)
        *hostname_len = (size_t)(p - host_start);

    /* optional port */
    if (*p == ':') {
        p++; /* skip ':' */
        const char *port_start = p;

        while (*p && *p != '/' && *p != '?')
            p++;

        if (p == port_start)
            return -1; /* ':' but no port */

        if (port)
            *port = port_start;
        if (port_len)
            *port_len = (size_t)(p - port_start);
    } else {
        /* default port based on scheme */
        if (port)
            *port = is_https ? PORT_HTTPS : PORT_HTTP;
        if (port_len)
            *port_len = is_https ? 3 : 2;
    }

    return 0;
}
#undef PORT_HTTP
#undef PORT_HTTPS
#undef SCHEME_HTTP
#undef SCHEME_HTTPS

#define HTTP_MAX_REQUEST (16 * 1024)
char *http_request(const char *method,
                   const char *url,
                   const char *headers,
                   size_t *request_len)
{
    struct url_parts u;

    if (parse_url(url, &u) != 0)
        return NULL;

    char *request = calloc(HTTP_MAX_REQUEST, 1);
    if (!request)
        return NULL;

    int len = snprintf(
        request,
        HTTP_MAX_REQUEST,
        "%s %.*s HTTP/1.1\r\n"
        "Host: %.*s\r\n"
        "User-Agent: vttp/1.0\r\n"
        "%s"
        "\r\n",
        method,
        (int)u.path_len, u.path,
        (int)u.host_len, u.host,
        headers
    );

    if (len <= 0 || len >= HTTP_MAX_REQUEST) {
        free(request);
        return NULL;
    }

    if (request_len)
        *request_len = (size_t)len;

    return request;
}
#undef HTTP_MAX_REQUEST

/**
 * Get the next CRLF count from current COUNT and last char TL. Max count up to 4.
 *
 * @returns COUNT + 1 if TL and COUNT match the next expected byte. Resets to 0 otherwise.
 * @retval -1 if count is not within 0 and 3 (inclusive).
 */
static int count_crlf(int count, char tl) {
    if (count < 0 || count > 3) {
        fprintf(stderr, "(http) COUNT out of range [0, 4) %d\n", count);
        return -1;
    }
    switch (count) {
    case 0:
        if (tl == '\r')
            return 1;
        break;
    case 1:
        if (tl == '\n')
            return 2;
        break;
    case 2:
        if (tl == '\r')
            return 3;
        break;
    case 3:
        if (tl == '\n')
            return 4;
        break;
    }
    return 0;
}

#define REQUIRE_NONNULL(ptr, fname)                         \
    do {                                                     \
        if ((ptr) == NULL) {                                \
            fprintf(stderr, "(%s) NULL argument: %s\n",     \
                    (fname), #ptr);                         \
            return false;                                   \
        }                                                    \
    } while (0)

static bool is_parse_headers_ptrs_ok(int *crlf_state, char *chunk,
                                    char *headers, size_t *headers_size,
                                    char *body, size_t *body_size,
                                    size_t *response_size)
{
    REQUIRE_NONNULL(crlf_state, "http_parse_headers");
    REQUIRE_NONNULL(chunk, "http_parse_headers");
    REQUIRE_NONNULL(headers, "http_parse_headers");
    REQUIRE_NONNULL(headers_size, "http_parse_headers");
    REQUIRE_NONNULL(body, "http_parse_headers");
    REQUIRE_NONNULL(body_size, "http_parse_headers");
    REQUIRE_NONNULL(response_size, "http_parse_headers");

    return true;
}

bool http_parse_headers(int *crlf_state, char *chunk, size_t chunk_size,
                       char *headers, size_t *headers_size,
                       char *body, size_t *body_size,
                       size_t *response_size)
{
    if (!is_parse_headers_ptrs_ok(crlf_state, chunk, 
                                  headers, headers_size, 
                                  body, body_size,
                                  response_size))
        return true;

    // Iterate over chunk
    for (ssize_t i = 0; i < chunk_size; i++) {
        char *curr_hd = chunk + i;
        char c = *curr_hd;
        headers[*headers_size] = c;
        *headers_size += 1;

        *crlf_state = count_crlf(*crlf_state, c);

        if (*crlf_state == 4) {
            *body_size = chunk_size - i;
            memcpy(body, curr_hd, *body_size);
            return true;
        }

        if (*headers_size == HTTP_MAX_HEADERS_SIZE) {
            if (i + 1 < chunk_size) {
                fprintf(stderr, "(http_parse_headers) Headers can't be larger than 4096 bytes\n");
            }
            return true;
        }
    }
    return false;
}
