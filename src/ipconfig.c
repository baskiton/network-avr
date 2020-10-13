#include <util/delay.h>
#include <avr/pgmspace.h>

#include <stdint.h>
#include <stdio.h>

#include "net/net.h"
#include "net/net_dev.h"
#include "net/ipconfig.h"

uint32_t my_ip = htonl(IN_ADDR_NONE);       // My IP Address
uint32_t net_mask = htonl(IN_ADDR_NONE);    // Netmask for local subnet
uint32_t gateway = htonl(IN_ADDR_NONE);     // Gateway IP Address

/*!
 * @brief Auto configuring the IP address with DHCP
 * for the currently active network device
 * @return 0 if success; errno if error
 */
int8_t ip_auto_config(void) {
    int8_t err = 0;

    if (!curr_net_dev) {
        // no device - error
        // ENODEV
        printf_P(PSTR("Error: IP Config: No Network Device\n"));
        return -1;
    }

    // opening net device...
    // in this operation, if successful, the up_state flag is set
    err = netdev_open(curr_net_dev);
    if (err) {
        printf_P(PSTR("Error: IP Config: Failed to open\n"));
        return err;
    }

    // loop until link status UP
    while (!curr_net_dev->flags.link_status) {
        /** BUG: for some reason does not work without delay */
        _delay_ms(0);
    }
    

    /** TODO: DHCP requesting */

    return err;
}
