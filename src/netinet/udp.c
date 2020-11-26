#include <string.h>

#include "net/net.h"
#include "net/socket.h"
#include "netinet/in.h"
#include "netinet/ip.h"
#include "netinet/udp.h"

static int8_t udp_send(struct socket *sk, struct sockaddr_in *daddr) {
    struct udp_hdr_s *udph;
    struct net_buff_s *nb;

    nb = sk->nb_tx_q.next;

    /* UDP header create */
    udph = get_udp_hdr(nb);

    udph->port_src = sk->src_port;
    udph->port_dst = daddr->sin_port;
    udph->len = htons(nb->pkt_len - nb->transport_hdr_offset);
    udph->chks = 0;

    /** TODO: calculate UDP checksum */

    return ip_send_sock(sk);
}

/*!
 * @brief Send data over UDP
 */
ssize_t udp_send_msg(struct socket *restrict sk,
                     struct msghdr *restrict msg) {
    size_t ulen = msg->msg_iov->iov_len;
    struct sockaddr_in *addr_in = msg->msg_name;
    struct sockaddr_in daddr;
    struct net_buff_s *nb;
    int8_t err;

    /* UDP does not support out-of-band data */
    if (msg->msg_flags & MSG_OOB)
        // EOPNOTSUPP
        return -1;

    ulen += sizeof(struct udp_hdr_s);

    /* verify address */
    if (addr_in) {
        if ((msg->msg_namelen < sizeof(*addr_in)) ||
            (addr_in->sin_port == 0))
            // EINVAL
            return -1;

        if ((addr_in->sin_family != AF_INET) &&
            (addr_in->sin_family != AF_UNSPEC)) {
            // EAFNOSUPPORT
            return -1;
        }

        memcpy(&daddr, addr_in, sizeof(daddr));
    } else {
        /* the socket is assumed to have already been established,
         * so the existing values are used.
         */
        daddr.sin_family = AF_INET;
        daddr.sin_addr.s_addr = sk->dst_addr;
        daddr.sin_port = sk->dst_port;
    }

    nb = ip_create_nb(sk, msg, sizeof(struct udp_hdr_s), ulen, &daddr);
    if (!nb)
        // error
        return -1;

    err = udp_send(sk, &daddr);

    if (err) {
        if (err > 0)
            err = -err;
        return err;
    }

    return msg->msg_iov->iov_len;
}

/*!
 *
 */
ssize_t udp_recv_msg(struct socket *restrict sk,
                     struct msghdr *restrict msg,
                     int8_t flags, socklen_t *restrict addr_len) {
    struct net_buff_s *nb;
    size_t max_len = msg->msg_iov->iov_len;
    size_t len;
    struct sockaddr_in *addr = msg->msg_name;
    void *from;
    struct udp_hdr_s *udph;

    if (!max_len)
        return(0);

    nb = net_buff_rcv(sk, flags);
    if (!nb)
        return -1;

    udph = get_udp_hdr(nb);

    from = udph + 1;
    len = ntohs(udph->len) - sizeof(*udph);

    if (len > max_len)
        /** TODO: truncate */
        len = max_len;

    /** TODO: validate checksum! */

    memcpy(msg->msg_iov->iov_base, from, len);

    /* copy address */
    if (addr) {
        addr->sin_family = AF_INET;
        addr->sin_port = udph->port_src;
        addr->sin_addr.s_addr = get_ip_hdr(nb)->ip_src;
        *addr_len = sizeof(*addr);
    }

    free_net_buff(nb);

    return len;
}
