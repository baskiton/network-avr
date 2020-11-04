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
    typedef int8_t (*func_t)(struct net_dev_s *);
    func_t f = pgm_read_ptr(&net_dev->netdev_ops->init);
    int8_t err = 0;

    if (f) {
        err = f(net_dev);
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
    typedef void (*func_t)(struct net_dev_s *);
    func_t f = pgm_read_ptr(&net_dev->netdev_ops->stop);

    if (f)
        f(net_dev);

    curr_net_dev = NULL;
}

/*!
 *
 */
void netdev_set_rx_mode(struct net_dev_s *net_dev) {
    typedef void (*func_t)(struct net_dev_s *);
    func_t f;

    if (!net_dev_upstate_is_run(net_dev))
        return;

    f = pgm_read_ptr(&net_dev->netdev_ops->set_rx_mode);
    
    if (f)
        f(net_dev);
}

/*!
 *
 */
int8_t netdev_open(struct net_dev_s *net_dev) {
    typedef int8_t (*func_t)(struct net_dev_s *);
    func_t f;
    int8_t err = 0;

    if (net_dev_upstate_is_run(net_dev))
        return 0;

    f = pgm_read_ptr(&net_dev->netdev_ops->open);

    if (f)
        err = f(net_dev);

    if (!err) {
        net_dev_set_upstate_run(net_dev);
        netdev_set_rx_mode(net_dev);
    }

    return err;
}

/*!
 *
 */
void netdev_close(struct net_dev_s *net_dev) {
    typedef void (*func_t)(struct net_dev_s *);
    func_t f;

    if (!net_dev_upstate_is_run(net_dev))
        return;

    f = pgm_read_ptr(&net_dev->netdev_ops->stop);

    if (f)
        f(net_dev);

    net_dev_set_upstate_stop(net_dev);
}

/*!
 * @brief Start transmit a buffer
 * @param net_buff buffer to transmit
 * @return errno if error occured
 */
int8_t netdev_start_tx(struct net_buff_s *net_buff) {
    typedef int8_t (*func_t)(struct net_buff_s *, struct net_dev_s *);
    struct net_dev_s *net_dev = net_buff->net_dev;
    func_t f = pgm_read_ptr(&net_dev->netdev_ops->start_tx);
    int8_t ret = -1;    // ENETDOWN

    if (net_dev_upstate_is_run(net_dev) && net_dev_link_is_up(net_dev)) {
        if (net_dev_tx_is_allow(net_dev)) {
            ret = f(net_buff, net_dev);

            if (ret == NETDEV_TX_OK)
                return ret;
        }
    }

    free_net_buff(net_buff);
    
    return ret;
}

/*!
 * @brief Set MAC address for network device
 * @param net_dev Device
 * @param addr Pointer to MAC
 * @return 0 if success
 */
int8_t netdev_set_mac_addr(struct net_dev_s *net_dev, const void *addr) {
    typedef int8_t (*func_t)(struct net_dev_s *, const void *);
    func_t f = pgm_read_ptr(&net_dev->netdev_ops->set_mac_addr);

    if (!addr)
        // EINVAL
        return -1;

    if (!f)
        // EOPNOTSUPP
        return -1;

    return f(net_dev, addr);
}