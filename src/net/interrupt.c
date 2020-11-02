#include <avr/io.h>
#include <avr/interrupt.h>

#include "net/net.h"
#include "net/net_dev.h"
#include "net/ether.h"
#include "net/interrupt.h"

struct nd_irq_hdlr_s {
    struct net_dev_s *net_dev;
};

static struct nd_irq_hdlr_s nd_irq_hdlr = {
    .net_dev = NULL
};

/*!
 * @brief Basic interrupt handler. It needs to be called from the ISR body.
 * E.g.:
ISR(INT0_vect) {
    net_dev_irq_handler();
}
 */
inline void net_dev_irq_handler(void) {
    if (nd_irq_hdlr.net_dev)
        nd_irq_hdlr.net_dev->netdev_ops->irq_handler(nd_irq_hdlr.net_dev);
}

/*!
 * @brief Add a handler for interrupt
 * @param net_dev Network device with \a irq_handler func
 * @return 0 if success
 */
int8_t irq_hdlr_add(struct net_dev_s *net_dev) {
    if (!net_dev ||
        !net_dev->netdev_ops ||
        !net_dev->netdev_ops->irq_handler)
        return -1;

    nd_irq_hdlr.net_dev = net_dev;

    return 0;
}

/*!
 * @brief Delete a handler for interrupt
 */
void irq_hdlr_del(void) {
    nd_irq_hdlr.net_dev = NULL;
}