#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <defines.h>

#include "net/net.h"
#include "net/net_dev.h"

struct net_dev_s *curr_net_dev = NULL;  // global current network device

/*!
 * @brief Allocate & Set Network Device
 * @param size Size of private data
 * @param setup Callback to init device
 */
struct net_dev_s *net_dev_alloc(uint8_t size, void (*setup)(struct net_dev_s *)) {
    struct net_dev_s *ndev;

    ndev = malloc(sizeof(struct net_dev_s) + size);
    if (!ndev) {
        // no mem
        return NULL;
    }

    memset(ndev, 0, sizeof(struct net_dev_s) + size);
    ndev->priv = (void *)ndev + sizeof(struct net_dev_s);
    setup(ndev);

    return ndev;
}

/*!
 *
 */
void net_dev_free(struct net_dev_s *net_dev) {
    free(net_dev);
    net_dev = NULL;
}

/*!
 * @brief Register a network device
 * @param net_dev Device to register
 * @return 0 if success; errno if error
 */
int8_t netdev_register(struct net_dev_s *net_dev) {
    int8_t err = 0;

    if (net_dev->netdev_ops->init) {
        err = net_dev->netdev_ops->init(net_dev);
        if (err)
            return err;
    }

    curr_net_dev = net_dev;

    return err;
}

/*!
 * @brief Unregister a network device
 * @param net_dev Device to unregister
 */
void netdev_unregister(struct net_dev_s *net_dev) {
    if (net_dev->netdev_ops->stop)
        net_dev->netdev_ops->stop(net_dev);
    curr_net_dev = NULL;
}

/*!
 *
 */
int8_t netdev_open(struct net_dev_s *net_dev) {
    int8_t err = 0;

    if (net_dev->flags.up_state)
        return 0;

    if (net_dev->netdev_ops->open)
        err = net_dev->netdev_ops->open(net_dev);

    if (!err) {
        net_dev->flags.up_state = 1;
        netdev_set_rx_mode(net_dev);
    }

    return err;
}

/*!
 *
 */
void netdev_close(struct net_dev_s *net_dev) {
    if (!net_dev->flags.up_state)
        return;
    
    if (net_dev->netdev_ops->stop)
        net_dev->netdev_ops->stop(net_dev);

    net_dev->flags.up_state = 0;
}

/*!
 *
 */
void netdev_set_rx_mode(struct net_dev_s *net_dev) {
    const struct net_dev_ops_s *ops = net_dev->netdev_ops;

    if (!net_dev->flags.up_state)
        return;
    
    if (ops->set_rx_mode)
        ops->set_rx_mode(net_dev);
}
