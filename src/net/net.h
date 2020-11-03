#ifndef NET_NET_H
#define NET_NET_H

#include <stdint.h>

#include <defines.h>

#include "net/ether.h"
#include "net/net_dev.h"
#include "net/socket.h"
#include "netinet/in.h"

#define IN_CLASS_A 0
#define IN_CLASS_A_MASK 0xFF000000  // 255.0.0.0
#define IN_CLASS_B 1
#define IN_CLASS_B_MASK 0xFFFF0000  // 255.255.0.0
#define IN_CLASS_C 2
#define IN_CLASS_C_MASK 0xFFFFFF00  // 255.255.255.0
#define IN_CLASS_D 3    // used for multicast
#define IN_CLASS_E 4    // reserved

/* Packet types */
#define PKT_HOST      0 // to us
#define PKT_BROADCAST 1 // to all
#define PKT_MULTICAST 2 // to group
#define PKT_OTHERHOST 3 // to someone else
#define PKT_LOOPBACK  4 // looped back

/* Check CRC flag */
#define CHECKSUM_NONE           0   // CRC have not yet been verified and this task must be performed by the system software.
#define CHECKSUM_HW             1   // The device has already performed CRC calculation at the hardware level.
#define CHECKSUM_UNNECESSARY    2   // Do not perform any CRC calculations.

/* Hardware Types */
#define HWT_ETHER 1

struct socket;
struct sockaddr;

/**
 * @brief
 * @param bind
 */
struct protocol_ops {
    int8_t (*bind)(struct socket *sk, const struct sockaddr *addr);
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

typedef int8_t (*proto_hdlr_t)(struct net_buff_s *net_buff);

struct net_buff_s *net_buff_alloc(uint16_t size);
struct net_buff_s *ndev_alloc_net_buff(struct net_dev_s *net_dev, uint16_t size);
void *put_net_buff(struct net_buff_s *net_buff, uint16_t len);
void free_net_buff(struct net_buff_s *net_buff);

int8_t net_class_determine(const void *restrict ip, in_addr_t *restrict netmask);

void inet_init(void);
int8_t inet_sock_create(struct socket *sk, uint8_t protocol);

#endif  /* !NET_NET_H */
