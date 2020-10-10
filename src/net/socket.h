/**
 * SOCKET: Transport Layer Interface (OSI Layer 4)
*/

#ifndef SOCKET_H
#define SOCKET_H

#include <stdint.h>
#include <stdio.h>

#include "net/net.h"

/* Address Families */
#define AF_INET 2   // internet IP protocol

/* Protocol Families. Same as Address Families */
#define PF_INET AF_INET

// #define send(sk, buff, buff_size) (send_to((sk), (buff), (buff_size), NULL))
// #define recv(sk, buff, buff_size) (recv_from((sk), (buff), (buff_size), NULL))

struct sock_addr {
    uint8_t family; // address family
    uint16_t port;  // port number
    uint32_t addr;  // ipv4 address
};

typedef struct soket {
    uint8_t state;
    uint8_t type;
    struct protocol_ops *p_ops;
    FILE *file;
} sk_t;

sk_t *socket(sk_t *sk, uint8_t family, uint8_t type);
int8_t bind(sk_t *sk, const struct sock_addr *sk_addr);
// int8_t connect(sk_t *sk, const struct sock_addr *sk_addr);
int16_t send_to(sk_t *sk, const char *buff, size_t buff_size,
                struct sock_addr *sk_addr);
int16_t send(sk_t *sk, const char *buff, size_t buff_size) {
    return send_to(sk, buff, buff_size, NULL);
}
int16_t recv_from(sk_t *sk, char *buff, size_t buff_size,
                  struct sock_addr *sk_addr);
int16_t recv(sk_t *sk, char *buff, size_t buff_size) {
    return recv_from(sk, buff, buff_size, NULL);
}
// int8_t listen(sk_t *sk);
int8_t shutdown(sk_t *sk, uint8_t flag);

#endif  /* !SOCKET_H */
