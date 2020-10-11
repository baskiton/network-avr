/**
 * SOCKET: Transport Layer Interface (OSI Layer 4)
 */

#ifndef SOCKET_H
#define SOCKET_H

#include <stdint.h>
#include <stdio.h>

#include "net.h"

/* Address Families */
#define AF_INET 2   // internet IP protocol

/* Protocol Families. Same as Address Families */
#define PF_INET AF_INET

struct sock_addr {
    uint8_t family; // address family
    uint16_t port;  // port number
    uint32_t addr;  // ipv4 address
};

struct soket {
    uint8_t state;
    uint8_t type;
    struct protocol_ops *p_ops;
    FILE *file;
};

struct soket *socket(struct soket *sk, uint8_t family, uint8_t type);
int8_t bind(struct soket *sk, const struct sock_addr *sk_addr);
// int8_t connect(struct soket *sk, const struct sock_addr *sk_addr);
int16_t send_to(struct soket *sk, const char *buff, size_t buff_size,
                struct sock_addr *sk_addr);
int16_t send(struct soket *sk, const char *buff, size_t buff_size);
int16_t recv_from(struct soket *sk, char *buff, size_t buff_size,
                  struct sock_addr *sk_addr);
int16_t recv(struct soket *sk, char *buff, size_t buff_size);
// int8_t listen(struct soket *sk);
int8_t shutdown(struct soket *sk, uint8_t flag);

#endif  /* !SOCKET_H */
