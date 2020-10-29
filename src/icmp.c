#include <stdint.h>

#include "net/net.h"
#include "net/icmp.h"

/*!
 * @brief Get the ICMP header
 * @param net_buff Pointer to network buffer
 * @return Pointer to ICMP header
 */
static struct icmp_hdr_s *get_icmp_hdr(struct net_buff_s *net_buff) {
    return (void *)(net_buff->head + net_buff->transport_hdr_offset);
}

/*!
 * @brief Initialize ICMP handler
 */
void icmp_init(void) {
    /** TODO: set the valid handlers */
    ip_proto_handler_add(IP_PROTO_ICMP, NULL);
}

/*!
 *
 */
struct net_buff_s *icmp_create() {

    return NULL;
}

/*!
 *
 */
void icmp_send() {
    
}
