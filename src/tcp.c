#include "net/net.h"

/*!
 * @brief Initialize TCP handler
 */
void tcp_init(void) {
    /** TODO: set the valid handlers */
    ip_proto_handler_add(IP_PROTO_TCP, NULL);
}
