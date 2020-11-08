#include "net/net.h"
#include "net/socket.h"
#include "netinet/in.h"
#include "netinet/ip.h"
#include "netinet/udp.h"

/*!
 * @brief Initialize UDP handler
 */
void udp_init(void) {
    /** TODO: set the valid handlers */
    ip_proto_handler_add(IPPROTO_UDP, NULL);
}

/*!
 * @brief Send data over UDP
 */
ssize_t udp_send_msg(struct socket *restrict sk,
                     struct msghdr *restrict msg) {
    ssize_t len = msg->msg_iov->iov_len;
    ssize_t ulen = len;
    struct net_buff_s *nb;

    /* UDP does not support out-of-band data */
    if (msg->msg_flags & MSG_OOB)
        // EOPNOTSUPP
        return -1;

    /** TODO: */

    return 0;
}
