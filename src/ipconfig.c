#include <util/delay.h>
#include <avr/pgmspace.h>

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "net/net.h"
#include "net/net_dev.h"
#include "net/ipconfig.h"
#include "net/ip.h"
#include "net/udp.h"
#include "net/checksum.h"

uint32_t my_ip = htonl(IN_ADDR_NONE);       // My IP Address
uint32_t net_mask = htonl(IN_ADDR_NONE);    // Netmask for local subnet
uint32_t gateway = htonl(IN_ADDR_NONE);     // Gateway IP Address
uint32_t serv_addr = htonl(IN_ADDR_NONE);   // Boot server IP address

static uint32_t xid = 0;

static const uint8_t dhcp_magic_cookie[4] PROGMEM = {99, 130, 83, 99};

/* packet ops */
#define BOOTP_REQUEST 1
#define BOOTP_REPLY 2

/* DHCP message types */
#define DHCP_DISCOVER 1
#define DHCP_OFFER 2
#define DHCP_REQUEST 3
#define DHCP_DECLINE 4
#define DHCP_ACK 5
#define DHCP_NAK 6
#define DHCP_RELEASE 7
#define DHCP_INFORM 8

struct dhcp_pkt_s {
    struct ip_hdr_s iph;    // IPv4 header
    struct udp_hdr_s udph;  // UDP header
    uint8_t op,             // 1 - request, 2 - reply
            htype,          // hw addr type
            hlen,           // hw addr len
            hops;           // used only by gataway. client set to 0
    uint32_t xid;           // transaction ID
    uint16_t secs;          // seconds since started. may not be used (set to 0)
    uint16_t flags;         // DHCP special parameters
    uint32_t ciaddr,        // client IP, if known
             yiaddr,        // assigned IP
             siaddr,        // server IP
             giaddr;        // relay agent IP
    uint8_t chaddr[16];     // client HW addr
    uint8_t sname[64];      // server name (null-terminated string, optional)
    uint8_t file[128];      // file name on server (null-terminated string, optional)
    uint8_t options[312];   // DHCP options
};

/*!
 * @brief Receive DHCP reply
 * @return 0 if succes
 */
static int8_t dhcp_recv(struct net_buff_s *net_buff,
                        struct net_dev_s *net_dev) {
    return 0;
}

/*!
 * @brief Initialize DHCP options
 * @param options Pointer to options field in the packet
 */
static void dhcp_options_init(uint8_t *options) {
    uint8_t *opt = options;
    uint8_t msg_type = (serv_addr == IN_ADDR_NONE) ? DHCP_DISCOVER : DHCP_REQUEST;
    
    memcpy_P(opt, dhcp_magic_cookie, 4);    // RFC 1048 Magic Cookie
    opt += 4;
    *opt++ = 0x35;  // DHCP message type
    *opt++ = 0x01;
    *opt++ = msg_type;

    if (msg_type == DHCP_REQUEST) {
        *opt++ = 0x36;  // Server ID (IP addr)
        *opt++ = 0x04;
        memcpy(opt, &serv_addr, 4);
        opt += 4;

        *opt++ = 0x32;  // Request IP addr
        *opt++ = 0x04;
        memcpy(opt, &my_ip, 4);
        opt += 4;
    }
    *opt++ = 0x37;  // Parameter request list
    *opt++ = 4;     // count of list elements

    *opt++ = 0x01;  // subnet mask
    *opt++ = 0x03;  // gateway
    *opt++ = 0x06;  // DNS server
    *opt++ = 0x0F;  // domain name

    *opt++ = 0xFF;  // endmark
}

/*!
 * @brief Send DHCP request
 */
