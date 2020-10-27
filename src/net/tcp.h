#ifndef TCP_H
#define TCP_H

#include <stdint.h>

struct tcp_hdr_s {
    uint16_t port_src;  // Source port
    uint16_t port_dst;  // Destination port
    uint32_t seq_num;   // Sequence Number
    uint32_t ack_num;   // Acknowledgment Number (if ACK set)
    uint16_t ns : 1;    // ECN-nonce - concealment protection
    uint16_t res : 3;   // Reserved
    uint16_t d_off : 4; // Data Offset
    uint16_t fin : 1;   // Last packet from sender (used for connection termination)
    uint16_t syn : 1;   // Synchronize sequence numbers
    uint16_t rst : 1;   // Reset the connection
    uint16_t psh : 1;   // Push function
    uint16_t ack : 1;   // Acknowledgement field is significant
    uint16_t urg : 1;   // Urgent pointer field is significant
    uint16_t ece : 1;   // ECN-Echo
    uint16_t cwr : 1;   // Congestion window reduced
    uint16_t win_size;  // Window size
    uint16_t chksum;    // Checksum
    uint16_t urg_ptr;   // Urgent pointer (if URG set)
    /* Options is here
     *  (if d_off > 5. Padded at the end with "0" bytes if necessary.)
     */
};

/* TCP Options */
#define TCP_OPT_EOL 0   // End of Options List
#define TCP_OPT_NOP 1   // No-Operation (used only for padding)
#define TCP_OPT_MSS 2   // Maximum Segment Size (only be sent when SYN is set)
#define TCP_OPT_WIN_SCL 3   // Window Scale (only be sent when SYN is set)
#define TCP_OPT_SACK_PERM 4 // SACK Permitted (only be sent when SYN is set)
#define TCP_OPT_SACK 5  // SACK (Selective Acknowledgment)
#define TCP_OPT_TIMESTAMP 8 // Timestamps
/* The remaining Option-Kind values are historical,
 *  obsolete, experimental, not yet standardized, or unassigned.
 */

/* Length of TCP Options Field */
#define TCP_OPT_MSS_LEN 4
#define TCP_OPT_WIN_SCL_LEN 3
#define TCP_OPT_SACK_PERM_LEN 2
#define TCP_OPT_TIMESTAMP_LEN 10
// SACK len is N - 10, 18, 26, or 34

struct tcp_opt_field_s {
    uint8_t opt_kind;
    uint8_t opt_len;
};

void tcp_init(void);

#endif  /* !TCP_H */
