#include "debug.h"
#include "http.h"
#include "tcp.h"
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct fetch_init {
    const char *method;
    const char *headers;
    int body;

    struct {
        char status_text[HTTP_MAX_STATUS_TEXT_LENGTH + 1];
        size_t status_text_len;

        char *headers;
        size_t *headers_len;

        int body;
        size_t *body_len;
    } response;
} fetch_init;

int fetch(const char *url, struct fetch_init init) {
    size_t request_len = 0;
    char *request = http_request(init.method, url, init.headers, &request_len);

    const char *hostname = NULL;
    size_t hostname_len = 0;
    const char *port = NULL;
    size_t port_len = 0;

    if (http_get_host(url, &hostname, &hostname_len, &port, &port_len) != 0) {
        // HANDLEME
        return -1;
    }

    struct addrinfo *addrinfo = NULL;
    if (tcp_getaddrinfo(hostname, hostname_len, port, port_len, &addrinfo) != 0) {
        // HANDLEME
        return -1;
    }

    int sockfd = tcp_socket(addrinfo);
    if (sockfd < 0) {
        // HANDLEME
        return -1;
    }

    if (tcp_connect(
        sockfd,
        addrinfo->ai_addr,
        addrinfo->ai_addrlen,
        NULL,
        NULL,
        hostname) != 0)
    {
        // HANDLEME
        return -1;
    }

    if (tcp_send(sockfd, request, request_len, NULL) < 0) {
        // HANDLEME
        return -1;
    }

    char body[16 * 1024] = {0};
    size_t body_size = 0;
    char chunk[4096] = {0};
    size_t headers_size = 0;
    int crlf_state = 0;

    size_t response_len = 0;

    while (headers_size < 4096) {
        size_t remaining = 4096 - headers_size;
        ssize_t n = tcp_recv(sockfd, chunk, remaining, NULL);
        if (n <= 0)
            break; // EOF or error
        response_len += n;

        if (http_parse_headers(&crlf_state, chunk, n,
                               init.response.headers, init.response.headers_len,
                               body, &body_size,
                               &response_len))
            break;
    }

    return http_parse_status(init.response.headers, *init.response.headers_len,
                             init.response.status_text, init.response.status_text_len);
}

int main() {
    char response_headers[4096] = {0};
    size_t hdrs_len = 0;

    int status = fetch("http://jsonplaceholder.typicode.com/todos", (fetch_init) {
        .method = "GET",
        .headers =
            "Accept: */*\r\n"
            "Connection: close\r\n",
        .response = {
            .headers = response_headers,
            .headers_len = &hdrs_len,
        }
    });
}
