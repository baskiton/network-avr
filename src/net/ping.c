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
 *
 */
ssize_t ping_send_msg(struct socket *restrict sk,
                      struct msghdr *restrict msg) {
    struct icmp_hdr_s *icmph = msg->msg_iov->iov_base;
    struct sockaddr_in *addr_in = msg->msg_name;
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
        sk->dst_addr = addr_in->sin_addr.s_addr;
        sk->dst_port = addr_in->sin_port = 0;
    /* } else {
     * the socket is assumed to have already been established,
     * so the existing values are used.
     */
    }

    icmph->hdr_data.echo.id = sk->src_port;
    icmph->chks = 0;
    icmph->chks = in_checksum(icmph, len);

    nb = ip_create_nb(sk, msg, 0, len);
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
 *
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

    nb = net_buff_rcv(&sk->nb_rx_q, flags);
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
    struct ip_hdr_s *iph = get_ip_hdr(nb);
    struct icmp_hdr_s *icmph = get_icmp_hdr(nb);
    struct sock_ap_pairs_s pairs = {
        .loc_addr = iph->ip_dst,
        .loc_port = icmph->hdr_data.echo.id,
        .fe_addr = iph->ip_src,
        .fe_port = 0,
    };

    sk = socket_find(&pairs);
    if (!sk)
        return false;

    nb_enqueue(nb, &sk->nb_rx_q);

    return true;

}
