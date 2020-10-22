#ifndef ETHER_H
#define ETHER_H

#include <stdint.h>

#include "net.h"
#include "net_dev.h"

#define ETH_MAC_LEN 6U
#define ETH_HDR_LEN 14U

#define ETH_P_IP        0x0800  // Internet Protocol packet
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

#endif  /* !ETHER_H */
