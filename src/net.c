#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "net/net.h"
#include "net/ether.h"

/*!
 * @brief Allocate a network buffer for rx on a specific device
 * @param dev Network device to receive
 * @param size Size to allocate (for ethernet is eth header, data, padding, crc)
 * @return Pointer to buffer or NULL if not enough memory
 */
struct net_buff_s *ndev_alloc_net_buff(struct net_dev_s *net_dev, uint16_t size) {
    struct net_buff_s *buff;
    uint8_t *head;

    buff = malloc(sizeof(struct net_buff_s));
    if (!buff) {
        return NULL;
    }
    
    head = malloc(size);
    if (!head) {
        free(buff);
        buff = NULL;
        goto end;
    }
    
    buff->head = buff->data = head;
    buff->mac_hdr_offset = buff->network_hdr_offset = buff->transport_hdr_offset = (uint8_t)-1U;
    buff->len = size;
    buff->net_dev = net_dev;

end:
    return buff;
}

/*!
 * @brief Free the allocating memory an Net Buffer
 */
void net_free_buff(struct net_buff_s *buff) {
    free(buff->head);
    free(buff);
}