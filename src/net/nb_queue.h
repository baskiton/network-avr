#ifndef NET_NB_QUEUE_H
#define NET_NB_QUEUE_H

#include <stdint.h>

#include "net/net.h"

struct nb_queue_s {
    struct net_buff_s *next,
                      *prev;
    uint8_t q_len;
};

#endif  /* !NET_NB_QUEUE_H */