#include <avr/io.h>
#include <avr/pgmspace.h>

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "net/net.h"
#include "net/ether.h"
#include "netinet/arp.h"
#include "netinet/ip.h"
#include "netinet/tcp.h"
#include "netinet/udp.h"
#include "netinet/icmp.h"

/*!
 * @brief Allocate a Network buffer
 * @param size Size to allocate
 * @return Pointer to buffer or \a NULL if error
 */
struct net_buff_s *net_buff_alloc(uint16_t size) {
    struct net_buff_s *buff;
    uint8_t *head;

    buff = malloc(sizeof(struct net_buff_s));
    if (!buff) {
        return NULL;
    }
    
    head = malloc(size);
    if (!head) {
        free(buff);
        return NULL;
    }

    memset(buff, 0, ((void *)&buff->head - (void *)buff));
    
    buff->head = buff->data = buff->tail = head;
    buff->end = buff->tail + size;
    buff->mac_hdr_offset = buff->data - buff->head;
    buff->network_hdr_offset = buff->mac_hdr_offset + ETH_HDR_LEN;
    buff->transport_hdr_offset = 0;

    return buff;
}

/*!
 * @brief Allocate a network buffer for rx on a specific device
 * @param dev Network device to receive
 * @param size Size to allocate (for ethernet is eth header, data, padding, crc)
 * @return Pointer to buffer or NULL if not enough memory
 */
struct net_buff_s *ndev_alloc_net_buff(struct net_dev_s *net_dev, uint16_t size) {
    struct net_buff_s *buff;

    buff = net_buff_alloc(size);
    if (buff)
        buff->net_dev = net_dev;

    return buff;
}

/*!
 * @brief Put a data to the buffer
 * @param net_buff Buffer to adding
 * @param len Length of adding data
 * @return Pointer to tail of buffer
 */
void *put_net_buff(struct net_buff_s *net_buff, uint16_t len) {
    void *old_tail = net_buff->tail;

    net_buff->tail += len;
    net_buff->pkt_len += len;

    if (net_buff->tail > net_buff->end) {
        /** TODO: error - out of range */
        printf_P(PSTR("Out of range!!! Stopping...\n"));
        while (1) {}
        return NULL;
    }
    return old_tail;
}

/*!
 * @brief Free the allocating memory an Net Buffer
 */
void free_net_buff(struct net_buff_s *net_buff) {
    free(net_buff->head);
    free(net_buff);
}

/*!
 * @brief Determine the Network Class and set the subnet mask
 * @param ip pointer to IP-address in network byte order (big-endian)
 * @param netmask pointer to save subnet mask in network
 *                byte order (big-endian) or NULL
 * @return Number of Network Class; -1 if error
 */
int8_t net_class_determine(const void *restrict ip, in_addr_t *restrict netmask) {
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
 * @brief Initialize the handlers and others for IPv4
 */
void inet_init(void) {
    arp_init();
    ip_init();
}

/*!
 * @brief Create INET socket
 * @param sk Pointer to Socket
 * @param protocol Inet Protocol (e.g. IP_PROTO_TCP). If 0 then
 *      sets by default
 * @return 0 on success
 */
int8_t inet_sock_create(struct socket *sk, uint8_t protocol) {
    /** if \p protocol == 0, set default value */
    if (!protocol) {
        switch (sk->type) {
            case SOCK_STREAM:
                protocol = IPPROTO_TCP;
                break;
            case SOCK_DGRAM:
                protocol = IPPROTO_UDP;
                break;
            case SOCK_RAW:
                protocol = IPPROTO_RAW;
                break;
            case SOCK_SEQPACKET:
                protocol = IPPROTO_IP;
                break;
            default:
                // EPROTOTYPE
                return -1;
        }
    }

    return 0;
}
