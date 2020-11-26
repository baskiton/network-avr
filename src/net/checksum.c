#include <stdint.h>

#include "net/checksum.h"
#include "net/socket.h"

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
 * @brief Calculate Hash Sum
 * @param data Data buffer
 * @param len Length of data
 * @return 32-bit hash sum
 */
uint32_t jenkins_hash_calc(void *data, uint8_t len) {
    uint32_t hash;
    uint8_t i;

    /* Jenkins one-at-a-time hash */
    for (hash = i = 0; i < len; i++) {
        hash += ((uint8_t *)data)[i];
        hash += (hash << 10);
        hash ^= (hash >> 6);
    }
    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);

    return hash;
}
