#ifndef NET_PING_H
#define NET_PING_H

#include "net/socket.h"

ssize_t ping_send_msg(struct socket *restrict sk,
                      struct msghdr *restrict msg);
ssize_t ping_recv_msg(struct socket *restrict sk,
                      struct msghdr *restrict msg,
                      int8_t flags, socklen_t *restrict addr_len);
bool ping_rcv(struct net_buff_s *nb);

#endif  /* !NET_PING_H */
