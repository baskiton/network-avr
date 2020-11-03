#ifndef NET_CHECKSUM_H
#define NET_CHECKSUM_H

#include <stdint.h>

uint16_t in_checksum(void *buf, uint16_t len);

#endif  /* !NET_CHECKSUM_H */
