#include <stdint.h>
#include <inttypes.h>
#include <string.h>
#include <stdlib.h>

#include <defines.h>

#include "arpa/inet.h"
#include "netinet/in.h"

/*!
 * @brief Determine the Network Class and set the subnet mask
 * @param ip pointer to IP-address in network byte order (big-endian)
 * @param netmask pointer to save subnet mask in network
 *                byte order (big-endian) or NULL
 * @return Number of Network Class; -1 if error
 */
int8_t inet_class_determine(const void *restrict ip, in_addr_t *restrict netmask) {
    int8_t ret = -1;
    uint8_t msb = *(uint8_t *)ip;

    if (!ip)
        return ret;

    if ((msb & 0x80) == 0x00) {
        // Class A
        if (netmask)
            *netmask = htonl(IN_CLASS_A_MASK);
        ret = IN_CLASS_A;
    } else if ((msb & 0xC0) == 0x80) {
        // Class B
        if (netmask)
            *netmask = htonl(IN_CLASS_B_MASK);
        ret = IN_CLASS_B;
    } else if ((msb & 0xE0) == 0xC0) {
        // Class C
        if (netmask)
            *netmask = htonl(IN_CLASS_C_MASK);
        ret = IN_CLASS_C;
    } else if ((msb & 0xF0) == 0xE0) {
        ret = IN_CLASS_D;
        // Class D
        // Net mask is not defined
    } else if ((msb & 0xF0) == 0xF0) {
        ret = IN_CLASS_E;
        // Class E
        // Net mask is not defined
    }

    return ret;
}

/*!
 * @brief Convert IP addr from ASCII-string in \p cp to binary in \p inp
 * @param cp ASCII-string with IP-addr
 * @param inp \c in_addr structure to store result
 * @return 0 on error
 */
int8_t inet_aton(const char *restrict cp, struct in_addr *restrict inp) {

    return 0;
}

/*!
 * @brief Convert the string pointed to by \p cp, in the standard
 *  IPv4 dotted decimal notation, to an integer value suitable
 *  for use as an Internet address.
 * @param cp Pointer to string with host address in numbers-and-dots notation
 * @return Binary IP in network byte order; (in_addr_t)(-1) on error
 */
in_addr_t inet_addr(const char *cp) {
    /** TODO: */
    in_addr_t ip = 0;
    char tmp_str[4] = {0, 0, 0, 0};
    int8_t y;

    for (int8_t i = 0; i < 4; i++) {
        ip <<= 8;
        y = 0;
        memset(tmp_str, 0, 4);
        while ((*cp != 0) && (*cp != '.') && (y < 3)) {
            tmp_str[y] = *cp;
            cp++;
            y++;
        }
        ip |= (atol(tmp_str) & 0xFF);
        if (*cp != 0)
            cp++;
    }

    return htonl(ip);
    // return (in_addr_t)(-1);
}

/*!
 * @brief Convert Internet number in \p in to ASCII representation.
 * @param in Internet host address
 * @return pointer to the network address in Internet standard dot notation
 */
char *inet_ntoa(struct in_addr in) {
    /** TODO: */

    return NULL;
}

/*!
 * @brief Convert a numeric address into a text
 * string suitable for presentation.
 * @param af Address Family (e.g. \a AF_INET)
 * @param src Internet address in binary network format
 * @param dst Buffer to stores the resulting text string; not be \a NULL
 * @param size Length of \p dst
 * @return Pointer to the buffer containing the text string
 * if the conversion succeeds, and \a NULL otherwise, and set
 * \a errno to indicate the error.
 */
const char *inet_ntop(int8_t af, const void *restrict src,
                      char *restrict dst, socklen_t size) {
    /** TODO: */

    return NULL;
}

/*!
 * @brief Сonvert an address in its standard text
 * presentation form into its numeric binary form
 * @param af Address Family (e.g. \a AF_INET)
 * @param src Pointer to the string in the standard presentation form
 * (e.g. "ddd.ddd.ddd.ddd" for IPv4)
 * @param dst Buffer to stores the numeric address; not be \a NULL
 * @return 1 if the conversion succeeds, with the address pointed to by \p dst
 * in network byte order; 0 if the input is not a valid dotted-decimal string;
 * -1 with \a errno set to \a [EAFNOSUPPORT] if the af argument is unknown.
 */
int8_t inet_pton(int8_t af, const char *restrict src, void *restrict dst) {
    /** TODO: */

    return 0;
}
