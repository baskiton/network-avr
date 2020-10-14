#ifndef NET_H
#define NET_H

#include <stdint.h>

#include <defines.h>

#include "ether.h"
#include "net_dev.h"
#include "socket.h"

#define ETH_HDR_ALIGN 2

#define IN_ADDR_ANY ((uint32_t)0x00000000)          // 0.0.0.0
#define IN_ADDR_BROADCAST ((uint32_t)0xFFFFFFFF)    // 255.255.255.255
#define IN_ADDR_NONE ((uint32_t)0xFFFFFFFF)         // 255.255.255.255
#define IN_ADDR_LOOPBACK ((uint32_t)0x7F000001)     // 127.0.0.1

/* Packet types */
#define PKT_HOST      0  // to us
#define PKT_BROADCAST 1  // to all
#define PKT_MULTICAST 2  // to group
#define PKT_OTHERHOST 3  // to someone else

/* Check CRC flag */
#define CHECKSUM_NONE           0   // CRC have not yet been verified and this task must be performed by the system software.
#define CHECKSUM_HW             1   // The device has already performed CRC calculation at the hardware level.
#define CHECKSUM_UNNECESSARY    2   // Do not perform any CRC calculations.

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
 * @param net_dev Network device
 * @param sock Socket for this packet
 * 
 * @param pkt_len Packet length
 * @param data_len Length of data
 * @param protocol Packet protocol
 * 
 * @param pkt_type Packet type
 * @param ip_summed Check CRC
 * 
 * @param transport_hdr_offset Offset from start frame to Transport Layer header (layer 4)
 * @param network_hdr_offset Offset from start frame to Network Layer header (layer 3)
 * @param mac_hdr_offset Offset from start frame to Data Link Layer header (layer 2)
 * 
 * @param head Pointer to start of buffer
 * @param data Pointer to actual data - ethernet frame (data link layer - layer 2)
 * @param tail Tail pointer
 * @param end End pointer
 */
struct net_buff_s {
    struct net_dev_s *net_dev;
    struct socket *sock;

    uint16_t pkt_len;
    uint16_t data_len;
    uint16_t protocol;

    struct {
        uint8_t pkt_type : 2;
        uint8_t ip_summed : 2;
    } flags;

    uint8_t transport_hdr_offset;
    uint8_t network_hdr_offset;
    uint8_t mac_hdr_offset;

    uint8_t *head;
    uint8_t *data;
    uint8_t *tail;
    uint8_t *end;
};

struct net_buff_s *ndev_alloc_net_buff(struct net_dev_s *net_dev, uint16_t size);
void *put_net_buff(struct net_buff_s *net_buff, uint16_t len);
void free_net_buff(struct net_buff_s *net_buff);

#endif  /* !NET_H */
