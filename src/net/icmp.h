#ifndef ICMP_H
#define ICMP_H

#include <stdint.h>

#define ICMP_ECHO_REPLY_T 0 // Echo Reply type
#define ICMP_ECHO_REPLY_C 8 // Echo Reply code
#define ICMP_ECHO_REQ_T 8   // Echo Request type
#define ICMP_ECHO_REQ_C 0   // Echo Request code

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

#endif  /* !ICMP_H */
