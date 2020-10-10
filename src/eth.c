#include <avr/io.h>

#include <stdint.h>
#include <string.h>

#include <net/ether.h>
#include <net/net.h>
#include <net/net_dev.h>

/*!
 * @brief Create the Ethernet Header
 * @param net_buff Buffer to create header
 * @param type Ethernet type
 * @param mac_d Destination MAC address
 * @param mac_s Source MAC address
 * @param len Packet length
 */
int8_t eth_header_create(struct net_buff_s *net_buff, int16_t type,
                         const void *mac_d, const void *mac_s,
                         int16_t len) {
    struct eth_header_s *header_p;

    header_p = (struct eth_header_s *)(net_buff->head + net_buff->mac_header);
    memcpy(header_p->mac_dest, mac_d, ETH_MAC_LEN);
    memcpy(header_p->mac_src, mac_s, ETH_MAC_LEN);

    if (type <= 1500)
        header_p->eth_type = htons(len);
    else
        header_p->eth_type = htons(type);
    
    return 0;
}

const struct header_ops_s eth_header_ops = {
    .create = eth_header_create
};

/*!
 * @brief Setut the Ethernet Netwirk Device
 * @param dev Network device
 */
void ether_setup(struct net_dev_s *dev) {
    dev->header_ops = &eth_header_ops;
}
