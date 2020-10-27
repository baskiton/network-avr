#include <stdint.h>
#include <string.h>

#include "net/net.h"
#include "net/net_dev.h"
#include "net/arp.h"
#include "net/pkt_handler.h"
#include "net/ipconfig.h"

/*!
 * @brief Get the ARP header
 * @param net_buff Pointer to network buffer
 * @return Pointer to ARP header
 */
static struct arp_hdr_s *get_arp_hdr(struct net_buff_s *net_buff) {
    return (void *)(net_buff->head + net_buff->network_hdr_offset);
}

/*!
 * @brief Process an ARP packet
 * @param net_buff Pointer to network buffer
 * @return 0 if success
 */
static int8_t arp_proc(struct net_buff_s *net_buff) {
    int8_t ret = NETDEV_RX_DROP;
    struct arp_hdr_s *arph = get_arp_hdr(net_buff);
    struct net_dev_s *ndev;
    struct eth_header_s *eth_hdr;

    if ((arph->ptype != htons(ETH_P_IP)) ||
        (arph->htype != htons(HWT_ETHER)))
        goto free_buf;

    /* Only Request and Reply operations available */
    if ((arph->oper != htons(ARP_OP_REQ)) &&
        (arph->oper != htons(ARP_OP_REPLY)))
        goto free_buf;

    /* Check for multicast and loopback target IP */
    if ((net_class_determine(arph->tpa, NULL) == IN_CLASS_D) ||
        ((arph->tpa[0] & 0xF0) == 0x7F))
        goto free_buf;

    /* just in case... */
    if (memcmp(arph->spa, arph->tpa, IP4_LEN))
        goto free_buf;

    /* Send reply if it is a request for us */
    if (arph->oper == htons(ARP_OP_REQ)) {
        /* so far, the same net buffer is used that we received,
            just overwrite the required fields. */
        ndev = net_buff->net_dev;
        eth_hdr = (void *)(net_buff->head + net_buff->mac_hdr_offset);

        /* check if our IP is in the request */
        if (memcmp(arph->tpa, &my_ip, IP4_LEN))
            goto free_buf;

        /* Our IP, transmit the reply */
        arph->oper = htons(ARP_OP_REPLY);

        memcpy(arph->tha, arph->sha, ETH_MAC_LEN);
        memcpy(arph->tpa, arph->spa, IP4_LEN);

        memcpy(arph->sha, ndev->dev_addr, ETH_MAC_LEN);
        memcpy(arph->spa, &my_ip, IP4_LEN);

        memcpy(eth_hdr->mac_dest, eth_hdr->mac_src, ETH_MAC_LEN);
        memcpy(eth_hdr->mac_src, ndev->dev_addr, ETH_MAC_LEN);

        /* and finally, begin transmission... */
        netdev_start_tx(net_buff);

        ret = NETDEV_RX_SUCCESS;
        goto out;
    }

    /** TODO: Implement processing of responses and
     *  creation of independent ARP packets.
     */

free_buf:
    free_net_buff(net_buff);
out:
    return ret;
}

/*!
 * @brief ARP receive handler
 * @param net_buff Network buffer
 * @return 0 if success
 */
static int8_t arp_recv(struct net_buff_s *net_buff) {
    int8_t ret = NETDEV_RX_DROP;
    const struct arp_hdr_s *arph;

    if ((net_buff->flags.pkt_type == PKT_OTHERHOST) ||
        (net_buff->flags.pkt_type == PKT_LOOPBACK)) {
        ret = NETDEV_RX_SUCCESS;
        goto out;
    }

    arph = get_arp_hdr(net_buff);

    if ((arph->hlen == ETH_MAC_LEN) || (arph->plen == IP4_LEN))
        return arp_proc(net_buff);

out:
    free_net_buff(net_buff);

    return ret;
}

/*!
 * @brief Initial the ARP protocol and set the handler for him
 */
void arp_init(void) {
    pkt_hdlr_add(ETH_P_ARP, arp_recv);
}