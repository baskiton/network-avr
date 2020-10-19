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
static volatile int8_t got_reply = 0;
static uint8_t dhcp_msg_type = 0;

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

/* DHCP options */
#define DHCP_OPT_PAD 0  /* Can be used to pad other options so that
                            they are aligned to the word boundary;
                            is not followed by length byte */
#define DHCP_OPT_SMASK 1    /* Subnet Mask; len=4. Must be sent before the router
                                option (option 3) if both are included */
#define DHCP_OPT_ROUTER 3   /* Router; len=4. Available routers, should be
                                listed in order of preference */
#define DHCP_OPT_DNS 6  /* Domain Name Server; len=4. Available DNS servers,
                            should be listed in order of preference */
#define DHCP_OPT_HNAME 12   /* Host Name; len=1 min. */
#define DHCP_OPT_DNAME 15   /* Domain Name; len=1 min. */

#define DHCP_OPT_REQIP 50   /* Requested IP address; len=4. */
#define DHCP_OPT_MT 53  /* Message Type; len=1. */
#define DHCP_OPT_SRVID 54   /* Server ID; len=4. */
#define DHCP_OPT_PRLIST 55  /* Parameter request list; len=1 min. */

#define DHCP_OPT_END 0xFF   /* Endmark */

struct dhcp_pkt_s {
    struct ip_hdr_s iph;    // IPv4 header
    struct udp_hdr_s udph;  // UDP header
    uint8_t op;             // 1 - request, 2 - reply
    uint8_t htype;          // hw addr type
    uint8_t hlen;           // hw addr len
    uint8_t hops;           // used only by gataway. client set to 0
    uint32_t xid;           // transaction ID
    uint16_t secs;          // seconds since started. may not be used (set to 0)
    uint16_t flags;         // DHCP special parameters
    uint32_t ciaddr;        // client IP, if known
    uint32_t yiaddr;        // assigned IP
    uint32_t siaddr;        // server IP
    uint32_t giaddr;        // relay agent IP
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
    struct dhcp_pkt_s *pkt;
    struct ip_hdr_s *iph;
    int16_t data_len, opt_len;

    if (net_dev != curr_net_dev)
        goto out;

    // check what packet is not for us
    if (net_buff->flags.pkt_type == PKT_OTHERHOST)
        goto out;
    
    pkt = (struct dhcp_pkt_s *)(net_buff->head + net_buff->network_hdr_offset);
    iph = &pkt->iph;

    // check receive protocol
    if ((iph->ihl != 5) || (iph->version != 4) ||
        (iph->protocol != IP_PROTO_UDP))
        goto out;
    
    // fragmentation not support
    if (iph->frag_off & htons(IP_MF | IP_OFFSET))
        goto out;
    
    // check checksum
    if (in_checksum(iph, iph->ihl))
        goto out;
    
    data_len = pkt->udph.len - sizeof(struct udp_hdr_s);
    opt_len = data_len - ((void *)pkt->options -
                          (void *)&pkt->udph -
                          sizeof(struct udp_hdr_s));
    if (opt_len < 0)
        goto out;
    
    // check what is already got reply
    if (got_reply)
        goto out;
    
    // check that this is the reply for us
    if ((pkt->op != BOOTP_REPLY) || pkt->xid != xid)
        goto out;
    
    // now parse
    // checking for existing options field
    if (opt_len >= 4 && !memcmp_P(pkt->options, dhcp_magic_cookie, 4)) {
        uint8_t *end = (void *)pkt + htons(pkt->iph.tot_len);
        uint8_t *opt_p = pkt->options + 4;
        uint32_t srv_id = IN_ADDR_NONE;
        uint8_t msg_type;

        // getting DHCP options from reply
        while (opt_p < end && (*opt_p != 0xFF)) {
            uint8_t *opt = opt_p++;

            if (!(*opt))    // padding
                continue;
            opt_p += *opt_p + 1;
            if (opt_p >= end)
                break;

            switch (*opt) {
                case DHCP_OPT_MT:
                    if (opt[1])
                        msg_type = opt[2];
                    break;

                case DHCP_OPT_SRVID:
                    if (opt[1] >= 4)
                        memcpy(&srv_id, opt + 2, 4);
                    break;
                
                default:
                    break;
            }
        }

        switch (msg_type) {
            case DHCP_ACK:
                if (!memcmp(net_dev->dev_addr, pkt->chaddr, ETH_MAC_LEN))
                    goto out;
                // SUCCESS
                break;

            case DHCP_OFFER:
                if (my_ip != IN_ADDR_NONE)
                    goto out;
                my_ip = pkt->yiaddr;
                serv_addr = srv_id;
                if ((serv_addr != IN_ADDR_NONE) && (serv_addr != pkt->siaddr))
                    pkt->siaddr = serv_addr;
                break;

            default:    // failed
                my_ip = IN_ADDR_NONE;
                serv_addr = IN_ADDR_NONE;
                goto out;
        }
        dhcp_msg_type = msg_type;

        // getting BOOTP options from reply
        opt_p = pkt->options + 4;
        while (opt_p < end && (*opt_p != 0xFF)) {
            uint8_t *opt = opt_p++;

            if (*opt == 0)  // padding
                continue;
            opt_p += *opt_p + 1;
            if (opt_p >= end)
                break;

            switch (*opt) {
                case DHCP_OPT_SMASK:
                    if (net_mask != IN_ADDR_NONE)
                        memcpy(&net_mask, opt + 1, 4);
                    break;
                
                case DHCP_OPT_ROUTER:
                    if (gateway != IN_ADDR_NONE)
                        memcpy(&net_mask, opt + 1, 4);
                    break;
                
                case DHCP_OPT_DNS:
                    /* code */
                    break;
                
                case DHCP_OPT_DNAME:
                    /* code */
                    break;
                
                default:
                    break;
            }
        }
    }

