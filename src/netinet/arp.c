#include <stdint.h>
#include <string.h>

#include "net/net.h"
#include "net/net_dev.h"
#include "net/ether.h"
#include "net/pkt_handler.h"
#include "net/ipconfig.h"
#include "arpa/inet.h"
#include "netinet/arp.h"
#include "netinet/ip.h"

struct arp_tbl_entry_s arp_tbl;

/*!
 * @brief Search the IP and MAC address to the ARP table
 * @param ip IP address to set
 * @param mac MAC address to set
 */
void arp_tbl_set(uint8_t *restrict ip, uint8_t *restrict mac) {
    if (memcmp(&arp_tbl.ip, ip, IP4_LEN))
        memcpy(&arp_tbl.ip, ip, IP4_LEN);

    memcpy(arp_tbl.mac, mac, ETH_MAC_LEN);
}

/*!
 * @brief Search the IP address in the ARP table
 *  and return the corresponding MAC address
 * @param ip IP address to search
 * @return MAC address or NULL if there is no entry
 */
uint8_t *arp_tbl_get(uint8_t *ip) {
    if (!memcmp(&arp_tbl.ip, ip, IP4_LEN))
        return arp_tbl.mac;
    return NULL;
}

/*!
 * @brief Get the ARP header
 * @param net_buff Pointer to network buffer
 * @return Pointer to ARP header
 */
static struct arp_hdr_s *get_arp_hdr(struct net_buff_s *net_buff) {
    return (void *)(net_buff->head + net_buff->network_hdr_offset);
}

static inline int8_t arp_xmit(struct net_buff_s *net_buff) {
    return netdev_queue_xmit(net_buff);
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
    if ((inet_class_determine(arph->tpa, NULL) == IN_CLASS_D) ||
        ((arph->tpa[0] & 0xF0) == 0x7F))
        goto free_buf;

    /* just in case... */
    if (!memcmp(arph->spa, arph->tpa, IP4_LEN))
        goto free_buf;

    /* check if the packet is for us */
    if (memcmp(arph->tpa, &my_ip, IP4_LEN))
        goto free_buf;

    /* Send reply if it is a REQUEST for us */
    if (arph->oper == htons(ARP_OP_REQ)) {
        /** TODO: so far, the same net buffer is used that
         * we received, just overwrite the required fields.
         */
        net_buff->pkt_len -= 4;
        ndev = net_buff->net_dev;
        eth_hdr = (void *)(net_buff->head + net_buff->mac_hdr_offset);

        /* transmit the reply */
        /* set fields */
        arph->oper = htons(ARP_OP_REPLY);

        memcpy(arph->tha, arph->sha, ETH_MAC_LEN);
        memcpy(arph->tpa, arph->spa, IP4_LEN);

        memcpy(arph->sha, ndev->dev_addr, ETH_MAC_LEN);
        memcpy(arph->spa, &my_ip, IP4_LEN);

        memcpy(eth_hdr->mac_dest, eth_hdr->mac_src, ETH_MAC_LEN);
        memcpy(eth_hdr->mac_src, ndev->dev_addr, ETH_MAC_LEN);

        /* and finally, begin transmission... */
        arp_xmit(net_buff);

        ret = NETDEV_RX_SUCCESS;
        goto out;
    }

    /* If it is a REPLY, update ARP table */
    arp_tbl_set(arph->spa, arph->sha);
    ret = NETDEV_RX_SUCCESS;

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
    memset(&arp_tbl, 0, sizeof(arp_tbl));
    pkt_hdlr_add(ETH_P_ARP, arp_recv);
}

/*!
 * @brief Create the ARP packet
 * @param net_dev Network device
 * @param oper Operation Type (reply or request)
 * @param ptype Protocol Type (for ARP header, e.g. ETH_P_IP)
 * @param dest_hw Destination MAC (for Link layer header (ethernet)).
 *                  If \a NULL, it set as broadcast.
 * @param sha Source MAC (migth be \a NULL)
 * @param spa Source IP (migth be \a NULL)
 * @param tha Target MAC (migth be \a NULL)
 * @param tpa Target IP
 * @return Pointer to new buffer with ARP packet
 */
struct net_buff_s *arp_create(struct net_dev_s *net_dev,
                              uint16_t oper, uint16_t ptype,
                              const uint8_t *dest_hw,
                              const uint8_t *sha, const uint8_t *spa,
                              const uint8_t *tha, const uint8_t *tpa) {
    struct net_buff_s *net_buff;
    struct arp_hdr_s *arph;

    net_buff = ndev_alloc_net_buff(net_dev,
                                   (sizeof(struct eth_header_s) +
                                    sizeof(struct arp_hdr_s)));
    if (!net_buff)
        return NULL;

    if (!dest_hw)
        dest_hw = net_dev->broadcast;
    if (!sha)
        sha = net_dev->dev_addr;
    if (!spa)
        spa = (void *)&my_ip;

    net_buff->protocol = htons(ETH_P_ARP);
    net_buff->tail += ETH_HDR_LEN;
    if (netdev_hdr_create(net_buff, net_dev, ETH_P_ARP,
                          dest_hw, sha, net_buff->pkt_len))
        goto out;

    arph = put_net_buff(net_buff, sizeof(struct arp_hdr_s));
    arph->htype = htons(1); // Hardware type is Ethernet
    arph->ptype = htons(ptype);
    arph->hlen = ETH_MAC_LEN;
    arph->plen = IP4_LEN;
    arph->oper = htons(oper);
    memcpy(arph->sha, sha, ETH_MAC_LEN);
    memcpy(arph->spa, spa, IP4_LEN);
    if (tha)
        memcpy(arph->tha, tha, ETH_MAC_LEN);
    else
        memset(arph->tha, 0, ETH_MAC_LEN);
    memcpy(arph->tpa, tpa, IP4_LEN);

    return net_buff;

out:
    free_net_buff(net_buff);
    return NULL;
}

/*!
 * @brief Send ARP packet
 * @param net_dev Network device
 * @param oper Operation Type (reply or request)
 * @param ptype Protocol Type (for ARP header, e.g. IPv4)
 * @param dest_hw Destination MAC (for Link layer header (ethernet)).
 *                  If \a NULL, it set as broadcast.
 * @param sha Source MAC (migth be \a NULL)
 * @param spa Source IP
 * @param tha Target MAC (migth be \a NULL)
 * @param tpa Target IP
 */
int8_t arp_send(struct net_dev_s *net_dev,
                 uint16_t oper, uint16_t ptype,
                 const uint8_t *dest_hw,
                 const uint8_t *sha, const uint8_t *spa,
                 const uint8_t *tha, const uint8_t *tpa) {
    struct net_buff_s *nb;

    nb = arp_create(net_dev, oper, ptype, dest_hw, sha, spa, tha, tpa);
    if (!nb) {
        printf_P(PSTR("arp_send(): failed to creating buffer\n"));
        return -1;
    }

    return arp_xmit(nb);
}
