#include <avr/pgmspace.h>
#include <util/delay.h>

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "net/net.h"
#include "net/net_dev.h"
#include "net/ether.h"

extern void inet_init(void);

/*!
 * @brief Allocate a Network buffer
 * @param size Size to allocate
 * @return Pointer to buffer or \a NULL if error
 */
struct net_buff_s *net_buff_alloc(uint16_t size) {
    struct net_buff_s *buff;
    uint8_t *head;

    buff = malloc(sizeof(struct net_buff_s));
    if (!buff) {
        return NULL;
    }
    
    head = malloc(size);
    if (!head) {
        free(buff);
        return NULL;
    }

    memset(buff, 0, ((void *)&buff->head - (void *)buff));

    buff->head = buff->data = buff->tail = head;
    buff->end = buff->tail + size;
    buff->mac_hdr_offset = buff->data - buff->head;
    buff->network_hdr_offset = buff->mac_hdr_offset + ETH_HDR_LEN;
    buff->transport_hdr_offset = 0;

    return buff;
}

/*!
 * @brief Allocate a network buffer for rx on a specific device
 * @param dev Network device to receive
 * @param size Size to allocate (for ethernet is eth header, data, padding, crc)
 * @return Pointer to buffer or NULL if not enough memory
 */
struct net_buff_s *ndev_alloc_net_buff(struct net_dev_s *net_dev, uint16_t size) {
    struct net_buff_s *buff;

    buff = net_buff_alloc(size);
    if (buff)
        buff->net_dev = net_dev;

    return buff;
}

/*!
 * @brief Put a data to the buffer
 * @param net_buff Buffer to adding
 * @param len Length of adding data
 * @return Pointer to tail of buffer
 */
void *put_net_buff(struct net_buff_s *net_buff, uint16_t len) {
    void *old_tail = net_buff->tail;

    net_buff->tail += len;
    net_buff->pkt_len += len;

    if (net_buff->tail > net_buff->end) {
        /** TODO: error - out of range */
        printf_P(PSTR("Out of range!!! Stopping...\n"));
        while (1) {}
        return NULL;
    }
    return old_tail;
}

/*!
 * @brief Free the allocating memory an Net Buffer
 */
void free_net_buff(struct net_buff_s *net_buff) {
    free(net_buff->head);
    free(net_buff);
}

/*!
 * @brief Free list of buffer
 */
void free_net_buff_list(struct net_buff_s *net_buff) {
    while (net_buff) {
        struct net_buff_s *next = net_buff->next;

        free_net_buff(net_buff);
        net_buff = next;
    }
}

/*!
 * @brief Initialize the net working
 */
void network_init(void) {
    socket_list_init();

    inet_init();
}

/*!
 * @brief Get the net buffer from receive socket queue, wait if empty
 * @param q Queue
 * @param flags Flags
 * @return Pointer to buffer or \c NULL if error
 */
struct net_buff_s *net_buff_rcv(struct nb_queue_s *q, uint8_t flags) {
    struct net_buff_s *nb = NULL;
    uint32_t timeout;

    /** TODO: implement the timeout! */
    timeout = (flags & MSG_DONTWAIT) ? 0 : (F_CPU / 20);

    do {
        nb = nb_dequeue(q);
        if (nb)
            return nb;
        _delay_us(0);
    } while (timeout--);

    return NULL;
}
