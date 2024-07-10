#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/ip.h>


static void msg(const char *msg) {
    fprintf(stderr, "%s\n", msg);
}

static void die(const char *msg) {
    int err = errno;
    fprintf(stderr, "[%d] %s\n", err, msg);
    abort();
}

/*
Simple dummy processing
*/
static void do_something(int connfd) {
    char rbuf[64] = {};
    ssize_t n = read(connfd, rbuf, sizeof(rbuf) - 1);
    if (n < 0) {
        msg("read() error");
        return;
    }
    printf("client says: %s\n", rbuf);

    char wbuf[] = "world";
    write(connfd, wbuf, strlen(wbuf));
}

int main() {
    // Create a new socket
    int fd = socket(AF_INET, SOCK_STREAM, 0); // AF_INET is for IPv4; SOCK_STREAM is for TCP

    // Abort if invalid socket
    if (fd < 0) {
        die("socket()");
    }

    /*
    Configure the socket
    fd: the socket
    SOL_SOCKET, SO_REUSUEADR: specifying which options to set
    val: the value with which to set the option
    sizeof: the option value is arbitrary size, so we need to set the length

    Setting SO_REUSEADDR to 1 is necessary for every listening socket; without it, bind() will not work
    */
    int val = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));

    // Binding the socket to an IPv4 address and port 0.0.0.0:1234.
    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = ntohs(1234);
    addr.sin_addr.s_addr = ntohl(0);    // wildcard address 0.0.0.0
    int rv = bind(fd, (const sockaddr *)&addr, sizeof(addr));
    if (rv) {
        die("bind()");
    }

    // Listen
    rv = listen(fd, SOMAXCONN); // SOMAXCONN is the backlog argument that determines the size of the queue for system calls
    if (rv) {
        die("listen()");
    }

    while (true) {
        // Accept connections
        struct sockaddr_in client_addr = {};
        socklen_t socklen = sizeof(client_addr);
        int connfd = accept(fd, (struct sockaddr *)&client_addr, &socklen);
        if (connfd < 0) {
            continue;   // error
        }

        do_something(connfd);
        close(connfd);
    }

    return 0;
}



/*

struct sockaddr {
    unsigned short   sa_family;     // AF_INET, AF_INET6
    char             sa_data[14];   // useless
};

struct sockaddr_in {
    short           sin_family;     // AF_INET
    unsigned short  sin_port;       // port number, big endian
    struct in_addr  sin_addr;       // IPv4 address
    char            sin_zero[8];    // useless
};

struct sockaddr_in6 {
    uint16_t        sin6_family;    // AF_INET6
    uint16_t        sin6_port;      // port number, big endian
    uint32_t        sin6_flowinfo;
    struct in6_addr sin6_addr;      // IPv6 address
    uint32_t        sin6_scope_id;
};

struct sockaddr_storage {
    sa_family_t     ss_family;      // AF_INET, AF_INET6
    // enough space for both IPv4 and IPv6
    char    __ss_pad1[_SS_PAD1SIZE];
    int64_t __ss_align;
    char    __ss_pad2[_SS_PAD2SIZE];
};

*/
