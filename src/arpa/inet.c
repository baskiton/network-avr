#include <stdint.h>
#include <inttypes.h>

#include <defines.h>

#include "arpa/inet.h"
#include "netinet/in.h"

/*!
 * @brief Convert the string pointed to by \p cp, in the standard
 *  IPv4 dotted decimal notation, to an integer value suitable
 *  for use as an Internet address.
 * @param cp Pointer to string with host address in numbers-and-dots notation
 * @return Binary data in network byte order; (in_addr_t)(-1) on error
 */
in_addr_t inet_addr(const char *cp) {
    /** TODO: */

    return (in_addr_t)(-1);
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
const char *inet_ntop(int af, const void *restrict src,
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
int8_t inet_pton(int af, const char *restrict src, void *restrict dst) {
    /** TODO: */

    return 0;
}
