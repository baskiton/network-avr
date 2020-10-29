#ifndef IP_H
#define IP_H

#include <stdint.h>

#include "net.h"

#define IP4_LEN 4
#define IP6_LEN 6

/* IP flags. */
#define IP_CE 0x8000        // congestion (reserved, must be 0)
#define IP_DF 0x4000        // don't fragment
#define IP_MF 0x2000        // more fragments
#define IP_OFFSET 0x1FFF    // fragment offset part

struct ip_hdr_s {
    uint8_t ihl : 4;        // size of header - number of 32-bit words
    uint8_t version : 4;    // protocol version (for IPv4 it's 4)
    uint8_t tos;            // 
    uint16_t tot_len;       // Size of packet
    uint16_t id;            // 
    uint16_t frag_off;      // Fragment offset with flags (in ms 3 bits)
    uint8_t ttl;            // Time To Life
    uint8_t protocol;       // Protocol (TCP, ICMP, UDP... RFC 790)
    uint16_t hdr_chks;      // Header Checksum
    uint32_t ip_src;        // Source IP Address
    uint32_t ip_dst;        // Destination IP Address
    /* options start here.
     * If ihl > 5
     * size: 0-10 * 32bits */
};

void ip_init(void);

int8_t ip_proto_handler(uint8_t proto, struct net_buff_s *net_buff);
void ip_proto_handler_add(uint8_t proto, proto_hdlr_t handler);
void ip_proto_handler_del(uint8_t proto);

#endif  /* !IP_H */
