#ifndef NETINET_IN_H
#define NETINET_IN_H

#include <stdint.h>
#include <inttypes.h>

#include "net/socket.h"

typedef uint16_t in_port_t;
typedef uint32_t in_addr_t;

struct in_addr {
    in_addr_t s_addr;
};

#include "arpa/inet.h"

struct sockaddr_in {
    sa_family_t sin_family;     // AF_INET
    in_port_t sin_port;         // Port number
    struct in_addr sin_addr;    // IP address
};

#define IPPROTO_IP      0   // Dummy protocol for TCP
#define IPPROTO_ICMP    1   // Internet Control Message Protocol
#define IPPROTO_TCP     6   // Transmission Control Protocol
#define IPPROTO_UDP     17  // User Datagram Protocol
#define IPPROTO_IPV6    41  // IPv6-in-IPv4 tunnelling
#define IPPROTO_RAW     255 // Raw IP packet

#define INADDR_ANY ((in_addr_t)0x00000000)           // 0.0.0.0
#define INADDR_BROADCAST ((in_addr_t)0xFFFFFFFF)     // 255.255.255.255
#define INADDR_NONE ((in_addr_t)0xFFFFFFFF)          // 255.255.255.255
#define INADDR_LOOPBACK ((in_addr_t)0x7F000001)      // 127.0.0.1

#define INET_ADDRSTRLEN 16  // Length of the string form for IP

#endif  /* !NETINET_IN_H */
