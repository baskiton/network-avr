#ifndef ICMP_H
#define ICMP_H

#include <stdint.h>

#include "net.h"

#define ICMP_ECHO_REPLY 0       // Echo Reply type
#define ICMP_DEST_UNREACH 3     // Destination Unreachable
#define ICMP_SOURCE_QUENCH 4    // Source Quench (deprecated)
#define ICMP_REDIRECT 5         // Redirect (change route)
#define ICMP_ECHO_REQ 8         // Echo Request type
#define ICMP_TIME_EXCEEDED 11   // Time Exceeded
#define ICMP_PARAM_PROBLEM 12   // Parameter Problem
#define ICMP_TIMESTAMP_REQ 13   // Timestamp Request
#define ICMP_TIMESTAMP_REPLY 14 // Timestamp Reply
#define ICMP_INFO_REQ 15        // Information Request (deprecated)
#define ICMP_INFO_REPLY 16      // Information Reply (deprecated)
#define ICMP_ADDRESS_REQ 17     // Address Mask Request (deprecated)
#define ICMP_ADDRESS_REPLY 18   // Address Mask Reply (deprecated)

struct icmp_hdr_s {
    uint8_t type;   // Type of message
    uint8_t code;   // Code
    uint16_t chks;  // Checksum
    union {
        struct {
            uint16_t id;        // Identifier
            uint16_t seq_num;   // Sequence Number
        } echo;         // Used for ping
        uint32_t data;
    } hdr_data;     // Header Data
};

void icmp_init(void);
struct net_buff_s *icmp_create();
void icmp_send(struct net_buff_s *nb, uint8_t type, uint8_t code);

#endif  /* !ICMP_H */
