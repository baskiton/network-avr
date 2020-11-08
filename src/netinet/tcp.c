#include "net/net.h"
#include "net/socket.h"
#include "netinet/in.h"
#include "netinet/ip.h"
#include "netinet/tcp.h"

/*!
 * @brief Initialize TCP handler
 */
void tcp_init(void) {
    /** TODO: set the valid handlers */
    ip_proto_handler_add(IPPROTO_TCP, NULL);
}

/*!
 *
 */
ssize_t tcp_send_msg(struct socket *restrict sk,
                     struct msghdr *restrict msg) {

    return 0;
}
