#include <stdint.h>

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
