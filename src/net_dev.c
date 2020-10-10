#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <defines.h>

#include "net/net.h"
#include "net/net_dev.h"

/*!
 * @brief Register a network device
 * @param net_dev Device to register
 * @return 0 if success; negative if error
 */
int8_t netdev_register(struct net_dev_s *net_dev) {
    int8_t err;

    if (net_dev->netdev_ops->init) {
        err = net_dev->netdev_ops->init(net_dev);
    }

    return err;
}

/*!
 *
 */
struct net_dev_s *net_dev_alloc(uint8_t size) {
    struct net_dev_s *ndev;
    void *priv;

    ndev = malloc(sizeof(struct net_dev_s));
    if (!ndev)
        return NULL;
    priv = malloc(size);
    if (!priv) {
        free(ndev);
        return NULL;
    }
    memset(ndev, 0, sizeof(struct net_dev_s));
    memset(priv, 0, size);
    ndev->data = priv;

    return ndev;
}