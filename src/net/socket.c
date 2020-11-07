#include <avr/pgmspace.h>

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "net/net.h"
#include "net/socket.h"
#include "net/uio.h"

extern int8_t inet_sock_create(struct socket *sk, uint8_t protocol);

/* List of sockets */
struct socket *socket_list;

void socket_list_init(void) {
    socket_list->next = socket_list->prev = NULL;
}

/*!
 * @brief Add socket entry to the beginning of the socket list
 * @param new New entry to be added
 */
static void socket_list_add(struct socket *new) {
    if (!socket_list) {
        new->next = NULL;
    } else {
        socket_list->prev = new;
        new->next = socket_list;
    }
    socket_list = new;
    socket_list->prev = NULL;
}

/*!
 * @brief Remove entry from socket list
 * @param entry Socket to delete
 */
static void socket_list_del(struct socket *entry) {
    if (entry->next)
        entry->next->prev = entry->prev;
    if (entry->prev)
        entry->prev->next = entry->next;
    else
        socket_list = entry->next;
}

/*!
 * @brief Iterate over a socket list
 * @param i socket struct to iterate
 */
#define socket_list_for_each(i)   \
        for (i = socket_list; i; i = i->next)

/*!
 * @brief Used on \a server side. Accepts a received incoming attempt
 * to create a new TCP connection from the remote client, and creates
 * a new socket associated with the socket address pair of this connection.
 * @param sk Pointer to main socket
 * @param addr Pointer to socket address structure
 * @param addr_len Length of \p addr
 * @return New Socket pointer or NULL if error
 */
struct socket *accept(struct socket *restrict sk,
                      struct sockaddr *restrict addr,
                      socklen_t *restrict addr_len) {

    return NULL;
}

/*!
 * @brief Associates a socket with an address
 * @param sk Pointer to socket
 * @param addr Pointer to socket address structure to bind to
 * @param addr_len Length of \p addr
 * @return 0 on success
 */
int8_t bind(struct socket *sk,
            const struct sockaddr *addr,
            socklen_t addr_len) {
    int8_t err = -1;
    typedef int8_t (*func_t)(struct socket *,
                             const struct sockaddr *,
                             uint8_t);
    func_t f;

    if (sk) {
        f = pgm_read_ptr(&sk->p_ops->bind);
        if (f)
            err = f(sk, addr, addr_len);
    }
    return err;
}

/*!
 * @brief Used on \a client side. Assigns a free local port number to a socket.
 * Establishes a direct communication link to a specific remote
 * host identified by its address \p addr via a socket \p sk
 * @param sk Pointer to socket
 * @param addr Pointer to socket address structure
 * @param addr_len Length of \p addr
 * @return 0 on success
 */
int8_t connect(struct socket *sk,
               const struct sockaddr *addr,
               socklen_t addr_len) {

    return 0;
}

/*!
 * @brief Prepare to accept connections on socket \p sk.
 * Only necessary for the stream-oriented (connection-oriented)
 * data modes (SOCK_STREAM).
 * @param sk Pointer to socket
 * @param backlog Number of pending connections
 *      that can be queued up at any one time
 * @return 0 on success
 */
int8_t listen(struct socket *sk, uint8_t backlog) {

    return 0;
}

/*!
 * @brief Read \p buff_size bytes into \p buff from socket \p sk
 * @param sk Pointer to socket
 * @param buff Buffer to receive data
 * @param buff_size Size of \p buff
 * @param flag Flag (MSG_PEEK, MSG_OOB, MSG_WAITALL)
 * @return Number of received bytes or -1 for error
 */
ssize_t recv(struct socket *restrict sk,
             void *restrict buff,
             size_t buff_size,
             uint8_t flag) {
    return recvfrom(sk, buff, buff_size, flag, NULL, NULL);
}

/*!
 * @brief Read \p buff_size bytes into \p buff through socket \p sk.
 * If \p addr is not \c NULL, fill in \p *addr_len bytes of it with
 * that address of the sender, and store the actual size of
 * the address in \p *addr_len.
 * @param sk Pointer to socket
 * @param buff Buffer to receive data
 * @param buff_size Size of \p buff
 * @param flag Flag (MSG_PEEK, MSG_OOB, MSG_WAITALL)
 * @param addr Pointer to socket address structure
 * @param addr_len Length of \p addr
 * @return Number of received bytes or -1 for error
 */
