#include <util/delay.h>
#include <avr/pgmspace.h>

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "net/net.h"
#include "net/net_dev.h"
#include "net/ipconfig.h"
#include "net/checksum.h"
#include "net/pkt_handler.h"
#include "arpa/inet.h"
#include "netinet/in.h"
#include "netinet/ip.h"
#include "netinet/udp.h"

in_addr_t my_ip = htonl(INADDR_NONE);       // My IP Address
in_addr_t net_mask = htonl(INADDR_NONE);    // Netmask for local subnet
in_addr_t gateway = htonl(INADDR_NONE);     // Gateway IP Address
in_addr_t dns_serv = htonl(INADDR_NONE);    // DNS server IP Address

static in_addr_t serv_addr = htonl(INADDR_NONE);    // Boot server IP Address
static uint32_t xid = 0;

static const uint8_t dhcp_magic_cookie[4] PROGMEM = {99, 130, 83, 99};
static volatile bool got_reply = false;
static uint8_t dhcp_msg_type = 0;

const uint8_t host_name[] PROGMEM = "bosduino";

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
    in_addr_t ciaddr;       // client IP, if known
    in_addr_t yiaddr;       // assigned IP
    in_addr_t siaddr;       // next server IP
    in_addr_t giaddr;       // relay agent IP
    uint8_t chaddr[16];     // client HW addr
    uint8_t sname[64];      // server name (null-terminated string, optional)
    uint8_t file[128];      // file name on server (null-terminated string, optional)
    uint8_t options[312];   // DHCP options
};

/*!
 * @brief Generate xid
 */
static void xid_gen(void) {
    for (uint8_t i = 0; i < 2; i++)
        ((uint16_t *)&xid)[i] = rand();
}

/*!
 * @brief Receive DHCP reply
 * @return 0 if succes
 */
static int8_t dhcp_recv(struct net_buff_s *net_buff) {
    struct dhcp_pkt_s *pkt;
    struct ip_hdr_s *iph;
    int16_t data_len, opt_len;
    struct net_dev_s *net_dev = net_buff->net_dev;

    if (net_dev != curr_net_dev)
        goto out;

    // check what packet is not for us
    if (net_buff->flags.pkt_type == PKT_OTHERHOST)
        goto out;
    
    pkt = (struct dhcp_pkt_s *)(net_buff->head + net_buff->network_hdr_offset);
    iph = &pkt->iph;

    // check receive protocol
    if ((iph->ihl != 5) || (iph->version != 4) ||
        (iph->protocol != IPPROTO_UDP))
        goto out;
    
    // fragmentation not support
    if (iph->frag_off & htons(IP_MF | IP_OFFSET))
        goto out;
    
    // check checksum
    if (in_checksum(iph, iph->ihl * 4))
        goto out;
    
    data_len = ntohs(pkt->udph.len) - sizeof(struct udp_hdr_s);
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
        in_addr_t srv_id = htonl(INADDR_NONE);
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
            case DHCP_OFFER:
                if (my_ip != htonl(INADDR_NONE))
                    goto out;
                my_ip = pkt->yiaddr;
                serv_addr = srv_id;
                if ((serv_addr != htonl(INADDR_NONE)) &&
                    (serv_addr != pkt->siaddr))
                    pkt->siaddr = serv_addr;
                break;

            case DHCP_ACK:
                if (memcmp(net_dev->dev_addr, pkt->chaddr, ETH_MAC_LEN))
                    goto out;
                // SUCCESS
                break;

            default:    // failed
                my_ip = htonl(INADDR_NONE);
                serv_addr = htonl(INADDR_NONE);
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

            switch (*opt++) {
                case DHCP_OPT_SMASK:
                    memcpy(&net_mask, opt + 1, 4);
                    break;
                
                case DHCP_OPT_ROUTER:
                    memcpy(&gateway, opt + 1, 4);
                    break;
                
                case DHCP_OPT_DNS:
                    memcpy(&dns_serv, opt + 1, 4);
                    break;
                
                case DHCP_OPT_DNAME:
                    /* code */
                    // break;
                
                default:
                    break;
            }
        }
    }

    my_ip = pkt->yiaddr;
    if (serv_addr == htonl(INADDR_NONE))
        serv_addr = pkt->siaddr;
    if ((gateway == htonl(INADDR_NONE)) && (pkt->giaddr))
        gateway = pkt->giaddr;
    got_reply = true;

out:
    free_net_buff(net_buff);

    return NETDEV_RX_SUCCESS;
}

/*!
 * @brief Initialize DHCP options
 * @param options Pointer to options field in the packet
 */
