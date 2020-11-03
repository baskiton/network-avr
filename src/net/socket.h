/**
 * SOCKET: Transport Layer Interface (OSI Layer 4)
 */

#ifndef NET_SOCKET_H
#define NET_SOCKET_H

#include <stdint.h>
#include <stdio.h>
#include <stddef.h>

//#include "net/net.h"

/* Address Families */
#define AF_UNSPEC 0 // Unspecified
#define AF_INET 1   // internet IP protocol
#define AF_MAX 2

/* Protocol Families. Same as Address Families */
#define PF_UNSPEC AF_UNSPEC
#define PF_INET AF_INET

/* Socket States */
#define SS_FREE 0           // not allocated
#define SS_UNCONNECTED 1    // unconnected to any socket
#define SS_CONNECTING 2     // in process of connecting
#define SS_CONNECTED 3      // connected to socket
#define SS_DISCONNECTING 4  // in process of disconnecting

/* Socket Types */
#define SOCK_STREAM 1       // stream (connection) socket
#define SOCK_DGRAM 2        // datagram (conn.less) socket
#define SOCK_RAW 3          // raw socket
#define SOCK_SEQPACKET 4    // sequenced-packet socket
#define SOCK_MAX 5

/* Sutdown Flags */
#define SHUT_RD 0   // Disables further receive operations
#define SHUT_WR 1   // Disables further send operations
#define SHUT_RDWR 2 // Disables further send and receive operations

/* MSG Flags */
#define MSG_CTRUNC 1        // Control data truncated.
#define MSG_DONTROUTE 2     // Send without using routing tables.
#define MSG_EOR 4           // Terminates a record (if supported by the protocol).
#define MSG_OOB 8           // Out-of-band data.
#define MSG_NOSIGNAL 16     // No SIGPIPE generated when an attempt to send is made on a stream-oriented socket that is no longer connected.
#define MSG_PEEK 32         // Leave received data in queue.
#define MSG_TRUNC 64        // Normal data truncated.
#define MSG_WAITALL 128     // Attempt to fill the read buffer.

typedef uint8_t socklen_t;
typedef uint8_t sa_family_t;

#ifndef ssize_t
    typedef int16_t ssize_t;
#endif

struct sockaddr {
    sa_family_t sa_family;  // Address family
    uint8_t sa_data[6];     // Socket address
};

struct sockaddr_storage {
    sa_family_t ss_family;
};

struct socket {
    uint8_t state : 4;  // Socket State (e.g. SS_CONNECTED)
    uint8_t type : 4;   // Socket Type (e.g. SOCK_STREAM)
    struct protocol_ops *p_ops;
    FILE *file;
};

struct socket *accept(struct socket *restrict sk,
                      struct sockaddr *restrict addr,
                      socklen_t *restrict addr_len);
int8_t bind(struct socket *sk,
            const struct sockaddr *addr,
            socklen_t addr_len);
int8_t connect(struct socket *sk,
               const struct sockaddr *addr,
               socklen_t addr_len);
int8_t listen(struct socket *sk, uint8_t backlog);
ssize_t recv(struct socket *restrict sk,
             void *restrict buff,
             size_t buff_size,
             uint8_t flag);
ssize_t recvfrom(struct socket *restrict sk,
                 void *restrict buff,
                 size_t buff_size,
                 uint8_t flag,
                 struct sockaddr *restrict addr,
                 socklen_t *restrict addr_len);
ssize_t send(struct socket *sk, const void *buff,
             size_t buff_size, uint8_t flag);
ssize_t sendto(struct socket *sk, const void *buff, size_t buff_size,
               uint8_t flag, const struct sockaddr *addr, socklen_t addr_len);
int8_t shutdown(struct socket *sk, uint8_t flag);
struct socket *socket(struct socket *sk, uint8_t family,
                      uint8_t type, uint8_t protocol);

#endif  /* !NET_SOCKET_H */