    my_ip = pkt->yiaddr;
    serv_addr = pkt->siaddr;
    if ((gateway == IN_ADDR_NONE) && (pkt->giaddr))
        gateway = pkt->giaddr;
    got_reply = 1;

out:
    free_net_buff(net_buff);
    return 0;
}

/*!
 * @brief Initialize DHCP options
 * @param options Pointer to options field in the packet
 */
static void dhcp_options_init(uint8_t *options) {
    uint8_t *opt = options;
    uint8_t msg_type = ((serv_addr == IN_ADDR_NONE) ?
                        DHCP_DISCOVER : DHCP_REQUEST);
    
    memcpy_P(opt, dhcp_magic_cookie, 4);    // RFC 1048 Magic Cookie
    opt += 4;
    *opt++ = DHCP_OPT_MT;
    *opt++ = 0x01;  // size of field
    *opt++ = msg_type;

    if (msg_type == DHCP_REQUEST) {
        *opt++ = DHCP_OPT_SRVID;
        *opt++ = 0x04;  // size of field
        memcpy(opt, &serv_addr, 4);
        opt += 4;

        *opt++ = DHCP_OPT_REQIP;
        *opt++ = 0x04;  // size of field
        memcpy(opt, &my_ip, 4);
        opt += 4;
    }
    *opt++ = DHCP_OPT_PRLIST;
    *opt++ = 4;     // count of list elements

    *opt++ = DHCP_OPT_SMASK;
    *opt++ = DHCP_OPT_ROUTER;   // gateway
    *opt++ = DHCP_OPT_DNS;
    *opt++ = DHCP_OPT_DNAME;

    *opt++ = DHCP_OPT_END;
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
        printf_P(PSTR("Error: IP config: dhcp_send_request: net_buff_alloc: not enough memory\n"));
        return;
    }
    net_buff->data += ETH_HDR_LEN;
    net_buff->tail += ETH_HDR_LEN;
    pkt = put_net_buff(net_buff, sizeof(struct dhcp_pkt_s));
    memset(pkt, 0, sizeof(struct dhcp_pkt_s));

    /* create IP header */
    net_buff->network_hdr_offset = net_buff->data - net_buff->head;
    iph = (struct ip_hdr_s *)(net_buff->head + net_buff->network_hdr_offset);
    iph->version = 4;
    iph->ihl = 5;
    iph->tot_len = htons(sizeof(struct dhcp_pkt_s));
    iph->frag_off = htons(IP_DF);
    iph->ttl = 64;
    iph->protocol = IP_PROTO_UDP;
    iph->ip_dst = htonl(IN_ADDR_BROADCAST);
    iph->hdr_chks = in_checksum(iph, iph->ihl * 4);
    /** \c tos, \c id and \c ip_src is already zero */

    /* create UDP header */
    pkt->udph.port_src = htons(68);
    pkt->udph.port_dst = htons(67);
    pkt->udph.len = htons(sizeof(struct dhcp_pkt_s) - sizeof(struct ip_hdr_s));
    // UDP checksum is not calculated - this is allowed in the BOOTP RFC

    /* create DHCP header */
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
    int8_t retries = 6;
    int32_t t_out;

    /** TODO: add packet handler for ETH_P_IP */

    printf_P(PSTR("Sending DHCP request..."));

    /* 6 attempt with timeout ~4 sec */
    while (1) {
        dhcp_send_request();

        t_out = F_CPU * 2;
        while (t_out && !got_reply)
            t_out--;
        
        if (got_reply) {
            if (dhcp_msg_type == DHCP_ACK) {
                printf_P(PSTR(" OK!\n"));
                break;
            }
            /* if message type is not DHCP_ACK (eg. OFFER),
                sending a new request according
                to the received data */
            got_reply = 0;
            continue;
        }
        if (!(--retries)) {
            printf_P(PSTR(" timed out!\n"));
            break;
        }
    }

    /** TODO: clean packet handler */

    if (!got_reply) {
        my_ip = IN_ADDR_NONE;
        return -1;
    }

    uint8_t *ptr;
    printf_P(PSTR("IP config: Success\n"));
    ptr = (void *)&my_ip;
    printf_P(PSTR("ip: %u.%u.%u.%u "), ptr[0], ptr[1], ptr[2], ptr[3]);
    ptr = (void *)&net_mask;
    printf_P(PSTR("Mask: %u.%u.%u.%u\n"), ptr[0], ptr[1], ptr[2], ptr[3]);
    ptr = (void *)&gateway;
    printf_P(PSTR("Gateway: %u.%u.%u.%u "), ptr[0], ptr[1], ptr[2], ptr[3]);
    ptr = (void *)&serv_addr;
    printf_P(PSTR("Server Addr: %u.%u.%u.%u\n\n"), ptr[0], ptr[1], ptr[2], ptr[3]);

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
    
    err = dhcp();
    if (err) {
        netdev_close(curr_net_dev);
        curr_net_dev = NULL;
        printf_P(PSTR("Error: IP config: Autoconfig of network failed\n"));
        return err;
    }

    return err;
}
