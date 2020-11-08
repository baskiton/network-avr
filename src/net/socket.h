/**
 * SOCKET: Transport Layer Interface (OSI Layer 4)
 */

#ifndef NET_SOCKET_H
#define NET_SOCKET_H

#include <stdint.h>
#include <stddef.h>

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
// #define SOCK_RAW 3          // raw socket
// #define SOCK_SEQPACKET 4    // sequenced-packet socket
#define SOCK_MAX 3

/* Sutdown Flags */
#define SHUT_RD 0   // Disables further receive operations
#define SHUT_WR 1   // Disables further send operations
#define SHUT_RDWR 2 // Disables further send and receive operations

/* MSG Flags */
#define MSG_CTRUNC 1        // Control data truncated
#define MSG_DONTROUTE 2     // Send without using routing tables
#define MSG_EOR 4           // Terminates a record (if supported by the protocol)
#define MSG_OOB 8           // Out-of-band data (only for stream socket type?)
#define MSG_NOSIGNAL 16     // No SIGPIPE generated when an attempt to send is made on a stream-oriented socket that is no longer connected
#define MSG_PEEK 32         // Leave received data in queue
#define MSG_TRUNC 64        // Normal data truncated
#define MSG_WAITALL 128     // Attempt to fill the read buffer

typedef uint8_t socklen_t;
typedef uint8_t sa_family_t;

#ifndef ssize_t
    typedef int16_t ssize_t;
#endif

#include "net/net.h"
#include "net/uio.h"
#include "netinet/in.h"
#include "net/nb_queue.h"

struct sockaddr {
    sa_family_t sa_family;  // Address family
    uint8_t sa_data[6];     // Socket address
};

struct sockaddr_storage {
    sa_family_t ss_family;
};

struct msghdr {
    void *msg_name;         // ptr to socket address structure
    socklen_t msg_namelen;  // size of socket address structure
    struct iovec *msg_iov;  // data
    uint8_t msg_flags;      // flags on received message
};

struct socket {
    uint8_t state : 4;  // Socket State (e.g. SS_CONNECTED)
    uint8_t type : 4;   // Socket Type (e.g. SOCK_STREAM)
    const struct protocol_ops *p_ops;

    union {
        uint64_t addr_pair;
        struct {
            in_addr_t dst_addr; // destination address
            in_addr_t src_addr; // source address
        };
    };
    union {
        uint32_t port_pair;
        struct {
            in_port_t dst_port; // destination port
            in_port_t src_port; // source port
        };
    };
    uint32_t sk_hash;   // port-addr hash used for lookup

    uint8_t protocol;

    struct nb_queue_s nb_tx_q;  // transmit queue
    struct nb_queue_s nb_rx_q;  // receive queue

    struct socket *prev,    // prev socket in list
                  *next;    // next socket in list
};

void socket_list_init(void);

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
             uint8_t flags);
ssize_t recvfrom(struct socket *restrict sk,
                 void *restrict buff,
                 size_t buff_size,
                 uint8_t flags,
                 struct sockaddr *restrict addr,
                 socklen_t *restrict addr_len);
ssize_t send(struct socket *sk, const void *buff,
             size_t buff_size, uint8_t flags);
ssize_t sendto(struct socket *sk,
               const void *buff,
               size_t buff_size,
               uint8_t flags,
               const struct sockaddr *addr,
               socklen_t addr_len);
ssize_t sendmsg(struct socket *sk,
                const struct msghdr *message,
                uint8_t flags);
int8_t shutdown(struct socket *sk, uint8_t how);
struct socket *socket(uint8_t family, uint8_t type, uint8_t protocol);
void sock_close(struct socket **sk);

#endif  /* !NET_SOCKET_H */
