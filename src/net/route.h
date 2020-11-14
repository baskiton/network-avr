#ifndef NET_ROUTE_H
#define NET_ROUTE_H

#include "netinet/in.h"
#include "net/net_dev.h"

/*!
 * @brief Route Table for IPv4
 * @param network Network IP
 * @param net_mask Subnet Mask
 * @param gateway Gateway
 * @param ndev Interface (network device)
 */
struct rtable {
    in_addr_t network;
    in_addr_t net_mask;
    in_addr_t gateway;
    struct net_dev_s *ndev;
};

#endif  /* !NET_ROUTE_H */
