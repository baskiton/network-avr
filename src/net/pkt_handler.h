#ifndef PKT_HANDLER_H
#define PKT_HANDLER_H

#include <stdint.h>

#include "net/net.h"

typedef int8_t (*proto_hdlr_t)(struct net_buff_s *net_buff);

/*!
 * @brief Handlers for specific protocols.
 */
struct recv_pkt_hdlr_ops_s {
    proto_hdlr_t eth_ip;    // IPv4 handler
    proto_hdlr_t eth_arp;   // ARP handler
    proto_hdlr_t eth_ipv6;  // IPv6 handler
};

void recv_pkt_handler(struct net_buff_s *net_buff);
void pkt_hdlr_add(uint16_t type, proto_hdlr_t handler);
void pkt_hdlr_del(uint16_t type);

#endif  /* !PKT_HANDLER_H */
