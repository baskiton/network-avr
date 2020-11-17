#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#include "net/net.h"
#include "net/net_dev.h"
#include "net/checksum.h"
#include "net/ipconfig.h"
#include "net/ether.h"
#include "netinet/ip.h"
#include "netinet/icmp.h"

/*!
 * @brief Get the ICMP header
 * @param net_buff Pointer to network buffer
 * @return Pointer to ICMP header
 */
struct icmp_hdr_s *get_icmp_hdr(struct net_buff_s *net_buff) {
    return (void *)(net_buff->head + net_buff->transport_hdr_offset);
}

/*!
 *
 */
static bool icmp_enque_to_socket(struct net_buff_s *nb) {
    struct socket *sk;
    struct ip_hdr_s *iph = get_ip_hdr(nb);
    struct sock_ap_pairs_s pairs = {
        .my_addr = iph->ip_dst,
        .fe_addr = iph->ip_src,
    };

    sk = socket_find(&pairs, iph->protocol);
    if (!sk)
        return false;

    nb_enqueue(nb, &sk->nb_rx_q);

    return true;
}

/*!
 * @brief ICMP is query message type
 */
static inline bool icmp_is_query(uint8_t type) {
    switch (type) {
        case ICMP_ECHO_REPLY:
        case ICMP_ECHO_REQ:
        /** TODO: Not supported yet
        case ICMP_TIMESTAMP_REQ:
        case ICMP_TIMESTAMP_REPLY:
        case ICMP_INFO_REQ:
        case ICMP_INFO_REPLY:
        case ICMP_ADDRESS_REQ:
        case ICMP_ADDRESS_REPLY:
        // */
            return true;
        default:
            return false;
    }
}

/*!
 * @brief Reply to Echo Request (ping)
 */
static bool icmp_echo(struct net_buff_s *nb) {
    struct net_dev_s *ndev = nb->net_dev;
    struct eth_header_s *eth_hdr = (void *)(nb->head + nb->mac_hdr_offset);
    struct ip_hdr_s *iph = get_ip_hdr(nb);
    struct icmp_hdr_s *icmp_h = get_icmp_hdr(nb);

    /* drop if IP not ours */
    if (iph->ip_dst != my_ip)
        return NETDEV_RX_DROP;

    /** FIXME: used old net buffer to reply */
    nb->pkt_len -= 4;

    memcpy(eth_hdr->mac_dest, eth_hdr->mac_src, ETH_MAC_LEN);
    memcpy(eth_hdr->mac_src, ndev->dev_addr, ETH_MAC_LEN);

    memcpy(&iph->ip_dst, &iph->ip_src, IP4_LEN);
    memcpy(&iph->ip_src, &my_ip, IP4_LEN);
    iph->ttl = 64;
    iph->hdr_chks = 0;
    iph->hdr_chks = in_checksum(iph, iph->ihl * 4);

    icmp_h->type = ICMP_ECHO_REPLY;
    icmp_h->chks = 0;
    icmp_h->chks = in_checksum(icmp_h, (ntohs(iph->tot_len) - iph->ihl * 4));

    netdev_list_xmit(nb);

    return true;
}

/*!
 * @brief Handler for ICMP
 */
static int8_t icmp_recv(struct net_buff_s *nb) {
    bool ret = false;
    struct ip_hdr_s *iph = get_ip_hdr(nb);
    struct icmp_hdr_s *icmp_h = get_icmp_hdr(nb);

    /* drop if type is unknown */
    if (icmp_h->type > 18)
        goto drop;

    /* drop if dest addr is multicast/broadcast */
    if (ip4_is_multicast(&iph->ip_dst) || ip4_is_broadcast(&iph->ip_dst))
        goto drop;

    /* drop if invalid checksum */
    if (in_checksum(icmp_h, (ntohs(iph->tot_len) - iph->ihl * 4)))
        goto drop;

    /* handlers of the specified ICMP types */
    switch (icmp_h->type) {
        case ICMP_ECHO_REQ:
            ret = icmp_echo(nb);
            break;
        
        case ICMP_ECHO_REPLY:
            ret = icmp_enque_to_socket(nb);
            break;
        
        default:
            goto drop;
    }
    if (ret)
        return NETDEV_RX_SUCCESS;

drop:
    free_net_buff(nb);

    return NETDEV_RX_DROP;
}

/*!
 * @brief Initialize ICMP handler
 */
void icmp_init(void) {
    ip_proto_handler_add(IPPROTO_ICMP, icmp_recv);
}

/*!
 * @brief Create ICMP header
 * @param net_dev Network device
 * @param type Type of ICMP
 * @param code Subtype
 * @param hdr_data Rest of Header
 * @param data Pointer to additional data
 * @param data_len Size of \p data
 * @return 0 if success
 */
int8_t icmp_hdr_create(struct net_buff_s *nb,
                       uint8_t type, uint8_t code,
                       uint32_t hdr_data, const void *data,
                       uint16_t data_len) {
    struct icmp_hdr_s *icmp_h = get_icmp_hdr(nb);

    icmp_h->type = type;
    icmp_h->code = code;
    icmp_h->hdr_data.data = htonl(hdr_data);
    memcpy(icmp_h + (sizeof(struct icmp_hdr_s)), data, data_len);
    icmp_h->chks = 0;
    icmp_h->chks = in_checksum(icmp_h, sizeof(struct icmp_hdr_s) + data_len);
    if (in_checksum(icmp_h, sizeof(struct icmp_hdr_s) + data_len))
        return -1;

    return 0;
}
