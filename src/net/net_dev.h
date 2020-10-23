#ifndef NET_DEV_H
#define NET_DEV_H

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include <defines.h>

#include "ether.h"
#include "net.h"

extern struct net_dev_s *curr_net_dev;

#define NETDEV_TX_OK 0
#define NETDEV_TX_BUSY (uint8_t)~0U

#define RX_RT_BROADCAST (1 << 0)   // receive broadcast & unicast frames filtering (by default)
#define RX_RT_MULTICAST (1 << 1)   // receive multicast & unicast frames filtering
#define RX_RT_UNICAST   (1 << 2)   // receive unicast frames filtering
#define RX_RT_ALLMULTI  (1 << 3)   // receive broadcast & multicast & unicast frames filtering
#define RX_RT_PROMISC   (1 << 4)// receive all frames promiscuously

struct net_dev_s;
struct net_buff_s;

/*!
 * @param init Function for initialize a network device
 * @param open Function for change the state of net device to up
 * @param stop Function for change the state of net device to down
 * @param start_tx Function for transmit the packet
 * @param set_mac_addr Function for change the MAC address
 * @param set_dev_settings Set device settings
 */
struct net_dev_ops_s {
    int8_t (*init)(struct net_dev_s *net_dev);
    int8_t (*open)(struct net_dev_s *net_dev);
    void (*stop)(struct net_dev_s *net_dev);
    int8_t (*start_tx)(struct net_buff_s *net_buff, struct net_dev_s *net_dev);
    void (*set_rx_mode)(struct net_dev_s *net_dev);
    int8_t (*set_mac_addr)(struct net_dev_s *net_dev, const void *addr);
    int8_t (*set_dev_settings)(struct net_dev_s *net_dev, bool full_duplex);
    void (*irq_handler)(struct net_dev_s *net_dev);
};

/* Operations for Layer 2 header */
struct header_ops_s {
    int8_t (*create)(struct net_buff_s *net_buf, struct net_dev_s *net_dev,
                     int16_t type, const void *mac_d, const void *mac_s,
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
        uint8_t link_status : 1;    // 1 - Link is Up; 0 - Link is Down
        uint8_t full_duplex : 1;    // 1 - Full Duplex; 0 - Half Duplex
        uint8_t rx_mode : 5;        // see ndev_rx_mode enum
        uint8_t tx_allow : 1;       // 1 - tx allow; 0 - stop tx
    } flags;
    uint8_t broadcast[6];
    const struct net_dev_ops_s *netdev_ops;
    const struct header_ops_s *header_ops;
    uint8_t dev_addr[6];    /** FIXME: ETH_MAC_LEN */
    void *priv;
} net_dev_t;

/*!
 * @brief Set Up State flag of network device to Runing
 */
inline void net_dev_set_upstate_run(struct net_dev_s *net_dev) {
    net_dev->flags.up_state = 1;
}

/*!
 * @brief Set Up State flag of network device to Stopped
 */
inline void net_dev_set_upstate_stop(struct net_dev_s *net_dev) {
    net_dev->flags.up_state = 0;
}

/*!
 * @brief Check link status of network device
 * @return True if Link is UP
 */
inline bool net_dev_upstate_is_run(struct net_dev_s *net_dev) {
    return net_dev->flags.up_state;
}

/*!
 * @brief Set Link Status flag of network device to UP
 */
inline void net_dev_set_link_up(struct net_dev_s *net_dev) {
    net_dev->flags.link_status = 1;
}

/*!
 * @brief Set Link Status flag of network device to DOWN
 */
inline void net_dev_set_link_down(struct net_dev_s *net_dev) {
    net_dev->flags.link_status = 0;
}

/*!
 * @brief Check link status of network device
 * @return True if Link is UP
 */
inline bool net_dev_link_is_up(struct net_dev_s *net_dev) {
    return net_dev->flags.link_status;
}

/*!
 * @brief Allow transmit packet
 */
inline void net_dev_tx_allow(struct net_dev_s *net_dev) {
    net_dev->flags.tx_allow = 1;
}

/*!
 * @brief Disallow transmit packet
 */
inline void net_dev_tx_disallow(struct net_dev_s *net_dev) {
    net_dev->flags.tx_allow = 0;
}

/*!
 * @brief Check if transfer is allowed
 * @return True if TX is allowed
 */
inline bool net_dev_tx_is_allow(struct net_dev_s *net_dev) {
    return net_dev->flags.tx_allow;
}

/*!
 * @brief Create Headre for network device
 * @param net_buff Network Buffer
 * @param net_dev Network Device
 * @param type Type of packet
 * @param d_addr Destination Hardwae Address
 * @param s_addr Source Hardwae Address
 * @param len Length of packet
 * @return 0 if success
 */
inline int8_t netdev_hdr_create(struct net_buff_s *net_buff,
                                struct net_dev_s *net_dev,
                                int16_t type,
                                const void *d_addr, const void *s_addr,
                                int16_t len) {
    if (!net_dev->header_ops || !net_dev->header_ops->create)
        return -1;
    
    return net_dev->header_ops->create(net_buff, net_dev, type, d_addr, s_addr, len);
}

/*!
 * @brief Set settings for network device
 * @param net_dev Pointer to Network Device
 * @param full_duplex Duplex mode; \a true - full duplex;
 *                                 \a false - half duplex
 * @return 0 if success
 */
inline int8_t netdev_set_settings(struct net_dev_s *net_dev, bool full_duplex) {
    if (!net_dev->netdev_ops->set_dev_settings)
        return -1;  // EOPNOTSUPP

    return net_dev->netdev_ops->set_dev_settings(net_dev, full_duplex);
}

struct net_dev_s *net_dev_alloc(uint8_t size, void (*setup)(struct net_dev_s *));
void net_dev_free(struct net_dev_s *net_dev);
int8_t netdev_register(struct net_dev_s *net_dev);
void netdev_unregister(struct net_dev_s *net_dev);
int8_t netdev_open(struct net_dev_s *net_dev);
void netdev_close(struct net_dev_s *net_dev);
void netdev_set_rx_mode(struct net_dev_s *net_dev);
int8_t netdev_start_tx(struct net_buff_s *net_buff);

#endif  /* !NET_DEV_H */
