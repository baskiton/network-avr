#ifndef NET_PING_H
#define NET_PING_H

#include "net/socket.h"

ssize_t ping_send_msg(struct socket *restrict sk,
                      struct msghdr *restrict msg);

#endif  /* !NET_PING_H */
