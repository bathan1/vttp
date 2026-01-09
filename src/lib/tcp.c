#include "tcp.h"

#include <asm-generic/errno-base.h>
#include <netdb.h>
#include <openssl/err.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static void err_print() {
    unsigned long err = ERR_get_error();
    if (err != 0) {
        char err_buf[256] = {0};
        ERR_error_string_n(err, err_buf, sizeof(err_buf));
        fprintf(stderr, "%s\n", err_buf);
    }
}

static int tls_connect(int sockfd, SSL **ssl,
                       SSL_CTX **ctx, const char *hostname);

int tcp_connect(int fd, struct sockaddr *addr, socklen_t len,
                 SSL **ssl, SSL_CTX **ctx, const char *hostname)
{
    if (connect(fd, addr, len) < 0) {
        perror("connect()");
        return -1;
    }
    if (ssl != NULL && ctx != NULL && hostname != NULL) {
        if (tls_connect(fd, ssl, ctx, hostname) < 0) {
            perror("tls_connect()");
            return -1;
        } // else { ok! }
    }
    return 0;
}

ssize_t tcp_send(int fd, const char *bytes, size_t len, SSL *ssl) {
    if (fd < 0 && !ssl) {
        fprintf(
            stderr,
            "[tcp_send] FD can't be < 0 when SSL is NULL: %d\n",
            fd
        );
        return -1;
    }
    if (ssl)
        return SSL_write(ssl, bytes, len);
    else
        return send(fd, bytes, len, 0);
}

ssize_t tcp_recv(int sockfd, char *buf, size_t len, SSL *ssl) {
    if (sockfd < 0 && ssl == NULL) {
        fprintf(
            stderr,
            "[tcp_recv] SOCKFD can't be less than 0 when SSL is NULL: %d\n",
            sockfd
        );
        return -1;
    }

    if (ssl)
        return SSL_read(ssl, buf, len);
    else
        return recv(sockfd, buf, len, 0);
}

void tcp_tls_free(SSL *ssl, SSL_CTX *ctx) {
    if (ssl) {
        SSL_shutdown(ssl);
        SSL_free(ssl);
    }
    if (ctx)
        SSL_CTX_free(ctx);
}

#define MAX_HOSTNAME_LENGTH 255
#define MAX_PORT_LENGTH 15
int tcp_getaddrinfo(
    const char *hostname,
    size_t hostname_len,
    const char *port,
    size_t port_len,
    struct addrinfo **addr)
{
    if (!hostname || !port || !addr)
        return EINVAL;

    char hbuf[MAX_HOSTNAME_LENGTH + 1];
    char pbuf[MAX_PORT_LENGTH + 1];

    if (hostname_len >= sizeof(hbuf) || port_len >= sizeof(pbuf))
        return ENAMETOOLONG;

    memcpy(hbuf, hostname, hostname_len);
    hbuf[hostname_len] = '\0';

    memcpy(pbuf, port, port_len);
    pbuf[port_len] = '\0';

    struct addrinfo hints = {
        .ai_family   = AF_INET,
        .ai_socktype = SOCK_STREAM,
        .ai_flags    = AI_NUMERICSERV
    };

    return getaddrinfo(hbuf, pbuf, &hints, addr);
}
#undef MAX_HOSTNAME_LENGTH
#undef MAX_PORT_LENGTH 

int tcp_socket(struct addrinfo *addrinfo) {
    int sockfd = socket(
        addrinfo->ai_family,
        addrinfo->ai_socktype,
        addrinfo->ai_protocol
    );
    if (sockfd < 0) {
        fprintf(stderr, "couldn't open socket\n");
        return -1;
    }
    return sockfd;
}

static int tls_connect(int sockfd, SSL **ssl,
                       SSL_CTX **ctx, const char *hostname)
{
    SSL_load_error_strings();
    OpenSSL_add_ssl_algorithms();

    if (sockfd < 0 || !hostname || !ctx || !ssl) {
        return -1;
    }

    *ctx = SSL_CTX_new(TLS_client_method());
    if (!*ctx) {
        err_print();
        return -1;
    }

    *ssl = SSL_new(*ctx);
    if (!*ssl) {
        err_print();
        SSL_CTX_free(*ctx);
        return -1;
    }

    SSL_set_fd(*ssl, sockfd);
    SSL_set_tlsext_host_name(*ssl, hostname);
    int rc = SSL_connect(*ssl);

    if (rc <= 0) {
        err_print();
        SSL_free(*ssl);
        SSL_CTX_free(*ctx);
    }
    return 0;
}
