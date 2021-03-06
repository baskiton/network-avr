#include <stdint.h>
#include <string.h>

#include "net/net.h"
#include "net/pkt_handler.h"
#include "net/checksum.h"
#include "net/ipconfig.h"
#include "netinet/ip.h"
#include "netinet/icmp.h"
#include "netinet/tcp.h"
#include "netinet/udp.h"

/*!
 * @brief Get the IP header
 * @param net_buff Pointer to network buffer
 * @return Pointer to IP header
 */
struct ip_hdr_s *get_ip_hdr(struct net_buff_s *net_buff) {
    return (void *)(net_buff->head + net_buff->network_hdr_offset);
}

/*!
 * @brief IP receive main handler
 * @param nb Network buffer
 * @return 0 if success
 */
static int8_t ip_recv(struct net_buff_s *nb) {
    struct ip_hdr_s *iph;

    if (nb->flags.pkt_type == PKT_OTHERHOST)
        goto out;

    iph = get_ip_hdr(nb);

    /* check that version 4 and the length of the header are correct.
     *  else drop
     */
    if ((iph->version != 4) || (iph->ihl < 5) || (iph->ihl > 15))
        goto out;
    
    /* checksum is correct? */
    if (in_checksum(iph, iph->ihl * 4))
        goto out;

    /* check the length of packet */
    if ((nb->pkt_len < ntohs(iph->tot_len)) ||
        (ntohs(iph->tot_len < (iph->ihl * 4))))
        goto out;

    /* set transport header offset */
    nb->transport_hdr_offset = nb->network_hdr_offset + iph->ihl * 4;

    /** and process...
     *  FIXME: at this time IP options is not support
     */
    if (iph->ihl > 5)
        goto out;

    return ip_proto_handler(iph->protocol, nb);

out:
    free_net_buff(nb);

    return NETDEV_RX_DROP;
}

/*!
 * @brief Initialize Internet Protocols
 */
void ip_init(void) {
    icmp_init();
    tcp_init();
    udp_init();

    pkt_hdlr_add(ETH_P_IP, ip_recv);
}

/*!
 * @brief
 * @param sk Socket
 * @param msg Message
 * @param t_hdr_len Length of the transport layer header (TCP or UDP)
 * @param len Length of data + transport header
 */
struct net_buff_s *ip_create_nb(struct socket *sk,
                                struct msghdr *msg,
                                uint8_t t_hdr_len,
                                ssize_t len) {
    struct net_buff_s *nb;
    struct net_dev_s *ndev;
    struct ip_hdr_s *iph;
    uint8_t *data;
    uint16_t pkt_len;

    pkt_len = len + sizeof(*iph) + sizeof(struct eth_header_s);

    /**
     * TODO: when there are several devices (including virtual ones),
     * search for the device and its IP by the destination address.
     * Also need to get the destination MAC (routing).
     */
    ndev = curr_net_dev;
    sk->src_addr = my_ip;

    /** TODO: calculate addr-port hash in socket */

    nb = ndev_alloc_net_buff(ndev, pkt_len);
    if (!nb)
        // ENOBUFS
        return nb;

    nb->protocol = ETH_P_IP;
    nb->tail += ETH_HDR_LEN;
    nb->transport_hdr_offset = nb->network_hdr_offset + sizeof(*iph);

    if (netdev_hdr_create(nb, ndev, ETH_P_IP, ndev->broadcast,
                          ndev->dev_addr, pkt_len))
        goto error;

    /* build IP header */
    iph = put_net_buff(nb, sizeof(*iph));

    iph->version = 4;
    iph->ihl = 5;
    iph->tos = 0;
    iph->tot_len = htons(len + sizeof(*iph));
    iph->id = 0;
    iph->frag_off = htons(IP_DF);
    iph->ttl = 64;
    iph->protocol = sk->protocol;
    iph->ip_src = sk->src_addr;
    iph->ip_dst = sk->dst_addr;
    iph->hdr_chks = 0;
    iph->hdr_chks = in_checksum(iph, iph->ihl * 4);

    data = put_net_buff(nb, len);   // data is transport_header_offset

    data += t_hdr_len;  // now data is the pointer to store message

    /* copy message to buffer */
    memcpy(data, msg->msg_iov->iov_base, msg->msg_iov->iov_len);

    /* add buffer to socket queue */
    nb_enqueue(nb, &sk->nb_tx_q);

    nb->sock = sk;

out:
    return nb;
error:
    free_net_buff(nb);
    nb = NULL;
    goto out;
}

/*!
 * @brief Sending queue over IP
 * @param sk Socket with queue
 */
int8_t ip_send_sock(struct socket *sk) {
    return netdev_queue_xmit(&sk->nb_tx_q);
}
