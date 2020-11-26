#ifndef NET_CHECKSUM_H
#define NET_CHECKSUM_H

#include <stdint.h>

#include "netinet/in.h"
#include "net/socket.h"

uint16_t in_checksum(void *buf, uint16_t len);

uint32_t jenkins_hash_calc(void *data, uint8_t len);

#endif  /* !NET_CHECKSUM_H */
