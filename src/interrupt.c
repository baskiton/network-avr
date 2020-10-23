#include <avr/io.h>
#include <avr/interrupt.h>

#include "net/net.h"
#include "net/ether.h"
#include "net/interrupt.h"

/*!
 *
 */
int8_t request_irq(irq_handler_t handler) {
    return 0;
}