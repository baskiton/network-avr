#include <avr/pgmspace.h>

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <defines.h>

#include "net/net.h"
#include "net/net_dev.h"
#include "net/nb_queue.h"

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
 * @brief Free memory used by network device
 * @param net_dev pointer to device to free
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
    int8_t (*init_f)(struct net_dev_s *);
    init_f = pgm_read_ptr(&net_dev->netdev_ops->init);
    int8_t err = 0;

    if (init_f) {
        err = init_f(net_dev);
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
    void (*stop_f)(struct net_dev_s *);
    stop_f = pgm_read_ptr(&net_dev->netdev_ops->stop);

    if (stop_f)
        stop_f(net_dev);

    curr_net_dev = NULL;
}

/*!
 * @brief Apply rx mode settings to network device.
 */
void netdev_set_rx_mode(struct net_dev_s *net_dev) {
    void (*set_rx_mode_f)(struct net_dev_s *);

    if (!net_dev_upstate_is_run(net_dev))
        return;

    set_rx_mode_f = pgm_read_ptr(&net_dev->netdev_ops->set_rx_mode);

    if (set_rx_mode_f)
        set_rx_mode_f(net_dev);
}

/*!
 * @brief Open network device to working with it
 * @return 0 on succes
 */
int8_t netdev_open(struct net_dev_s *net_dev) {
    int8_t (*open_f)(struct net_dev_s *);
    int8_t err = 0;

    if (net_dev_upstate_is_run(net_dev))
        return 0;

    open_f = pgm_read_ptr(&net_dev->netdev_ops->open);

    if (open_f)
        err = open_f(net_dev);

    if (!err) {
        net_dev_set_upstate_run(net_dev);
        netdev_set_rx_mode(net_dev);
    }

    return err;
}

/*!
 * @brief Stopped network device
 */
void netdev_close(struct net_dev_s *net_dev) {
    void (*stop_f)(struct net_dev_s *);

    if (!net_dev_upstate_is_run(net_dev))
        return;

    stop_f = pgm_read_ptr(&net_dev->netdev_ops->stop);

    if (stop_f)
        stop_f(net_dev);

    net_dev_set_upstate_stop(net_dev);
}

static inline int8_t netdev_start_tx(struct net_buff_s *net_buff,
                                     struct net_dev_s *net_dev) {
    int8_t (*start_tx_f)(struct net_buff_s *, struct net_dev_s *);
    start_tx_f = pgm_read_ptr(&net_dev->netdev_ops->start_tx);

    return start_tx_f(net_buff, net_dev);   // start_tx()
}

/*!
 * @brief Start transmit a list of buffers
 * @param net_buff buffer to transmit
 * @return 0 on success
 */
int8_t netdev_list_xmit(struct net_buff_s *net_buff) {
    struct net_dev_s *net_dev = net_buff->net_dev;
    int8_t err = NETDEV_TX_BUSY;

    while (net_dev_upstate_is_run(net_dev) &&
           net_dev_link_is_up(net_dev) &&
           net_buff) {
        if (!net_dev_tx_is_allow(net_dev))
            continue;

        struct net_buff_s *next = net_buff->next;

        net_buff->next = NULL;

        err = netdev_start_tx(net_buff, net_dev);

        if (!net_dev_xmit_complete(err)) {
            net_buff->next = next;
            continue;
        }

        net_buff = next;
    }

    if (net_dev_xmit_complete(err))
        return err;

    err = -1;   // ENETDOWN
    free_net_buff_list(net_buff);

    return err;
}

/*!
 * @brief Start transmit a queue of buffers
 * @param queue Queue to transmit
 * @return 0 on success
 */
int8_t netdev_queue_xmit(struct nb_queue_s *queue) {
    struct net_buff_s *nb;
    int8_t err = NETDEV_TX_OK;

    while ((nb = nb_dequeue(queue))) {
        struct net_dev_s *ndev = nb->net_dev;
        err = -1;   // ENETDOWN

        while (net_dev_upstate_is_run(ndev) &&
               net_dev_link_is_up(ndev)) {
            if (!net_dev_tx_is_allow(ndev))
                continue;

            err = netdev_start_tx(nb, ndev);

            if (net_dev_xmit_complete(err))
                break;
        }

        if (!net_dev_xmit_complete(err)) {
            nb_queue_clear(queue);
            err = -1;   // ENETDOWN
            break;
        }
    }

    return err;
}

/*!
 * @brief Set MAC address for network device
 * @param net_dev Device
 * @param addr Pointer to MAC
 * @return 0 if success
 */
int8_t netdev_set_mac_addr(struct net_dev_s *net_dev, const void *addr) {
    int8_t (*set_mac_addr_f)(struct net_dev_s *, const void *);
    set_mac_addr_f = pgm_read_ptr(&net_dev->netdev_ops->set_mac_addr);

    if (!addr)
        // EINVAL
        return -1;

    if (!set_mac_addr_f)
        // EOPNOTSUPP
        return -1;

    return set_mac_addr_f(net_dev, addr);
}
