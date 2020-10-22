#ifndef IPCONFIG_H
#define IPCONFIG_H

#include <stdint.h>

#include "net.h"

extern uint32_t my_ip;
extern uint32_t net_mask;
extern uint32_t gateway;
extern uint32_t dns_serv;
extern const uint8_t host_name[] PROGMEM;

int8_t ip_auto_config(void);

#endif  /* !IPCONFIG_H */
