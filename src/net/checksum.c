#include <stdint.h>

#include "net/checksum.h"
#include "net/socket.h"
#include "netinet/in.h"

/*!
 * @brief Compute Internet Checksum.
 * RFC 1071: https://tools.ietf.org/html/rfc1071
 * @param buf Buffer with data to calculate
 * @param len Length of data buffer in bytes
 * @return 16-bit checksum
 */
uint16_t in_checksum(void *buf, uint16_t len) {
    uint32_t sum = 0;
    uint16_t *ptr = buf;

    if (len <= 0)
        goto out;

    while (len > 1) {
        sum += *ptr++;
        len -= 2;
    }
    
    /*  Add left-over byte, if any */
    if (len > 0)
        sum += *(uint8_t *)ptr;
    
    /*  Fold 32-bit sum to 16 bits */
    while (sum >> 16)
        sum = (sum & 0xffff) + (sum >> 16);
out:
    return (uint16_t)~sum;
}

/*!
 * @brief Calculate Hash for socket port-addr pairs
 * @param pairs Port-Addr pairs
 * @return 32-bit hash sum
 */
uint32_t sock_hash_calc(struct sock_ap_pairs_s *pairs) {
    uint8_t *key = (void *)pairs;
    uint32_t hash;
    uint8_t i;

    /* Jenkins one-at-a-time hash */
    for (hash = i = 0; i < sizeof(*pairs); i++) {
        hash += key[i];
        hash += (hash << 10);
        hash ^= (hash >> 6);
    }
    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);

    return hash;
}
