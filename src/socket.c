#include <stdint.h>
#include <stdlib.h>

#include "net/socket.h"

/*!
 * @brief Create the Socket
 * @param sk Pointer to socket
 * @param family Address Family
 * @param type Type of socket
 * @return Pointer to socket or NULL is error
 */
struct soket *socket(struct soket *sk, uint8_t family, uint8_t type) {

    return 0;
}

/*!
 * @brief Bind an address to a socket
 * @param sk Pointer to socket
 * @param sk_addr pointer to socket address structure
 * @return 0 if success; -1 if error
 */
int8_t bind(struct soket *sk, const struct sock_addr *sk_addr) {
    int8_t err = -1;

    if (sk) {
        err = sk->p_ops->bind(sk, sk_addr);
    }
    return err;
}

/*!
 * @brief
 * @param sk Pointer to socket
 * @param buff Buffer with sending data
 * @param buff_size Size of \p buffer
 * @param sk_addr pointer to socket address structure
 * @return Count of sending bytes
 */
int16_t send_to(struct soket *sk, const char *buff, size_t buff_size,
                struct sock_addr *sk_addr) {

    return 0;
}

int16_t send(struct soket *sk, const char *buff, size_t buff_size) {
    return send_to(sk, buff, buff_size, NULL);
}

/*!
 * @brief
 * @param sk Pointer to socket
 * @param buff Buffer to receive data
 * @param buff_size Size of \p buffer
 * @param sk_addr pointer to socket address structure
 * @return Count of received bytes
 */
int16_t recv_from(struct soket *sk, char *buff, size_t buff_size,
                  struct sock_addr *sk_addr) {

    return 0;
}

int16_t recv(struct soket *sk, char *buff, size_t buff_size) {
    return recv_from(sk, buff, buff_size, NULL);
}
