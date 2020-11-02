#ifndef ARPA_INET_H
#define ARPA_INET_H

#include <stdint.h>
#include <inttypes.h>

#include <defines.h>

#include "netinet/in.h"

#define htons(x) bswap_16(x)    // Converted value from host to network byte order
#define htonl(x) bswap_32(x)    // Converted value from host to network byte order
#define htonll(x) bswap_64(x)   // Converted value from host to network byte order
#define ntohs(x) bswap_16(x)    // Converted value from network to host byte order
#define ntohl(x) bswap_32(x)    // Converted value from network to host byte order
#define ntohll(x) bswap_64(x)   // Converted value from network to host byte order

in_addr_t inet_addr(const char *cp);
char *inet_ntoa(struct in_addr in);
const char *inet_ntop(int af, const void *restrict src,
                      char *restrict dst, socklen_t size);
int8_t inet_pton(int af, const char *restrict src, void *restrict dst);

#endif  /* !ARPA_INET_H */
