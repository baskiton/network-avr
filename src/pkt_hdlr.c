#include <stdint.h>

#include "net/net.h"
#include "net/ether.h"
#include "net/pkt_handler.h"

/*!
 * @brief Handlers for specific protocols.
 */
struct recv_pkt_hdlr_ops_s {
    proto_hdlr_t eth_ip;    // IPv4 handler
    proto_hdlr_t eth_arp;   // ARP handler
};

static struct recv_pkt_hdlr_ops_s recv_ops = {
    .eth_ip = NULL,
    .eth_arp = NULL,
};

/*!
 * @brief Received Packet Handler
 * @param net_buff Pointer to receive net buffer
 * @return 0 if success; 1 if drop
 */
int8_t recv_pkt_handler(struct net_buff_s *net_buff) {
    switch (net_buff->protocol) {
        case htons(ETH_P_IP):
            if (recv_ops.eth_ip) {
                return recv_ops.eth_ip(net_buff);
            }
            break;
        
        case htons(ETH_P_ARP):
            if (recv_ops.eth_arp) {
                return recv_ops.eth_arp(net_buff);
            }
            break;
        
        default:
            break;
    }
    free_net_buff(net_buff);

    return NETDEV_RX_DROP;  // No Handler, packet drop
}

/*!
 * @brief Add Packet Handler
 * @param type Packet type (e.g. ETH_P_IP)
 * @param handler Handler function for this type
 */
void pkt_hdlr_add(uint16_t type, proto_hdlr_t handler) {
    switch (type) {
        case ETH_P_IP:
            recv_ops.eth_ip = handler;
            break;
        
        case ETH_P_ARP:
            recv_ops.eth_arp = handler;
            break;
        
        default:
            break;
    }
}

/*!
 * @brief Remove Packet Handler
 * @param type Packet type (e.g. ETH_P_IP)
 */
void pkt_hdlr_del(uint16_t type) {
    switch (type) {
        case ETH_P_IP:
            recv_ops.eth_ip = NULL;
            break;
        
        case ETH_P_ARP:
            recv_ops.eth_arp = NULL;
            break;
        
        default:
            break;
    }
}

