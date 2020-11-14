#include <stdint.h>

#include "net/net.h"
#include "netinet/ip.h"

/*!
 * @brief Handlers for specific protocols.
 */
struct ip_proto_hdlr_ops_s {
    proto_hdlr_t icmp;  // ICMP handler
    proto_hdlr_t tcp;   // TCP handler
    proto_hdlr_t udp;   // UDP handler
};

static struct ip_proto_hdlr_ops_s ip_proto_ops = {
    .icmp = NULL,
    .tcp = NULL,
    .udp = NULL,
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
            if (ip_proto_ops.tcp) {
                return ip_proto_ops.tcp(net_buff);
            }
            break;
        
        case IPPROTO_UDP:
            if (ip_proto_ops.udp) {
                return ip_proto_ops.udp(net_buff);
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
        case IPPROTO_ICMP:
            ip_proto_ops.icmp = handler;
            break;
        
        case IPPROTO_TCP:
            ip_proto_ops.tcp = handler;
            break;
        
        case IPPROTO_UDP:
            ip_proto_ops.udp = handler;
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
        case IPPROTO_ICMP:
            ip_proto_ops.icmp = NULL;
            break;
        
        case IPPROTO_TCP:
            ip_proto_ops.tcp = NULL;
            break;
        
        case IPPROTO_UDP:
            ip_proto_ops.udp = NULL;
            break;
        
        default:
            break;
    }
}
