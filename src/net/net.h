#ifndef NET_H
#define NET_H

#include <stdint.h>

#include <defines.h>

#include "ether.h"
#include "net_dev.h"
#include "socket.h"

#define IN_ADDR_ANY ((uint32_t)0x00000000)          // 0.0.0.0
#define IN_ADDR_BROADCAST ((uint32_t)0xFFFFFFFF)    // 255.255.255.255
#define IN_ADDR_NONE ((uint32_t)0xFFFFFFFF)         // 255.255.255.255
#define IN_ADDR_LOOPBACK ((uint32_t)0x7F000001)     // 127.0.0.1

#define htons(x) bswap_16(x)
#define htonl(x) bswap_32(x)
#define htonll(x) bswap_64(x)
#define ntohs(x) bswap_16(x)
#define ntohl(x) bswap_32(x)
#define ntohll(x) bswap_64(x)

struct soket;
struct sock_addr;

/**
 * @brief
 * @param bind
 */
struct protocol_ops {
    int8_t (*bind)(struct soket *sk, const struct sock_addr *addr);
};

/**
 * @brief
 * @param dev Network device
 * @param len Packet length
 * @param transport_header Offset from start frame to Transport Layer header (layer 4)
 * @param network_header Offset from start frame to Network Layer header (layer 3)
 * @param mac_header Offset from start frame to Data Link Layer header (layer 2)
 * @param head Pointer to start of ethernet frame (data link layer - layer 2)
 * @param data Pointer to actual data
 */
struct net_buff_s {
    struct net_dev_s *net_dev;

    uint16_t len;

    uint8_t transport_header;
    uint8_t network_header;
    uint8_t mac_header;

    uint8_t *head;
    uint8_t *data;
};

struct net_buff_s *net_alloc_buff(struct net_dev_s *net_dev, uint16_t size);
void net_free_buff(struct net_buff_s *buff);

#endif  /* !NET_H */
