#include <avr/io.h>
#include <avr/pgmspace.h>

#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#include "net/ether.h"
#include "net/net.h"
#include "net/net_dev.h"

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

    header_p = (struct eth_header_s *)(net_buff->head + net_buff->mac_hdr_offset);
    memcpy(header_p->mac_dest, mac_d, ETH_MAC_LEN);
    memcpy(header_p->mac_src, mac_s, ETH_MAC_LEN);

    if (type <= 1500)
        header_p->eth_type = htons(len);
    else
        header_p->eth_type = htons(type);
    
    return 0;
}

/** TODO: move this to PROGMEM */
static const struct header_ops_s eth_header_ops = {
    .create = eth_header_create
};

/*!
 * @brief Setut the Ethernet Netwirk Device
 * @param dev Network device
 */
void ether_setup(struct net_dev_s *net_dev) {
    net_dev->header_ops = &eth_header_ops;
    net_dev->flags.rx_mode = RX_RT_BROADCAST | RX_RT_MULTICAST;

    memset(net_dev->broadcast, 0xFF, ETH_MAC_LEN);
}

/*!
 * @brief Allocate & Set Ethernet Device
 * @param size Size of private data
 */
struct net_dev_s *eth_dev_alloc(uint8_t size) {
    return net_dev_alloc(size, ether_setup);
}

/*!
 * @brief Compare two MAC addr
 * @return True if equal
 */
static bool mac_addr_equal(const uint8_t *addr1, const uint8_t *addr2) {
    const uint16_t *a = (const uint16_t *)addr1;
    const uint16_t *b = (const uint16_t *)addr2;

    return ((a[0] ^ b[0]) | (a[1] ^ b[1]) | (a[2] ^ b[2])) == 0;
}

/*!
 * @brief Determine the packet protocol and construct
 * @param net_buff Receiving socket data
 * @param net_dev Receiving net device
 * @return Packet Protocol ID
 */
uint16_t eth_type_proto(struct net_buff_s *net_buff, struct net_dev_s *net_dev) {
    struct eth_header_s *ehdr;

    net_buff->net_dev = net_dev;
    net_buff->mac_hdr_offset = net_buff->data - net_buff->head;
    ehdr = (struct eth_header_s *)net_buff->data;
    
    if (ETH_HDR_LEN <= net_buff->pkt_len) {
        net_buff->pkt_len -= ETH_HDR_LEN;
        net_buff->data += ETH_HDR_LEN;
    }

    if (!mac_addr_equal(net_dev->dev_addr, ehdr->mac_dest)) {
        if (ehdr->mac_dest[0] & 0x01) {
            if (mac_addr_equal(ehdr->mac_dest, net_dev->broadcast))
                net_buff->flags.pkt_type = PKT_BROADCAST;
            else
                net_buff->flags.pkt_type = PKT_MULTICAST;
        } else
            net_buff->flags.pkt_type = PKT_OTHERHOST;
    } else
        net_buff->flags.pkt_type = PKT_HOST;

    if (ehdr->eth_type >= htons(ETH_P_802_3_MIN))
        return ehdr->eth_type;
    
    return htons(ETH_P_802_2);
}
