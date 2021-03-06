#include <avr/pgmspace.h>

#include "netinet/tcp.h"
#include "netinet/udp.h"
#include "netinet/icmp.h"
#include "netinet/arp.h"
#include "netinet/ip.h"
#include "netinet/in.h"
#include "net/socket.h"
#include "net/net.h"
#include "net/nb_queue.h"

static uint16_t inet_port = 49152;

/*!
 * @brief Get free port number. Starting from 49152
 * @return Port number in network byte order
 */
uint16_t inet_get_port(void) {
    uint16_t port = inet_port++;

    if (!inet_port)
        inet_port = 49152;

    return htons(port);
}

/*!
 * TODO:
 */
static int8_t inet_release(struct socket *sk) {
    switch (sk->protocol) {
        case IPPROTO_TCP:
            /* code */
            break;
        case IPPROTO_UDP:
            /* code */
            break;
        case IPPROTO_ICMP:
            /* code */
            break;
        case IPPROTO_RAW:
            /* code */
            break;
        
        default:
            break;
    }

    return 0;
}

/*!
 * TODO:
 */
static int8_t inet_shutdown(struct socket *sk, uint8_t how) {
    if (how > 2)
        // EINVAL
        return -1;

    /** TODO: */

    sk->state = SS_UNCONNECTED;

    return 0;
}

/*!
 * TODO:
 */
static int8_t inet_bind(struct socket *sk,
                        const struct sockaddr *addr,
                        uint8_t addr_len) {
    struct sockaddr_in *addr_in;
    int8_t err = -1;    // EINVAL

    if (addr_len < sizeof(struct sockaddr_in))
        goto out;

    addr_in = (struct sockaddr_in *)addr;
    if (addr_in->sin_family != AF_INET) {
        // AF_UNSPEC can only be if s_addr is INADDR_ANY.
        if ((addr_in->sin_family != AF_UNSPEC) ||
            (addr_in->sin_addr.s_addr != htonl(INADDR_ANY))) {
            // EAFNOSUPPORT
            err = -1;
            goto out;
        }
    }

    if (sk->src_port)
        /** TODO: get port number automaticly */
        goto out;

    sk->src_addr = addr_in->sin_addr.s_addr;
    sk->dst_addr = 0;
    sk->src_port = htons(addr_in->sin_port);
    sk->dst_port = 0;
    err = 0;
out:
    return err;
}

/*!
 * @brief Sending message over Internet Protocol
 * @param sk Socket
 * @param msg Message structure
 */
static ssize_t inet_sendmsg(struct socket *restrict sk,
                            struct msghdr *restrict msg) {
    if (!sk->src_port)
        sk->src_port = inet_get_port();

    switch (sk->protocol) {
        case IPPROTO_TCP:
            return tcp_send_msg(sk, msg);
        case IPPROTO_UDP:
            return udp_send_msg(sk, msg);
        // case IPPROTO_ICMP:
        //     return 0;
        // case IPPROTO_RAW:
        //     return 0;
        
        default:
            // theoretically impossible, but still...
            return -1;
    }
}

/** TODO: */
static const struct protocol_ops inet_stream_ops PROGMEM = {
    .release = inet_release,
    .shutdown = inet_shutdown,
    .accept = NULL,
    .bind = inet_bind,
    .connect = NULL,
    .listen = NULL,
    .sendmsg = inet_sendmsg,
    .recvmsg = NULL,
};

/** TODO: */
static const struct protocol_ops inet_dgram_ops PROGMEM = {
    .release = inet_release,
    .shutdown = inet_shutdown,
    .accept = NULL,
    .bind = inet_bind,
    .connect = NULL,
    .listen = NULL,
    .sendmsg = inet_sendmsg,
    .recvmsg = NULL,
};

/*!
 * @brief Create INET socket
 * @param sk Pointer to Socket
 * @param protocol Inet Protocol (e.g. IP_PROTO_TCP). If 0 then
 *      sets by default
 * @return 0 on success
 */
int8_t inet_sock_create(struct socket *sk, uint8_t protocol) {
    int8_t err;

    /** if \p protocol == 0, set default value */
    switch (sk->type) {
        err = -1;   // EPROTONOSUPPORT
        case SOCK_STREAM:
            if (!protocol)
                protocol = IPPROTO_TCP;
            if (protocol == IPPROTO_TCP) {
                err = 0;
                sk->p_ops = &inet_stream_ops;
            }
            break;
        case SOCK_DGRAM:
            if (!protocol)
                protocol = IPPROTO_UDP;
            if ((protocol == IPPROTO_UDP) ||
                (protocol == IPPROTO_ICMP)) {
                err = 0;
                sk->p_ops = &inet_dgram_ops;
            }
            break;
        // case SOCK_RAW:
        //     protocol = IPPROTO_RAW;
        //     break;
        // case SOCK_SEQPACKET:
        //     protocol = IPPROTO_IP;
        //     break;
        default:
            if (!protocol)
                // EPROTOTYPE
                err = -1;
            break;
    }
    if (err)
        return err;

    sk->protocol = protocol;
    sk->state = SS_UNCONNECTED;

    nb_queue_init(&sk->nb_tx_q);
    nb_queue_init(&sk->nb_rx_q);

    return err;
}

/*!
 * @brief Initialize the handlers and others for IPv4
 */
void inet_init(void) {
    arp_init();
    ip_init();
}
