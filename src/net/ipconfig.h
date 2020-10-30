#ifndef IPCONFIG_H
#define IPCONFIG_H

#include <avr/pgmspace.h>

#include <stdint.h>

extern uint32_t my_ip;
extern uint32_t net_mask;
extern uint32_t gateway;
extern uint32_t dns_serv;
extern const uint8_t host_name[] PROGMEM;

int8_t ip_auto_config(void);
int8_t ip_config(const char *ip, const char *nm,
                 const char *gw, const char *dns);

#endif  /* !IPCONFIG_H */