static void dhcp_options_init(uint8_t *options) {
    uint8_t *opt = options;
    uint8_t msg_type = ((serv_addr == htonl(INADDR_NONE)) ?
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

    if (host_name[0] != 0) {
        *opt++ = DHCP_OPT_HNAME;
        *opt++ = sizeof(host_name);
        memcpy_P(opt, host_name, sizeof(host_name));
        opt += sizeof(host_name);
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
static void dhcp_send_request(void) {
    struct net_buff_s *net_buff;
    struct dhcp_pkt_s *pkt;

    net_buff = net_buff_alloc(sizeof(struct dhcp_pkt_s) + ETH_HDR_LEN);
    if (!net_buff) {
        printf_P(PSTR("\nError: IP config: dhcp_send_request: net_buff_alloc: not enough memory\n"));
        return;
    }

    net_buff->data += ETH_HDR_LEN;
    net_buff->tail += ETH_HDR_LEN;

    pkt = put_net_buff(net_buff, sizeof(struct dhcp_pkt_s));
    memset(pkt, 0, sizeof(struct dhcp_pkt_s));

    /* create IP header */
    net_buff->network_hdr_offset = net_buff->data - net_buff->head;
    
    pkt->iph.version = 4;
    pkt->iph.ihl = 5;
    pkt->iph.tot_len = htons(sizeof(struct dhcp_pkt_s));
    pkt->iph.frag_off = htons(IP_DF);
    pkt->iph.ttl = 64;
    pkt->iph.protocol = IPPROTO_UDP;
    pkt->iph.ip_dst = htonl(INADDR_BROADCAST);
    pkt->iph.hdr_chks = in_checksum(&pkt->iph, pkt->iph.ihl * 4);
    /** \c tos, \c id and \c ip_src is already zero */

    net_buff->data += sizeof(struct ip_hdr_s);
    net_buff->tail += sizeof(struct ip_hdr_s);

    /* create UDP header */
    net_buff->transport_hdr_offset = net_buff->data - net_buff->head;

    pkt->udph.port_src = htons(68);
    pkt->udph.port_dst = htons(67);
    pkt->udph.len = htons(sizeof(struct dhcp_pkt_s) - sizeof(struct ip_hdr_s));
    // UDP checksum is not calculated - this is allowed in the BOOTP RFC

    net_buff->data += sizeof(struct udp_hdr_s);
    net_buff->tail += sizeof(struct udp_hdr_s);

    /* create DHCP header */
    pkt->op = BOOTP_REQUEST;
    pkt->htype = 0x01;  // Ethernet
    pkt->hlen = ETH_MAC_LEN;
    memcpy(pkt->chaddr, curr_net_dev->dev_addr, ETH_MAC_LEN);
    pkt->xid = xid;
    /** \c yiaddr and \c siaddr is alrady zero. */

    /* add DHCP options */
    dhcp_options_init(pkt->options);

    /* create Ethernet Header */
    net_buff->net_dev = curr_net_dev;
    net_buff->protocol = htons(ETH_P_IP);

    if (netdev_hdr_create(net_buff, curr_net_dev, ntohs(net_buff->protocol),
                          curr_net_dev->broadcast, curr_net_dev->dev_addr,
                          net_buff->pkt_len)) {
        free_net_buff(net_buff);
        printf_P(PSTR("Error: IP config: device header create error\n"));
        return;
    }
    if (netdev_list_xmit(net_buff) < 0) {
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

    xid_gen();
    pkt_hdlr_add(ETH_P_IP, dhcp_recv);

    printf_P(PSTR("Sending DHCP request..."));

    /* 6 attempt with timeout ~4 sec */
    while (true) {
        dhcp_send_request();

        t_out = F_CPU / 4;
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
            got_reply = false;
            continue;
        }
        if (!(--retries)) {
            printf_P(PSTR(" timed out!\n"));
            break;
        }
        xid_gen();
        putchar('.');
    }

    pkt_hdlr_del(ETH_P_IP);

    if (!got_reply) {
        my_ip = htonl(INADDR_NONE);
        return -1;
    }

    return 0;
}

/*!
 * @brief Auto configuring the IP address with DHCP
 * for the currently active network device
 * @return 0 if success; errno if error
 */
int8_t ip_auto_config(void) {
    int8_t err = 0;
    uint8_t *ptr;

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

    if (my_ip == htonl(INADDR_NONE)) {
        // loop until link status is UP
        while (!net_dev_link_is_up(curr_net_dev)) {
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
    }

    if (net_mask == htonl(INADDR_NONE)) {
        err = inet_class_determine(&my_ip, &net_mask);
        if ((err < 0) || (err >= IN_CLASS_D)) {
            printf_P(PSTR("Error: IP config: This IP address is reserved and "
                          "cannot be assigned to a network or host.\n"));
            err = -1;
            return err;
        }
        err = 0;
    }

    printf_P(PSTR("IP config: Success\n"));
    ptr = (void *)&my_ip;
    printf_P(PSTR("ip: %u.%u.%u.%u\n"), ptr[0], ptr[1], ptr[2], ptr[3]);
    ptr = (void *)&net_mask;
    printf_P(PSTR("Mask: %u.%u.%u.%u\n"), ptr[0], ptr[1], ptr[2], ptr[3]);
    ptr = (void *)&gateway;
    printf_P(PSTR("Gateway: %u.%u.%u.%u\n"), ptr[0], ptr[1], ptr[2], ptr[3]);
    ptr = (void *)&dns_serv;
    printf_P(PSTR("DNS: %u.%u.%u.%u\n\n"), ptr[0], ptr[1], ptr[2], ptr[3]);

    return err;
}

/*!
 * @brief Configuration the IP address (or autoconfig with DHCP)
 *  for the currently network device.
 * @param ip IP-address to set, string (might be \a NULL)
 * @param nm Net Mask to set (might be \a NULL)
 * @param gw Gateway address (might be \a NULL)
 * @param dns DNS address (might be \a NULL)
 * @return 0 if success
 */
int8_t ip_config(const char *ip, const char *nm,
                 const char *gw, const char *dns) {
    if (ip && (ip[0] != '\0'))
        my_ip = inet_addr(ip);

    if (nm && (nm[0] != '\0'))
        net_mask = inet_addr(nm);

    if (gw && (gw[0] != '\0'))
        gateway = inet_addr(gw);

    if (dns && (dns[0] != '\0'))
        dns_serv = inet_addr(dns);

    return ip_auto_config();
}