static void dhcp_send_request() {
    struct net_buff_s *net_buff;
    struct dhcp_pkt_s *pkt;
    struct ip_hdr_s *iph;

    net_buff = net_buff_alloc(sizeof(struct dhcp_pkt_s) + ETH_HDR_LEN);
    if (!net_buff) {
        /** TODO: */
        printf_P(PSTR("Error: IP config: dhcp_send_request: net_buff_alloc: not enough memory\n"));
        return;
    }
    net_buff->data += ETH_HDR_LEN;
    net_buff->tail += ETH_HDR_LEN;
    pkt = put_net_buff(net_buff, sizeof(struct dhcp_pkt_s));
    memset(pkt, 0, sizeof(struct dhcp_pkt_s));

    /* construct IP header */
    net_buff->network_hdr_offset = net_buff->data - net_buff->head;
    iph = (struct ip_hdr_s *)(net_buff->head + net_buff->network_hdr_offset);
    iph->version = 4;
    iph->ihl = 5;
    // iph->tos = 0;
    iph->tot_len = htons(sizeof(struct dhcp_pkt_s));
    // iph->id = 0;
    iph->frag_off = htons(IP_DF);
    iph->ttl = 64;
    iph->protocol = IP_PROTO_UDP;
    iph->ip_src = 0;    // set from memset() */
    iph->ip_dst = htonl(IN_ADDR_BROADCAST);
    iph->hdr_chks = in_checksum(iph, iph->ihl * 4);

    /* construct UDP header */
    pkt->udph.port_src = htons(68);
    pkt->udph.port_dst = htons(67);
    pkt->udph.len = htons(sizeof(struct dhcp_pkt_s) - sizeof(struct ip_hdr_s));
    // UDP checksum is not calculated - this is allowed in the BOOTP RFC

    /* construct DHCP header */
    pkt->op = BOOTP_REQUEST;
    pkt->htype = 0x01;  // Ethernet
    pkt->hlen = ETH_MAC_LEN;
    memcpy(pkt->chaddr, curr_net_dev->dev_addr, ETH_MAC_LEN);
    pkt->xid = xid;
    /** \c yiaddr and \c siaddr is alrady zero. */

    /* add DHCP options */
    dhcp_options_init(pkt->options);

    net_buff->net_dev = curr_net_dev;
    net_buff->protocol = htons(ETH_P_IP);

    if (netdev_hdr_create(net_buff, curr_net_dev,
        ntohs(net_buff->protocol),
        curr_net_dev->broadcast, curr_net_dev->dev_addr,
        net_buff->pkt_len)) {
        free_net_buff(net_buff);
        printf_P(PSTR("Error: IP config: device header create error\n"));
        return;
    }
    if (netdev_xmit(net_buff)) {
        printf_P(PSTR("Error: IP config: transfer failed\n"));
    }
}

/*!
 * @brief DHCP configuration
 * @return 0 if success
 */
static int8_t dhcp(void) {
    /* add packet handler for ETH_P_IP */

    for (int8_t retries = 6; retries > 0; retries--) {
        dhcp_send_request();
        /* timeout */
        /* break if complete */
    }
    /* clean packet handler */

    /* if not complete return with -1 */

    return 0;
}

/*!
 * @brief Auto configuring the IP address with DHCP
 * for the currently active network device
 * @return 0 if success; errno if error
 */
int8_t ip_auto_config(void) {
    int8_t err = 0;

    if (!curr_net_dev) {
        // no device - error
        // ENODEV
        printf_P(PSTR("Error: IP Config: No Network Device\n"));
        return -1;
    }

    // opening net device...
    // in this operation, if successful, the up_state flag is set
    err = netdev_open(curr_net_dev);
    if (err) {
        printf_P(PSTR("Error: IP Config: Failed to open\n"));
        return err;
    }
    xid = random();

    // loop until link status UP
    while (!curr_net_dev->flags.link_status) {
        /** BUG: for some reason does not work without delay */
        _delay_ms(0);
    }
    
    if (dhcp()) {
        netdev_close(curr_net_dev);
        curr_net_dev = NULL;
        printf_P(PSTR("Error: IP config: Autoconfig of network failed\n"));
        return -1;
    }

    return err;
}
