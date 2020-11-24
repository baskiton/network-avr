#include <stdint.h>
#include <string.h>

#include "net/net.h"
#include "net/pkt_handler.h"
#include "net/checksum.h"
#include "net/ipconfig.h"
#include "net/socket.h"
#include "netinet/in.h"
#include "netinet/ip.h"
#include "netinet/icmp.h"
#include "netinet/tcp.h"
#include "netinet/udp.h"
#include "netinet/arp.h"

/*!
 * @brief Get the IP header
 * @param net_buff Pointer to network buffer
 * @return Pointer to IP header
 */
struct ip_hdr_s *get_ip_hdr(struct net_buff_s *net_buff) {
    return (void *)(net_buff->head + net_buff->network_hdr_offset);
}

/*!
 * @brief Common handler for IP (except ICMP)
 */
static int8_t ip_common_recv(struct net_buff_s *nb) {
    struct socket *sk;
    struct ip_hdr_s *iph = get_ip_hdr(nb);
    struct sock_ap_pairs_s pairs = {
        .loc_addr = iph->ip_dst,
        .fe_addr = iph->ip_src,
    };

    switch (iph->protocol) {
        case IPPROTO_TCP:
            pairs.loc_port = get_tcp_hdr(nb)->port_dst;
            pairs.fe_port =  get_tcp_hdr(nb)->port_src;
            pairs.proto = IPPROTO_TCP;
            break;
        
        case IPPROTO_UDP:
            pairs.loc_port =  get_udp_hdr(nb)->port_dst;
            pairs.fe_port =  get_udp_hdr(nb)->port_src;
            pairs.proto = IPPROTO_UDP;
            break;
        
        default:    // unreach in theory
            goto drop;
    }

    sk = socket_find(&pairs);
    if (sk) {
        nb_enqueue(nb, &sk->nb_rx_q);
        return NETDEV_RX_SUCCESS;
    }

    socket_list_for_each(sk, iph->protocol) {
        if (sk->src_port != pairs.loc_port)
            continue;
        if (sk->src_addr != iph->ip_dst)
            continue;
        if (sk->dst_addr){
            if (sk->dst_addr != iph->ip_src)
                /** TODO: check if addr is 0.0.0.0 or bcast */
                continue;
        }
        if (sk->dst_port) {
            if (sk->dst_port != pairs.fe_port)
                continue;
        }
    }

drop:
    free_net_buff(nb);

    return NETDEV_RX_DROP;
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

    return ip_proto_handler(nb);

out:
    free_net_buff(nb);

    return NETDEV_RX_DROP;
}

/*!
 * @brief Initialize Internet Protocols
 */
void ip_init(void) {
    icmp_init();

    ip_proto_handler_add(IPPROTO_IP, ip_common_recv);

    pkt_hdlr_add(ETH_P_IP, ip_recv);
}

/*!
 * @brief Create net Buffer for IPv4
 * @param sk Socket
 * @param msg Message
 * @param t_hdr_len Length of the transport layer header (TCP or UDP)
 * @param len Length of data + transport header
 */
struct net_buff_s *ip_create_nb(struct socket *sk,
                                struct msghdr *msg,
                                uint8_t t_hdr_len,
                                size_t len) {
    struct net_buff_s *nb;
    struct net_dev_s *ndev;
    struct ip_hdr_s *iph;
    uint8_t *data;
    size_t pkt_len;
    in_addr_t next_hop;
    uint8_t mac_dest[6];

    pkt_len = len + sizeof(*iph) + sizeof(struct eth_header_s);

    /**
     * TODO: when there are several devices (including virtual ones),
     * search for the device and its IP by the destination address.
     */
    ndev = curr_net_dev;
    sk->src_addr = my_ip;

    /* If set, then the address has been changed
     * and need to update the hash sum. */
    if (msg->msg_name)
        socket_set_hash(sk);

    /* create net buffer */
    nb = ndev_alloc_net_buff(ndev, pkt_len);
    if (!nb)
        // ENOBUFS
        return nb;

    nb->protocol = ETH_P_IP;
    nb->tail += ETH_HDR_LEN;
    nb->transport_hdr_offset = nb->network_hdr_offset + sizeof(*iph);

    /* Get the destination MAC */
    /* check that dest IP in the same subnet */
    if (ip4_check_same_subnet(sk->src_addr, sk->dst_addr, net_mask))
        // if same, next-hop = dest IP
        next_hop = sk->dst_addr;
    // else if (ip4_is_loopback(sk->dst_addr))
    /** TODO: if dest addr is loopback... ? */
    //     next_hop = gateway;
    else {
        // if not same and not loopback, next-hop = default gateway
        if (gateway == htonl(INADDR_BROADCAST))
            // drop if gateway is not set
            goto error;
        next_hop = gateway;
    }

    /* find addr in ARP table */
    if (arp_lookup(&next_hop, mac_dest, ndev, &sk->src_addr))
        // Failed. Drop packet
        goto error;

    /* build hard header */
    if (netdev_hdr_create(nb, ndev, ETH_P_IP, mac_dest,
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
