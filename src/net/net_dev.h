#ifndef NET_DEV_H
#define NET_DEV_H

#include <stdint.h>
#include <stdbool.h>

#include <defines.h>

#include "ether.h"
#include "net.h"

extern struct net_dev_s *curr_net_dev;

enum ndev_rx_mode {
    RX_RT_BROADCAST,    // receive broadcast & unicast frames filtering (by default)
    RX_RT_MULTICAST,    // receive multicast & unicast frames filtering
    RX_RT_UNICAST,      // receive unicast frames filtering
    RX_RT_ALLMULTI,     // receive broadcast & multicast & unicast frames filtering
    RX_RT_PROMISC = 7,  // receive all frames promiscuously
};

struct net_dev_s;
struct net_buff_s;

/*!
 * @param init Function for initialize a network device
 * @param open Function for change the state of net device to up
 * @param stop Function for change the state of net device to down
 * @param start_tx Function for transmit the packet
 * @param set_mac_addr Function for change the MAC address
 */
struct net_dev_ops_s {
    int8_t (*init)(struct net_dev_s *net_dev);
    int8_t (*open)(struct net_dev_s *net_dev);
    void (*stop)(struct net_dev_s *net_dev);
    void (*start_tx)(struct net_buff_s *net_buff, struct net_dev_s *net_dev);
    void (*set_rx_mode)(struct net_dev_s *net_dev);
    int8_t (*set_mac_addr)(struct net_dev_s *net_dev, const void *addr);
    void (*irq_handler)(struct net_dev_s *net_dev);
};

/* Operations for Layer 2 header */
struct header_ops_s {
    int8_t (*create)(struct net_buff_s *net_buf, int16_t type,
                     const void *mac_d, const void *mac_s,
                     int16_t len);
    // int8_t (*parse)(const struct net_buff_s *net_buf, uint8_t *h_addr);
    // uint16_t (*parse_potocol)(const struct net_buff_s *net_buf);
};

/*!
 * @brief Network device structure
 * @param flags State flags
 * @param netdev_ops Callbacks for control functions
 * @param header_ops Callbacks for eth header functions
 * @param dev_addr Hardware address (MAC)
 * @param priv pointer to device private data
 */
typedef struct net_dev_s {
    struct {
        uint8_t up_state : 1;       // 1 - is running; 0 - is stopped
        uint8_t link_status : 1;    // 1 - Link i Up; 0 - Link is Down
        uint8_t full_duplex : 1;    // 1 - Full Duplex; 0 - Half Duplex
        uint8_t rx_mode : 3;        // see ndev_rx_mode enum
    } flags;
    const struct net_dev_ops_s *netdev_ops;
    const struct header_ops_s *header_ops;
    uint8_t dev_addr[6];    /** FIXME: ETH_MAC_LEN */
    void *priv;
} net_dev_t;

inline bool net_check_link(struct net_dev_s *net_dev) {
    return net_dev->flags.link_status;
}

struct net_dev_s *net_dev_alloc(uint8_t size, void (*setup)(struct net_dev_s *));
void net_dev_free(struct net_dev_s *net_dev);
int8_t netdev_register(struct net_dev_s *net_dev);
void netdev_unregister(struct net_dev_s *net_dev);
int8_t netdev_open(struct net_dev_s *net_dev);
void netdev_set_rx_mode(struct net_dev_s *net_dev);

#endif  /* !NET_DEV_H */
