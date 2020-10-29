#ifndef PKT_HANDLER_H
#define PKT_HANDLER_H

#include <stdint.h>

#include "net/net.h"

int8_t recv_pkt_handler(struct net_buff_s *net_buff);
void pkt_hdlr_add(uint16_t type, proto_hdlr_t handler);
void pkt_hdlr_del(uint16_t type);

#endif  /* !PKT_HANDLER_H */
