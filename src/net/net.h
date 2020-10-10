#ifndef NET_H
#define NET_H

#include <stdint.h>

#include <defines.h>

#include "net/ether.h"
#include "net/net_dev.h"
#include "net/socket.h"

#define htons(x) bswap_16(x)
#define htonl(x) bswap_32(x)
#define htonll(x) bswap_64(x)
#define ntohs(x) bswap_16(x)
#define ntohl(x) bswap_32(x)
#define ntohll(x) bswap_64(x)

/**
 * @brief
 * @param bind
 * 
 * TODO: разобраться, почему здесь не видны типы из хедера 'socket.h'
 */
struct protocol_ops {
    int8_t (*bind)(void *sk, const void *addr);
    // int8_t (*bind)(sk_t *sk, const struct sock_addr *addr);
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
