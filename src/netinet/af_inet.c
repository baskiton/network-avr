#include "netinet/arp.h"
#include "netinet/ip.h"
#include "net/socket.h"

/*!
 * @brief Create INET socket
 * @param sk Pointer to Socket
 * @param protocol Inet Protocol (e.g. IP_PROTO_TCP). If 0 then
 *      sets by default
 * @return 0 on success
 */
int8_t inet_sock_create(struct socket *sk, uint8_t protocol) {
    int8_t err = 0;

    sk->state = SS_UNCONNECTED;

    /** if \p protocol == 0, set default value */
    if (!protocol) {
        switch (sk->type) {
            case SOCK_STREAM:
                protocol = IPPROTO_TCP;
                break;
            case SOCK_DGRAM:
                protocol = IPPROTO_UDP;
                break;
            // case SOCK_RAW:
            //     protocol = IPPROTO_RAW;
            //     break;
            // case SOCK_SEQPACKET:
            //     protocol = IPPROTO_IP;
            //     break;
            default:
                // EPROTOTYPE
                err = -1;
                break;
        }
    } else {
        /* checking whether the protocol matches the socket type */
        switch (sk->type) {
            err = -1;   // EPROTONOSUPPORT
            case SOCK_STREAM:
                if (protocol == IPPROTO_TCP)
                    err = 0;
                break;
            case SOCK_DGRAM:
                if ((protocol == IPPROTO_UDP) ||
                    (protocol == IPPROTO_ICMP))
                    err = 0;
                break;
            // case SOCK_RAW:
            //     if (protocol == IPPROTO_RAW)
            //         err = 0;
            //     break;
            // case SOCK_SEQPACKET:
            //     if (protocol == )
            //         err = 0;
            //     break;
            default:
                // EPROTONOSUPPORT
                break;
        }
    }
    if (err)
        return err;

    /** TODO: init data */

    sk->p_ops = NULL;   /** TODO: */
    sk->protocol = protocol;

    /** TODO: protocol init socket */

    return 0;
}

/*!
 * @brief Initialize the handlers and others for IPv4
 */
void inet_init(void) {
    arp_init();
    ip_init();
}
