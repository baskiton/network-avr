#ifndef INTERRUPT_H
#define INTERRUPT_H

#include <stdint.h>

#include "net/net.h"
#include "net/net_dev.h"

typedef void (*irq_handler_t)(struct net_dev_s *net_dev);

int8_t request_irq(irq_handler_t handler);

#endif  /* !INTERRUPT_H */
