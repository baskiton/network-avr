#include <stdint.h>

#include "net/net.h"
#include "net/ether.h"
#include "net/pkt_handler.h"

static struct recv_pkt_hdlr_ops_s recv_ops = {
    .eth_ip = NULL,
    .eth_arp = NULL,
    .eth_ipv6 = NULL
};

/*!
 * @brief Received Packet Handler
 * @param net_buff Pointer to receive net buffer
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
        
        case htons(ETH_P_IPV6):
            if (recv_ops.eth_ipv6) {
                return recv_ops.eth_ipv6(net_buff);
            }
            break;
        
        default:
            break;
    }
    free_net_buff(net_buff);

    return -1;  // No Handler
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
        
        case ETH_P_IPV6:
            recv_ops.eth_ipv6 = handler;
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
        
        case ETH_P_IPV6:
            recv_ops.eth_ipv6 = NULL;
            break;
        
        default:
            break;
    }
}

