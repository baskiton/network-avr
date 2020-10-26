#ifndef INTERRUPT_H
#define INTERRUPT_H

#include <stdint.h>

#include "net/net.h"
#include "net/net_dev.h"

typedef void (*irq_handler_t)(struct net_dev_s *net_dev);

void net_dev_irq_handler(void);
int8_t irq_hdlr_add(struct net_dev_s *net_dev);
void irq_hdlr_del(void);

#endif  /* !INTERRUPT_H */
