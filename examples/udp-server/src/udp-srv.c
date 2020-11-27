#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#include <defines.h>
#include <spi.h>
#include <uart.h>
#include <net/net.h>
#include <net/ipconfig.h>
#include <netinet/arp.h>
#include <arpa/inet.h>
#include <netinet/icmp.h>
#include <net/net_dev.h>
#include <net/interrupt.h>
#include <ENC28J60.h>

/*
Connection for arduino nano:
    ETHER   NANO
    SCK     D13 (SPI SCK)
    SO      D12 (SPI MISO)
    SI      D11 (SPI MOSI)
    CS      D10 (SPI !SS)
    INT     D2
    RESET   RST
    VCC     3V3
    GND     GND
*/

#define ETH_CS          PORTB2  // pin7
#define ETH_CS_PORT     &PORTB
#define ETH_RST         PORTC6  // pin reset
#define ETH_RST_PORT    &PORTC
#define ETH_INTR        PORTD2  // pin2
#define ETH_INTR_PORT   &PORTD

#define DHCP_ON 1  // DHCP enable
#define DHCP_OFF 0 // DHCP disable
#define MY_IP "192.168.1.200"       // your IP
#define NET_MASK "255.255.255.0"    // optional
#define GATEWAY "192.168.1.1"       // your router IP
#define DNS "8.8.8.8"               // optional

#define UDP_ADDRESS "0.0.0.0"
#define UDP_PORT 5000   // Make sure the port is forwarded!
#define MAX_BUF_LEN 64  // Maximum size of receiving buffer


ISR(INT0_vect) {
    net_dev_irq_handler();
}

void intr_config(void) {
    EICRA = 0;
    EIMSK = 1;
}

int8_t board_init(void) {
    spi_dev_t enc28j60_spi;
    int8_t err;
    uint8_t mac[6] = {0x00, 0x04, 0xA3, 0, 0, 1};

    cli();
    intr_config();

    uart_init(9600);
    uart_set_stdio();
    spi_init();

    err = spi_device_register(&enc28j60_spi,
                              ETH_CS, ETH_CS_PORT,
                              0, NULL,
                              ETH_INTR, ETH_INTR_PORT,
                              0, NULL);
    if (err) {
        printf_P(PSTR("Error from init enc28j60\n"));
        err = 0;
        goto out;
    }
    err = enc28j60_probe(&enc28j60_spi);
    if (err) {
        printf_P(PSTR("Error from probe enc28j60\n"));
        err = 0;
        goto out;
    }

    netdev_set_settings(curr_net_dev, true);
    netdev_set_mac_addr(curr_net_dev, mac);

out:
    sei();

    return err;
}

int8_t net_config(bool dhcp, const char *my_ip, const char *net_mask, const char *gateway, const char *dns) {
    int8_t err;

    if (dhcp)
        err = ip_auto_config();
    else
        err = ip_config(my_ip, net_mask, gateway, dns);

    if (err)
        return err;

    network_init();

    return err;
}

int8_t udp_server(void) {
    struct socket *sock;
    struct sockaddr_in serv_addr, client_addr;
    ssize_t count;
    socklen_t client_addr_len;
    char buf[MAX_BUF_LEN + 1];

    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (!sock) {
        printf_P(PSTR("The socket could not be created\n"));
        return -1;
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    memset(&client_addr, 0, sizeof(client_addr));

    client_addr_len = sizeof(client_addr);

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(inet_addr(UDP_ADDRESS));
    serv_addr.sin_port = htons(UDP_PORT);

    if (bind(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr))) {
        printf_P(PSTR("bind(): \n"));\
        goto out;
    }

    printf_P(PSTR("Server was started on %s:%u\n"),
             inet_ntoa(serv_addr.sin_addr),
             ntohs(serv_addr.sin_port));

    while (true) {
        count = recvfrom(sock, buf, MAX_BUF_LEN, 0,
                         (struct sockaddr *)&client_addr, &client_addr_len);
        if (count == -1) {
            printf_P(PSTR("recvfrom(): receiving error: %d\n"), count);
            break;
        }
        buf[count] = '\0';

        printf_P(PSTR("\nReceived from %s:%u\n"),
                 inet_ntoa(client_addr.sin_addr),
                 ntohs(client_addr.sin_port));
        printf("%s\n", buf);

        count = sendto(sock, buf, strlen(buf), 0,
                       (struct sockaddr *)&client_addr, client_addr_len);
        if (count != strlen(buf)) {
            printf_P(PSTR("sendto(): sending error: %d\n"), count);
            break;
        }
    }

out:
    sock_close(&sock);
    return -1;
}

int main(void) {
    int8_t err;

    err = board_init();
    if (err)
        return err;

    // configuring network
    err = net_config(DHCP_ON, NULL, NULL, NULL, NULL);
    // err = net_config(DHCP_OFF, MY_IP, NET_MASK, GATEWAY, DNS);
    if (err)
        return err;

    // and start UDP server!
    err = udp_server();

    return err;
}
