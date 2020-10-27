#ifndef ARP_H
#define ARP_H

#include <stdint.h>

#include "ether.h"
#include "ip.h"

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

void arp_init(void);

#endif  /* !ARP_H */
