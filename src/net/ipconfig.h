#ifndef NET_IPCONFIG_H
#define NET_IPCONFIG_H

#include <avr/pgmspace.h>

#include <stdint.h>

#include "netinet/in.h"

extern in_addr_t my_ip;
extern in_addr_t net_mask;
extern in_addr_t gateway;
extern in_addr_t dns_serv;
extern const uint8_t host_name[] PROGMEM;

int8_t ip_auto_config(void);
int8_t ip_config(const char *ip, const char *nm,
                 const char *gw, const char *dns);

#endif  /* !NET_IPCONFIG_H */
