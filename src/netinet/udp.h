#ifndef NETINET_UDP_H
#define NETINET_UDP_H

#include <stdint.h>

#include "net/socket.h"
#include "netinet/in.h"

struct udp_hdr_s {
    in_port_t port_src; // Source port
    in_port_t port_dst; // Destination port
    uint16_t len;       // Datagram length
    uint16_t chks;      // Checksum
};

void udp_init(void);
ssize_t udp_send_msg(struct socket *restrict sk,
                     struct msghdr *restrict msg);

#endif  /* !NETINET_UDP_H */
