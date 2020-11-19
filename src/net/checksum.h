#ifndef NET_CHECKSUM_H
#define NET_CHECKSUM_H

#include <stdint.h>

#include "netinet/in.h"
#include "net/socket.h"

uint16_t in_checksum(void *buf, uint16_t len);

uint32_t sock_hash_calc(struct sock_ap_pairs_s *pairs);

#endif  /* !NET_CHECKSUM_H */
