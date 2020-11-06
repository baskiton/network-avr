#include <stdint.h>

#include "net/checksum.h"

/*!
 * @brief Compute Internet Checksum.
 * RFC 1071: https://tools.ietf.org/html/rfc1071
 * @param buf Buffer with data to calculate
 * @param count Length of data buffer in bytes
 * @return 16-bit checksum
 */
uint16_t in_checksum(void *buf, uint16_t count) {
    uint32_t sum = 0;
    uint16_t *ptr = buf;

    if (count <= 0)
        goto out;

    while (count > 1) {
        sum += *ptr++;
        count -= 2;
    }
    
    /*  Add left-over byte, if any */
    if (count > 0)
        sum += *(uint8_t *)ptr;
    
    /*  Fold 32-bit sum to 16 bits */
    while (sum >> 16)
        sum = (sum & 0xffff) + (sum >> 16);
out:
    return (uint16_t)~sum;
}
