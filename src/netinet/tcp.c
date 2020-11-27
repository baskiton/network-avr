#include "net/net.h"
#include "net/socket.h"
#include "netinet/in.h"
#include "netinet/ip.h"
#include "netinet/tcp.h"

/*!
 * TODO:
 */
ssize_t tcp_send_msg(struct socket *restrict sk,
                     struct msghdr *restrict msg) {

    return 0;
}
