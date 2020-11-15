#include "net/net.h"
#include "net/socket.h"
#include "netinet/in.h"
#include "netinet/ip.h"
#include "netinet/udp.h"

/*!
 * @brief Get the UDP header
 * @param net_buff Pointer to network buffer
 * @return Pointer to UDP header
 */
struct udp_hdr_s *get_udp_hdr(struct net_buff_s *net_buff) {
    return (void *)(net_buff->head + net_buff->transport_hdr_offset);
}

/*!
 * @brief Initialize UDP handler
 */
void udp_init(void) {
    /** TODO: set the valid handlers */
    ip_proto_handler_add(IPPROTO_UDP, NULL);
}

static int8_t udp_send(struct socket *sk) {
    struct udp_hdr_s *udph;
    struct net_buff_s *nb;

    nb = sk->nb_tx_q.next;

    /* UDP header create */
    udph = get_udp_hdr(nb);

    udph->port_src = sk->src_port;
    udph->port_dst = sk->dst_port;
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

        sk->dst_addr = addr_in->sin_addr.s_addr;
        sk->dst_port = addr_in->sin_port;
    /* } else {
     * the socket is assumed to have already been established,
     * so the existing values are used.
     */
    }

    nb = ip_create_nb(sk, msg, sizeof(struct udp_hdr_s), ulen);
    if (!nb)
        // error
        return -1;

    err = udp_send(sk);

    if (err) {
        if (err > 0)
            err = -err;
        return err;
    }

    return msg->msg_iov->iov_len;;
}
