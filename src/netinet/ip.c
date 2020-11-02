#include <stdint.h>

#include "net/net.h"
#include "net/pkt_handler.h"
#include "net/checksum.h"
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
 *
 */
void ip_init(void) {
    icmp_init();
    tcp_init();
    udp_init();

    pkt_hdlr_add(ETH_P_IP, ip_recv);
}
