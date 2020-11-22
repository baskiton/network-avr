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

/*!
 * @brief Get the UDP header
 * @param nb Pointer to network buffer
 * @return Pointer to UDP header
 */
static inline struct udp_hdr_s *get_udp_hdr(struct net_buff_s *nb) {
    return (void *)(nb->head + nb->transport_hdr_offset);
}

ssize_t udp_send_msg(struct socket *restrict sk,
                     struct msghdr *restrict msg);
ssize_t udp_recv_msg(struct socket *restrict sk,
                     struct msghdr *restrict msg,
                     int8_t flags, socklen_t *restrict addr_len);

#endif  /* !NETINET_UDP_H */
