#ifndef NETINET_UDP_H
#define NETINET_UDP_H

#include <stdint.h>

#include "netinet/in.h"

struct udp_hdr_s {
    in_port_t port_src; // Source port
    in_port_t port_dst; // Destination port
    uint16_t len;       // Datagram length
    uint16_t chks;      // Checksum
};

void udp_init(void);

#endif  /* !NETINET_UDP_H */
