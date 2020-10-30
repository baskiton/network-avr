#include "net/net.h"

/*!
 * @brief Initialize UDP handler
 */
void udp_init(void) {
    /** TODO: set the valid handlers */
    ip_proto_handler_add(IP_PROTO_UDP, NULL);
}
