#ifndef UDP_H
#define UDP_H

#include <stdint.h>

struct udp_hdr_s {
    uint16_t port_src;  // Source port
    uint16_t port_dst;  // Destination port
    uint16_t len;       // Datagram length
    uint16_t chks;      // Checksum
};

void udp_init(void);

#endif  /* !UDP_H */
