#ifndef UDP_H
#define UDP_H

#include <stdint.h>

struct udp_hdr_s {
    uint16_t port_src;
    uint16_t port_dst;
    uint16_t len;
    uint16_t chks;
};

#endif  /* !UDP_H */
