#ifndef NET_ETHER_H
#define NET_ETHER_H

#include <stdint.h>

#include "net/net.h"
#include "net/net_dev.h"

#define ETH_MAC_LEN 6U
#define ETH_HDR_LEN 14U

/* Ether Types */
#define ETH_P_IP        0x0800  // Internet Protocol
#define ETH_P_ARP       0x0806  // Address Resolution Protocol
#define ETH_P_IPV6      0x86DD  // Internet Protocol Version 6 (IPv6)
#define ETH_P_802_3_MIN 0x0600
#define ETH_P_802_2     0x0004  // 802.2 frame

struct eth_header_s {
    uint8_t mac_dest[ETH_MAC_LEN];
    uint8_t mac_src[ETH_MAC_LEN];
    uint16_t eth_type;
};

struct eth_frame_s {
    struct eth_header_s *header;
    uint8_t *payload;
};

int8_t eth_header_create(struct net_buff_s *net_buff, struct net_dev_s *net_dev,
                         int16_t type, const void *mac_d, const void *mac_s,
                         int16_t len);
void ether_setup(struct net_dev_s *net_dev);
struct net_dev_s *eth_dev_alloc(uint8_t size);
uint16_t eth_type_proto(struct net_buff_s *net_buff, struct net_dev_s *net_dev);

#endif  /* !NET_ETHER_H */
