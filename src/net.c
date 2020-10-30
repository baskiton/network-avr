#include <avr/io.h>
#include <avr/pgmspace.h>

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "net/net.h"
#include "net/ether.h"
#include "net/arp.h"
#include "net/ip.h"
#include "net/tcp.h"
#include "net/udp.h"
#include "net/icmp.h"

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
int8_t net_class_determine(const void *ip, uint32_t *netmask) {
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
 * @brief Convert IP addr from ASCII-string to binary
 * @param ip_str ASCII-string with IP-addr
 * @return Binary IP in Network byte order (big-endian)
 */
uint32_t ip_addr_parse(const char *ip_str) {
    uint32_t ip = 0;
    char tmp_str[4] = {0, 0, 0, 0};
    int8_t y;

    for (int8_t i = 0; i < 4; i++) {
        ip <<= 8;
        y = 0;
        memset(tmp_str, 0, 4);
        while ((*ip_str != 0) && (*ip_str != '.') && (y < 3)) {
            tmp_str[y] = *ip_str;
            ip_str++;
            y++;
        }
        ip |= atol(tmp_str);
        if (*ip_str != 0)
            ip_str++;
    }

    return htonl(ip);
}

/*!
 * @brief Initialize the handlers and others for IPv4
 */
void inet_init(void) {
    arp_init();
    ip_init();
}
