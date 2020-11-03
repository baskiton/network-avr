#include "net/net.h"
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
