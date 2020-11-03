#include "net/net.h"
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
