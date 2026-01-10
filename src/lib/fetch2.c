#include "debug.h"
#include "http.h"
#include "tcp.h"
#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <ctype.h>

#define MAX_EVENTS 10

typedef struct request_init {
    const char *method;
    const char *headers;
    int body;
    struct {
        char status_text[HTTP_MAX_STATUS_TEXT_LENGTH + 1];
        size_t status_text_len;

        char *headers;
        size_t *headers_len;

        int *body;
    } response;
} fetch_init;

#define BODY_FD_LEN 3
int fetch(const char *url, struct request_init init) {
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
    size_t body_size = 0, headers_size = 0;
    char chunk[4096] = {0};
    int crlf_state = 0;

    size_t response_len = 0;

    char res_headers_own[4096] = {0};
    char *res_headers = init.response.headers 
        ? init.response.headers
        : res_headers_own;

    size_t res_headers_len_own = 0;
    size_t *res_headers_len = init.response.headers_len 
        ? init.response.headers_len
        : &res_headers_len_own;

    while (headers_size < 4096
           && *res_headers_len < 4
           && strncmp(res_headers, "\r\n\r\n", 4) != 0)
    {
        size_t remaining = 4096 - headers_size;
        ssize_t n = tcp_recv(sockfd, chunk, remaining, NULL);
        if (n <= 0)
            break; // EOF or error
        response_len += n;

        http_parse_headers(&crlf_state, chunk, n,
                           res_headers, res_headers_len,
                           body, &body_size,
                           &response_len);
    }

    if (pipe(init.response.body) == -1) {
        perror("pipe BODYFDS");
        return -1;
    }
    if (body_size > 0)
        write(init.response.body[1], body, body_size);

    int epollfd = epoll_create1(0);
    if (epollfd == -1) {
        perror("epoll_create1");
        return -1;
    }
    if (fcntl(sockfd, F_SETFL, 
              fcntl(sockfd, F_GETFL, 0) | O_NONBLOCK) < 0)
    {
        perror("fcntl");
        return -1;
    }
    if (fcntl(init.response.body[0], F_SETFL, 
              fcntl(init.response.body[0], F_GETFL, 0) | O_NONBLOCK) < 0)
    {
        perror("fcntl");
        return -1;
    }
    init.response.body[2] = sockfd;
    //
    struct epoll_event ev = {
        .events = EPOLLIN,
        .data.ptr = init.response.body
    };
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, sockfd, &ev) == -1) {
        perror("epoll_ctl: sockfd");
        return -1;
    }

    return epollfd;
}
#undef BODY_FD_LEN

#define MAX_CHUNK 4096
#define MAX_NEXT 5 * MAX_CHUNK
int pipe2(int readable, int writable,
           int f(const char *, size_t, char *))
{
    char chunk[4096] = {0};
    char out[4096] = {0};
    ssize_t n = read(readable, chunk, sizeof chunk);
    if (n < 0) {
        if (errno == EAGAIN) {}
    }
    if (n == 0) {
        return n;
    }

    if (f) {
        int rc = f(chunk, n, out);
        ssize_t off = 0;
        while (off < rc) {
            ssize_t w = write(writable, out + off, rc - off);
            if (w < 0) {
                perror("write");
                return w;
            }
            off += w;
        }
        return rc;
    } 

    ssize_t off = 0;
    while (off < n) {
        ssize_t w = write(writable, chunk + off, n - off);
        if (w < 0) {
            perror("write");
            return w;
        }
        off += w;
    }
    return off;
}
#undef MAX_NEXT
#undef MAX_CHUNK

int touppercase(const char *chunk, size_t n, char *next) {
    for (int i = 0; i < n; i++) {
        next[i] = toupper(chunk[i]);
    }
    return n;
}


int main() {
    int body[3] = {0};
    int epollfd = fetch("http://jsonplaceholder.typicode.com/todos", (fetch_init) {
        .method = "GET",
        .headers =
            "Connection: close\r\n",
        .response.body = body
    });
    struct epoll_event events[MAX_EVENTS] = {0};
    for (;;) {
        int nfds = epoll_wait(epollfd, events, 10, -1);
        if (nfds == -1) {
            perror("epoll_wait");
            return 1;
        }
        if (pipe2(body[2], body[1], touppercase) == 0) {
            break;
        }
    }

    int n = 0;
    char buf[4096] = {0};
    while ((n = read(body[0], buf, 4096)) > 0) {
        printf("%.*s", n, buf);
    }
    printf("final n = %d\n", n);
    return 0;
}
