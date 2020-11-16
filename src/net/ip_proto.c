#include <stdint.h>

#include "net/net.h"
#include "netinet/ip.h"

/*!
 * @brief Handlers for specific protocols.
 */
struct ip_proto_hdlr_ops_s {
    proto_hdlr_t ip_c;  // IP common handler
    proto_hdlr_t icmp;  // ICMP handler
};

static struct ip_proto_hdlr_ops_s ip_proto_ops = {
    .ip_c = NULL,
    .icmp = NULL,
};

/*!
 * @brief Handler for IPv4
 * @param proto IP number
 * @param net_buff Pointer to receive net buffer
 * @return 0 if success; 1 if drop
 */
int8_t ip_proto_handler(uint8_t proto, struct net_buff_s *net_buff) {
    switch (proto) {
        case IPPROTO_ICMP:
            if (ip_proto_ops.icmp) {
                return ip_proto_ops.icmp(net_buff);
            }
            break;
        
        case IPPROTO_TCP:
        case IPPROTO_UDP:
            if (ip_proto_ops.ip_c) {
                return ip_proto_ops.ip_c(net_buff);
            }
            break;
        
        default:
            break;
    }
    free_net_buff(net_buff);

    return NETDEV_RX_DROP;  // No Handler, packet drop
}

/*!
 * @brief Add the IP handler
 * @param proto IP number
 * @param handler Handler function for this protocol
 */
void ip_proto_handler_add(uint8_t proto, proto_hdlr_t handler) {
    switch (proto) {
        case IPPROTO_IP:
            ip_proto_ops.ip_c = handler;
            break;
        
        case IPPROTO_ICMP:
            ip_proto_ops.icmp = handler;
            break;
        
        default:
            break;
    }
}

/*!
 * @brief Delete the IP handler
 * @param proto IP number
 */
void ip_proto_handler_del(uint8_t proto) {
    switch (proto) {
        case IPPROTO_IP:
            ip_proto_ops.ip_c = NULL;
            break;
        
        case IPPROTO_ICMP:
            ip_proto_ops.icmp = NULL;
            break;
        
        default:
            break;
    }
}
