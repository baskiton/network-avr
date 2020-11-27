#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "net/net.h"
#include "net/socket.h"
#include "net/checksum.h"
#include "netinet/in.h"
#include "netinet/icmp.h"
#include "netinet/ip.h"

/*!
 * @brief PING sending message
 * @param sk Socket
 * @param msg Message header, contains data to send
 * @return Number of sending bytes
 */
ssize_t ping_send_msg(struct socket *restrict sk,
                      struct msghdr *restrict msg) {
    struct icmp_hdr_s *icmph = msg->msg_iov->iov_base;
    struct sockaddr_in *addr_in = msg->msg_name;
    struct sockaddr_in daddr;
    struct net_buff_s *nb;
    size_t len = msg->msg_iov->iov_len;
    int8_t err;

    /* data length must not be less than ICMP header */
    if (len < sizeof(*icmph))
        // EINVAL
        return -1;

    /* not support out-of-band data */
    if (msg->msg_flags & MSG_OOB)
        // EOPNOTSUPP
        return -1;

    if (icmph->type != ICMP_ECHO_REQ ||
        icmph->code != 0)
        // EINVAL
        return -1;

    daddr.sin_family = AF_INET;

    /* verify address */
    if (addr_in) {
        if (msg->msg_namelen < sizeof(*addr_in))
            // EINVAL
            return -1;
        if ((addr_in->sin_family != AF_INET) &&
            (addr_in->sin_family != AF_UNSPEC)) {
            // EAFNOSUPPORT
            return -1;
        }

        daddr.sin_addr.s_addr = addr_in->sin_addr.s_addr;
        daddr.sin_port = addr_in->sin_port = 0;
    } else {
        /* the socket is assumed to have already been established,
         * so the existing values are used.
         */
        daddr.sin_addr.s_addr = sk->dst_addr;
        daddr.sin_port = sk->dst_port;
    }

    icmph->hdr_data.echo.id = sk->src_port;
    icmph->chks = 0;
    icmph->chks = in_checksum(icmph, len);

    nb = ip_create_nb(sk, msg, 0, len, &daddr);
    if (!nb)
        // error
        return -1;

    err = ip_send_sock(sk);
    if (err) {
        if (err > 0)
            err = -err;
        return err;
    }

    return len;
}

/*!
 * @brief PING receiving message
 * @param sk Socket
 * @param msg Message structure to store
 * @param flags Flags (MSG_PEEK, MSG_DONTWAIT)
 * @param addr_len Address length to store
 * @return Size of receiving message in bytes
 */
ssize_t ping_recv_msg(struct socket *restrict sk,
                      struct msghdr *restrict msg,
                      int8_t flags, socklen_t *restrict addr_len) {
    struct net_buff_s *nb;
    size_t max_len = msg->msg_iov->iov_len;
    size_t len;
    struct sockaddr_in *addr = msg->msg_name;
    void *from;

    /* not support out-of-band data */
    if (msg->msg_flags & MSG_OOB)
        // EOPNOTSUPP
        return -1;

    nb = net_buff_rcv(sk, flags);
    if (!nb)
        return -1;

    from = get_ip_hdr(nb);
    len = nb->end - (uint8_t *)from;

    if (len > max_len)
        /** TODO: truncate */
        len = max_len;

    memcpy(msg->msg_iov->iov_base, from, len);

    if (addr) {
        addr->sin_family = AF_INET;
        addr->sin_port = 0;
        addr->sin_addr.s_addr = get_ip_hdr(nb)->ip_src;
        *addr_len = sizeof(*addr);
    }

    free_net_buff(nb);

    return len;
}

/*!
 * @brief Deliver network buffer to socket (if exist)
 * @param nb Netbuffer
 * @return True if success
 */
bool ping_rcv(struct net_buff_s *nb) {
    struct socket *sk;

    socket_list_for_each(sk, IPPROTO_ICMP) {
        /* In fact, it is enough to verify only the local port and local IP */
        if ((sk->src_port == get_icmp_hdr(nb)->hdr_data.echo.id) &&
            (sk->src_addr == get_ip_hdr(nb)->ip_dst)) {
            nb_enqueue(nb, &sk->nb_rx_q);
            return true;
        }
    }

    return false;
}
