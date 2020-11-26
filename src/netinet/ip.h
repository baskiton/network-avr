#ifndef NETINET_IP_H
#define NETINET_IP_H

#include <stdint.h>
#include <stdbool.h>

#include "net/net.h"
#include "netinet/in.h"
#include "arpa/inet.h"

#define IP4_LEN 4
#define IP6_LEN 6

/* IP flags. */
#define IP_CE 0x8000        // congestion (reserved, must be 0)
#define IP_DF 0x4000        // don't fragment
#define IP_MF 0x2000        // more fragments
#define IP_OFFSET 0x1FFF    // fragment offset part

/* IP TOS */
#define IP_TOS_MASK 0x1E
#define IP_TOS_LOW_DELAY 0x10   // Low Delay
#define IP_TOS_HIGH_THRPUT 0x08 // High Throughput
#define IP_TOS_HIGH_RELIAB 0x04 // High Reliability
#define IP_TOS_LOWCOST 0x02     // Minimize Monetary Cost

#define IP_TOS_PREC_NET_CTRL 0xE0   // Network Control
#define IP_TOS_PREC_INET_CTRL 0xC0  // Internetwork Control
#define IP_TOS_PREC_CRIT_ECP 0xA0   // Critic/ECP
#define IP_TOS_PREC_FLASH_OVR 0x80  // Flash Override
#define IP_TOS_PREC_FLASH 0x60      // Flash
#define IP_TOS_PREC_IMMED 0x40      // Immediate
#define IP_TOS_PREC_PRIOR 0x20      // Priority
#define IP_TOS_PREC_ROUTINE 0x00    // Routine

/* IP TOS for difference protocols */
#define IP_TOS_TELNET 0x10      // minimize delay. Includes all interactive user protocols (e.g., rlogin)
#define IP_TOS_FTP_CTRL 0x10    // minimize delay
#define IP_TOS_FTP_DATA 0x08    // maximize throughput. Includes all bulk data transfer protocols (e.g., rcp)
#define IP_TOS_TFTP 0x10        // minimize delay
#define IP_TOS_SMTP_CMD 0x10    // minimize delay
#define IP_TOS_SMTP_DATA 0x08   // maximize throughput
#define IP_TOS_DNS_UDP 0x10     // minimize delay
#define IP_TOS_DNS_TCP 0x00
#define IP_TOS_DNS_ZONE 0x08    // maximize throughput
#define IP_TOS_NNTP 0x02        // minimize monetary cost
#define IP_TOS_ICMP_ERR 0x00
#define IP_TOS_ICMP_REQ 0x00
#define IP_TOS_ICMP_RESP IP_TOS_ICMP_REQ    // same as request
#define IP_TOS_IGP 0x04         // maximize reliability
#define IP_TOS_EGP 0x00
#define IP_TOS_SNMP 0x04        // maximize reliability
#define IP_TOS_BOOTP 0x00

struct ip_hdr_s {
    uint8_t ihl : 4;        // size of header - number of 32-bit words
    uint8_t version : 4;    // protocol version (for IPv4 it's 4)
    uint8_t tos;            // 
    uint16_t tot_len;       // Size of packet
    uint16_t id;            // 
    uint16_t frag_off;      // Fragment offset with flags (in ms 3 bits)
    uint8_t ttl;            // Time To Life
    uint8_t protocol;       // Protocol (TCP, ICMP, UDP... RFC 790, e.g. IPPROTO_TCP)
    uint16_t hdr_chks;      // Header Checksum
    in_addr_t ip_src;       // Source IP Address
    in_addr_t ip_dst;       // Destination IP Address
    /* options start here.
     * If ihl > 5
     * size: 0-10 * 32bits */
};

/*!
 * @brief Check that the destination address is on the same subnet
 * @param my Our IP
 * @param dest Destination IP
 * @param mask Subnet Mask
 * @return True if dest in same subnet
 */
static inline bool ip4_check_same_subnet(in_addr_t my, in_addr_t dest, in_addr_t mask) {
    return ((my ^ dest) & mask) ? false : true;
}

inline bool ip4_is_broadcast(const void *ip) {
    return (*(in_addr_t *)ip == htonl(INADDR_BROADCAST));
}

inline bool ip4_is_multicast(const void *ip) {
    return ((*(uint8_t *)ip & 0xF0) == 0xE0);
}

inline bool ip4_is_loopback(const void *ip) {
    return ((*(uint8_t *)ip & 0xFF) == 0x7F);
}

inline bool ip4_is_zero(const void *ip) {
    return (*(in_addr_t *)ip == 0);
}

struct ip_hdr_s *get_ip_hdr(struct net_buff_s *net_buff);
void ip_init(void);
struct net_buff_s *ip_create_nb(struct socket *sk,
                                struct msghdr *msg,
                                uint8_t t_hdr_len,
                                size_t len,
                                struct sockaddr_in *daddr);
int8_t ip_send_sock(struct socket *sk);

int8_t ip_proto_handler(struct net_buff_s *net_buff);
void ip_proto_handler_add(uint8_t proto, proto_hdlr_t handler);
void ip_proto_handler_del(uint8_t proto);

#endif  /* !NETINET_IP_H */
