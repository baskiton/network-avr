#include <stdint.h>
#include <string.h>
#include <ctype.h>

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
    /** TODO: */

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
    in_addr_t ip;

    if (inet_pton(AF_INET, cp, &ip) <= 0)
        ip = (in_addr_t)(-1);

    return ip;
}

/*!
 * @brief Convert Internet number in \p in to ASCII representation.
 * @param in Internet host address
 * @return pointer to the network address in Internet standard dot notation
 */
char *inet_ntoa(struct in_addr in) {
    /** TODO: */
    static char tmp[16];

    memset(tmp, 0, 16);

    if (!inet_ntop(AF_INET, &in, tmp, 16))
        return NULL;

    return tmp;
}

/*!
 * @brief Convert a numeric address into a text
 * string suitable for presentation.
 * @param af Address Family (e.g. \a AF_INET)
 * @param src Internet address in binary network format
 * @param dst Buffer to stores the resulting text string; not be \a NULL
 * @param size Length of \p dst
 * @return Pointer to the buffer containing the text string (dst)
 * if the conversion succeeds, and \a NULL otherwise, and set
 * \a errno to indicate the error.
 */
const char *inet_ntop(int8_t af, const void *restrict src,
                      char *restrict dst, socklen_t size) {
    if (af == AF_INET) {
        const uint8_t *ptr = src;
        const char *fmt = "%u.%u.%u.%u";
        char tmp[16];
        int len;

        len = sprintf(tmp, fmt, ptr[0], ptr[1], ptr[2], ptr[3]);
        if ((len <= 0) || (len >= size)) {
            // ENOSPC
            return NULL;
        }
        return strcpy(dst, tmp);
    }

    // EAFNOSUPPORT
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
    // const char *end = src + strlen(src);
    char tmp[4], *tmp_p, ch;
    uint8_t octets, flag;

    if (af != AF_INET)
        // EAFNOSUPPORT
        return -1;

    octets = flag = 0;
    *(tmp_p = tmp) = 0;

    // while (src < end) {
    while ((ch = *src++)) {
        // ch = *src++;
        if (isdigit(ch)) {
            uint16_t new = *tmp_p * 10 + (ch - '0');

            if ((flag && !(*tmp_p)) || (new > 255))
                return 0;

            *tmp_p = new;

            if (!flag) {
                if (++octets > 4)
                    return 0;
                flag = 1;
            }
        } else if (ch == '.' && flag) {
            if (octets == 4)
                return 0;
            *++tmp_p = 0;
            flag = 0;
        } else
            return 0;
    }

    if (octets < 4)
        return 0;

    memcpy(dst, tmp, sizeof(tmp));

    return 1;
}
