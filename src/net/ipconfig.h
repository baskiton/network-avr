#ifndef IPCONFIG_H
#define IPCONFIG_H

#include <stdint.h>

extern uint32_t my_ip;
extern uint32_t net_mask;
extern uint32_t gateway;

int8_t ip_auto_config(void);

#endif  /* !IPCONFIG_H */
