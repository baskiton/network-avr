#ifndef NETINET_ARP_H
#define NETINET_ARP_H

#include <stdint.h>

#include "net/net.h"
#include "net/ether.h"
#include "netinet/ip.h"

struct __attribute__((packed)) arp_tbl_entry_s {
    in_addr_t ip;
    uint8_t mac[ETH_MAC_LEN];
};

extern struct arp_tbl_entry_s arp_tbl;

struct arp_hdr_s {
    uint16_t htype; // Hardware Type
    uint16_t ptype; // Protocol Type
    uint8_t hlen;   // Hardware Length (for Ethernet is 6 bytes)
    uint8_t plen;   // Protocol Length (for IPv4 is 4 bytes)
    uint16_t oper;  // Operation

    /** FIXME: the following fields are only valid for Ethernet with IPv4 */
    uint8_t sha[ETH_MAC_LEN];   // Sender hardware address (MAC)
    uint8_t spa[IP4_LEN];       // Sender protocol address (IP)
    uint8_t tha[ETH_MAC_LEN];   // Target hardware address (MAC)
    uint8_t tpa[IP4_LEN];       // Target protocol address (IP)
};

/* ARP Operations */
#define ARP_OP_REQ 1    // ARP Request
#define ARP_OP_REPLY 2  // ARP Reply

int8_t arp_lookup(const in_addr_t *next_hop,
                  uint8_t *mac_dest,
                  struct net_dev_s *ndev,
                  const in_addr_t *src_ip);

void arp_init(void);
int8_t arp_send(struct net_dev_s *net_dev,
                uint16_t oper, uint16_t ptype,
                const uint8_t *dest_hw,
                const uint8_t *sha, const in_addr_t *spa,
                const uint8_t *tha, const in_addr_t *tpa);

#endif  /* !NETINET_ARP_H */