ssize_t recvfrom(struct socket *restrict sk,
                 void *restrict buff,
                 size_t buff_size,
                 uint8_t flag,
                 struct sockaddr *restrict addr,
                 socklen_t *restrict addr_len) {

    return 0;
}

/*!
 * @brief Send \p buff_size bytes of \p buff to socket \p sk
 * @param sk Pointer to socket
 * @param buff Buffer with sending data
 * @param buff_size Size of \p buff
 * @param flag Flag (MSG_EOR, MSG_OOB, MSG_NOSIGNAL)
 * @param addr Pointer to socket address structure
 * @param addr_len Length of \p addr
 * @return Number of sending bytes
 */
ssize_t send(struct socket *sk, const void *buff,
             size_t buff_size, uint8_t flag) {
    return sendto(sk, buff, buff_size, flag, NULL, 0);
}

/*!
 * @brief Send \p buff_size bytes of \p buff on socket \p sk
 * to peer at address \p addr (which is \p addr_len bytes long).
 * @param sk Pointer to socket
 * @param buff Buffer with sending data
 * @param buff_size Size of \p buff
 * @param flag Flag (MSG_EOR, MSG_OOB, MSG_NOSIGNAL)
 * @param addr Pointer to socket address structure
 * @param addr_len Length of \p addr
 * @return Number of sending bytes
 */
ssize_t sendto(struct socket *sk, const void *buff, size_t buff_size,
               uint8_t flag, const struct sockaddr *addr, socklen_t addr_len) {
    ssize_t ret;
    struct msghdr msg;
    struct iovec iov;

    if (!sk) {
        // errno
        return -1;
    }

    /** TODO: */
    iovec_import(&iov, buff, buff_size);

    msg.msg_name = NULL;
    msg.msg_namelen = 0;
    msg.msg_flags = flag;

    if (addr) {
        msg.msg_name = addr;
        msg.msg_namelen = addr_len;
    }
    typedef ssize_t (*func_t)(struct socket *, struct msghdr *);
    func_t f = pgm_read_ptr(&sk->p_ops->sendmsg);
    ret = f(sk, &msg);

    return ret;
}

/*!
 * @brief Shut down all or part of the connection open on socket FD
 * @param sk Pointer to socket
 * @param flag Determines what to shut down:
 *      SHUT_RD   = No more receptions;
 *      SHUT_WR   = No more transmissions;
 *      SHUT_RDWR = No more receptions and transmissions.
 * @return 0 on success
 */
int8_t shutdown(struct socket *sk, uint8_t flag) {

    return 0;
}

/*!
 * @brief Creates a new Socket of a certain \c type
 * @param sk Pointer to Socket
 * @param family Address Family (e.g. AF_INET)
 * @param type Type of Socket (e.g. SOCK_STREAM)
 * @param protocol Protocol type (e.g. IPPROTO_TCP)
 * @return Pointer to Socket or NULL if error
 */
struct socket *socket(uint8_t family, uint8_t type, uint8_t protocol) {
    /**
    The socket() function shall fail if:
    \c [EAFNOSUPPORT]
        The implementation does not support the specified address family.
    \c [EPROTONOSUPPORT]
        The protocol is not supported by the address family,
        or the protocol is not supported by the implementation.
    \c [EPROTOTYPE]
        The socket type is not supported by the protocol.

    The socket() function may fail if:
    \c [ENOBUFS]
        Insufficient resources were available in
        the system to perform the operation.
    \c [ENOMEM]
        Insufficient memory was available to fulfill the request.
    */
    struct socket *sock;
    int8_t err;

    if (family >= AF_MAX)
        // EAFNOSUPPORT
        return NULL;
    if (type >= SOCK_MAX)
        // EPROTOTYPE
        return NULL;

    sock = malloc(sizeof(struct socket));
    if (!sock)
        // ENOMEM
        return NULL;

    memset(sock, 0, sizeof(struct socket));

    sock->type = type;

    switch (family) {
        case AF_INET:
            err = inet_sock_create(sock, protocol);
            break;
        
        default:
            // EAFNOSUPPORT
            err = -1;
            break;
    }
    if (err)
        goto release;

    socket_list_add(sock);

    return sock;

release:
    free(sock);
// out:
    return NULL;
}
