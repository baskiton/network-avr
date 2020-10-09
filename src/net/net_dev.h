#ifndef NET_DEV_H
#define NET_DEV_H

#include <stdint.h>
#include <stdbool.h>

#include <defines.h>
#include <net/ether.h>
#include <net/net.h>

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
    void (*open)(struct net_dev_s *net_dev);
    void (*stop)(struct net_dev_s *net_dev);
    void (*start_tx)(struct net_buff_s *net_buff, struct net_dev_s *net_dev);
    uint8_t (*set_mac_addr)(struct net_dev_s *net_dev, const void *addr);
    void (*irq_handler)(struct net_dev_s *net_dev);
};

/* Operations for Layer 2 header */
struct header_ops_s {
    void (*create)(struct net_buff_s *net_buf, int16_t type,
                   const void *mac_d, const void *mac_s,
                   int16_t len);
};

/*!
 * @brief Network device structure
 * @param flags State flags
 * @param data pointer to device data
 * @param netdev_ops Callbacks for control functions
 * @param header_ops Callbacks for eth header functions
 */
struct net_dev_s {
    struct {
        uint8_t link_status : 1;
    } flags;
    void *data;
    const struct net_dev_ops_s *netdev_ops;
    const struct header_ops_s *header_ops;
};

inline bool net_check_link(struct net_dev_s *net_dev) {
    return net_dev->flags.link_status;
}

int8_t netdev_register(struct net_dev_s *net_dev);
struct net_dev_s *net_dev_alloc(uint8_t size);

#endif  /* !NET_DEV_H */
