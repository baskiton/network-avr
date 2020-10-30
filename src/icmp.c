#include <stdint.h>
#include <string.h>

#include "net/net.h"
#include "net/net_dev.h"
#include "net/ip.h"
#include "net/icmp.h"
#include "net/checksum.h"
#include "net/ipconfig.h"
#include "net/ether.h"

/*!
 * @brief Get the ICMP header
 * @param net_buff Pointer to network buffer
 * @return Pointer to ICMP header
 */
static struct icmp_hdr_s *get_icmp_hdr(struct net_buff_s *net_buff) {
    return (void *)(net_buff->head + net_buff->transport_hdr_offset);
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
        */
            return true;
    }
    return false;
}

/*!
 *
 */
static int8_t icmp_echo(struct net_buff_s *nb) {
    struct net_dev_s *ndev = nb->net_dev;
    struct eth_header_s *eth_hdr = (void *)(nb->head + nb->mac_hdr_offset);
    struct ip_hdr_s *iph = get_ip_hdr(nb);
    struct icmp_hdr_s *icmp_h = get_icmp_hdr(nb);

    /** FIXME: used old net buffer to reply */
    nb->pkt_len -= 4;

    memcpy(eth_hdr->mac_dest, eth_hdr->mac_src, ETH_MAC_LEN);
    memcpy(eth_hdr->mac_src, ndev->dev_addr, ETH_MAC_LEN);

    memcpy(&iph->ip_dst, &iph->ip_src, IP4_LEN);
    memcpy(&iph->ip_src, &my_ip, IP4_LEN);
    iph->ttl--;
    iph->hdr_chks = 0;
    iph->hdr_chks = in_checksum(iph, iph->ihl * 4);

    icmp_h->type = ICMP_ECHO_REPLY;
    icmp_h->chks = 0;
    icmp_h->chks = in_checksum(icmp_h, (ntohs(iph->tot_len) - iph->ihl * 4));

    return netdev_start_tx(nb);
}

/*!
 * @brief Handler for ICMP
 */
int8_t icmp_recv(struct net_buff_s *nb) {
    int8_t ret = NETDEV_RX_DROP;
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
            goto out;
        
        default:
            ret = NETDEV_RX_SUCCESS;
            break;
    }

drop:
    free_net_buff(nb);
out:
    return ret;
}

/*!
 * @brief Initialize ICMP handler
 */
void icmp_init(void) {
    ip_proto_handler_add(IP_PROTO_ICMP, icmp_recv);
}

/*!
 *
 */
struct net_buff_s *icmp_create() {

    return NULL;
}

/*!
 * @brief Send an ICMP message
 * @param nb Network Buffer
 * @param type Type of message
 * @param code Subtype
 */
void icmp_send(struct net_buff_s *nb, uint8_t type, uint8_t code) {
    struct ip_hdr_s *iph;
    struct icmp_hdr_s *icmp_h;

    iph = get_ip_hdr(nb);

    /* The following rules are described in RFC1122: 3.2.2 */
    /* Don't reply to multicast/broadcast MAC */
    if (nb->flags.pkt_type != PKT_HOST)
        return;

    /* Don't reply to multicast/broadcast IP */
    if (ip4_is_multicast(&iph->ip_dst) || ip4_is_broadcast(&iph->ip_dst))
        return;

    /* Only reply to fragment 0 */
    if (iph->frag_off & htons(IP_OFFSET))
        return;

    /* Don't reply if source IP is not define a single host */
    if (ip4_is_zero(&iph->ip_src) ||
        ip4_is_loopback(&iph->ip_src) ||
        ip4_is_broadcast(&iph->ip_src) ||
        ip4_is_multicast(&iph->ip_src) ||
        (net_class_determine(&iph->ip_src, NULL) == IN_CLASS_E))
        return;

    icmp_h = get_icmp_hdr(nb);

    /* Check if we send ICMP error message */
    if (!icmp_is_query(type)) {
        /* Don't reply to error and any unknown/not supported ICMP type */
        if (!icmp_is_query(icmp_h->type))
            return;
    }

    /* RFC1812: 4.3.2.5 TOS and Precedence */
    if (icmp_is_query(type)) {}
        // tos = iph->tos;
    else {}
        // tos = (iph->tos & IP_TOS_MASK) | IP_TOS_PREC_INET_CTRL;
    // saddr = iph->ip_dst
}
