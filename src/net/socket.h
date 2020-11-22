/**
 * SOCKET: Transport Layer Interface (OSI Layer 4)
 */

#ifndef NET_SOCKET_H
#define NET_SOCKET_H

#include <stdint.h>
#include <stddef.h>
#include <limits.h>

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
#define MSG_PEEK 16         // Leave received data in queue
#define MSG_TRUNC 32        // Normal data truncated
#define MSG_WAITALL 64      // Attempt to fill the read buffer
#define MSG_DONTWAIT 128    // Noblocking io

/* Socket settings */
#define SOL_SOCKET 1    // Options to be accessed at socket level, not protocol level.

#define SO_RCVTIMEO 11  // (struct timeval) Receive timeout
/** TODO: not support at this time
#define SO_ACCEPTCONN 1 // (int) Socket is accepting connections (getsockopt() only)
#define SO_BROADCAST 2  // (int) Transmission of broadcast messages is supported (SOCK_DGRAM sockets only)
#define SO_DEBUG 3      // (int) Debugging information is being recorded
#define SO_DONTROUTE 4  // (int) Bypass normal routing
#define SO_ERROR 5      // (int) Socket error status (getsockopt() only)
#define SO_KEEPALIVE 6  // (int) Connections are kept alive with periodic messages (protocol-specific)
#define SO_LINGER 7     // (struct linger) Socket lingers on close
#define SO_OOBINLINE 8  // (int) Out-of-band data is transmitted in line
#define SO_RCVBUF 9     // (int) Receive buffer size (in bytes)
#define SO_RCVLOWAT 10  // (int) Receive "low water mark" (in bytes)
#define SO_REUSEADDR 12 // (int) Reuse of local addresses is supported (protocol-specific)
#define SO_SNDBUF 13    // (int) Send buffer size (in bytes)
#define SO_SNDLOWAT 14  // (int) Send "low water mark" (in bytes)
#define SO_SNDTIMEO 15  // (struct timeval) Send timeout
#define SO_TYPE 16      // (int) Socket type (getsockopt() only)
// */

#define MAX_TIMEOUT INT32_MAX

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
    char data[6];
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

    /* socket options */
    int32_t rcv_timeout;   // Receive Timeout setting SO_RCVTIMEO
    int32_t tx_timeout;

    struct nb_queue_s nb_tx_q;  // transmit queue
    struct nb_queue_s nb_rx_q;  // receive queue

    struct socket *prev,    // prev socket in list
                  *next;    // next socket in list
};

struct sock_ap_pairs_s {
    in_addr_t loc_addr; // local address
    in_addr_t fe_addr;  // foreign address
    in_port_t loc_port;  // local port
    in_port_t fe_port;  // foreign port
};

void socket_list_init(void);

void socket_set_hash(struct socket *sk);
struct socket *socket_find(struct sock_ap_pairs_s *pairs);

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
int8_t setsockopt(struct socket *sk, uint8_t level, uint8_t option_name,
                  const void *option_value, socklen_t option_len);
int8_t shutdown(struct socket *sk, uint8_t how);
struct socket *socket(uint8_t family, uint8_t type, uint8_t protocol);
void sock_close(struct socket **sk);

#endif  /* !NET_SOCKET_H */
