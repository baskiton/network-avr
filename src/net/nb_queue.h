#ifndef NET_NB_QUEUE_H
#define NET_NB_QUEUE_H

#include <stdint.h>

#include "net/net.h"

struct nb_queue_s {
    struct net_buff_s *next,
                      *prev;
    uint8_t q_len;
};

/*!
 * @brief Network Buffer queue initialize
 * @param q Queue
 */
static inline void nb_queue_init(struct nb_queue_s *q) {
    q->next = q->prev = (struct net_buff_s *)q;
    q->q_len = 0;
}

#endif  /* !NET_NB_QUEUE_H */